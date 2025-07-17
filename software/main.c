/*******************************************************************************
* (c) 2025 by Theo Borm
* see LICENSE file in this repository
*
* This firmware Controls the electronic "trinket"
*
* The trinket has two separate functions that have little in common:
* 1) It is a push-button operated flashlight
* 2) It communicates with other trinkets in the neighborhood, displaying a pallette
*    of colours when it detects others, and white flashes if it doesn't
*
* FLASHLIGHT
* the flashlight is operated by a push-button, operating a state machine.
* the state machine code is run 1000 times per second, so this statemachine
* also handles timing through a timeout counter
*
* fstate   | condition 		| action(s)
* start    |           		| OFF, fstate=0
* 0        | BUTTON    		| ON, fstate=1, timeout=2000
* 0        | nBUTTON   		|
*
* 1	   | BUTTON & nTIMEOUT	| timeout--
* 1	   | nBUTTON & nTIMEOUT | fstate=2, timeout=10000
* 1 	   | TIMEOUT            | fstate=5, timeout=50000
*
* 2        | nBUTTON & nTIMEOUT | timeout--
* 2	   | nBUTTON & TIMEOUT  | OFF, fstate=0
* 2	   | BUTTON		| fstate=3, OFF, timeout=500
*
* 3        | nTIMEOUT & BUTTON	| timeout--
* 3	   | TIMEOUT & BUTTON	| fstate=4, ON, timeout=500
* 3        | nBUTTON		| fstate=0
*
* 4	   | nTIMEOUT * BUTTON  | timeout--
* 4	   | TIMEOUT & BUTTON	| fstate=3, OFF, timeout=500
* 4	   | nBUTTON		| OFF, fstate=0
*
* 5	   | nTIMEOUT		| timeout--
* 5	   | TIMEOUT		| OFF, fstate=6, timeout=100
*
* 6	   | nTIMEOUT		| timeout--
* 6	   |  TIMEOUT		| ON, fstate=7, timeout=10000
*
* 7	   | BUTTON		| fstate=5, timeout=50000
* 7	   | nBUTTON & nTIMEOUT | timeout--
* 7        | nBUTTON & TIMEOUT  | OFF, fstate=0
*
* off -> short press -> on -> short press -> off
* off -> short press -> on -> long press -> blink -> release -> off
* off -> short press -> on -> 10s timeout -> off
* off -> long press -> on -> 50s timeout -> short blink -> 10s timeout -> off
* off -> long press -> on -> (*) 50s timeout -> short blink -> short press -> (*)
*
* TRINKET DETECTOR
*
* Trinkets detect each others presence through IR pulses they transmit about
* once a minute. Each pulse lasts a few ms, and modulates a38 kHz carrier.
* While a trinket is transmitting an IR pulses plus a few ms afterwards, reception
* of IR pulses is disabled so it does not react to itself.

* Trinkets can be in one of two states: "together" or "alone". When "alone", a
* trinket transmits a short (~16ms) white flash every ~2s on alternating RGB leds.
* When "together", a trinket slowly but continuously cycles through a range of
* colours, albeit at e alower intensity. The color patters and the together/alone
* handling are handled using two separate state machines. These state machines'
* code is run 1000 times per second and timing is handled through timeout counters
*
*<------ one synchronization cycle ------------>
*   +-----+
*   |     |
*---+     +------------------------------------- pulse from some trinket A - detected by other trinkets
*
*   +---------+
*   |         |
*---+         +--------------------------------- deaf time trinket A
*
*             +-----------+
*             |           |
*-----------=-+           +-------------------- register-pulse-but-don't-resync-time A
*
*---+                     +---------------------
*   |                     |
*   +---------------------+                      register AND resync time A
*
*phases trinket A
*
*00001111112222333333333333000000000000000000000
* 0: during this time pulses are registered AND used to resynchronize
* 1: during this time a pulse is transmitted and the tinket is deaf
* 2: during this time the trinket is still deaf
* 3: during this time the trinket registers pulses, but doesn't synchronize
*
*
phases trinket B (synchronizing with A)
00000044444411111122223333333333330000000000000
*   +-----+
*   |     |
*---+     +------------------------------------- received pulse from trinket A
^
*           +-----+
*           |     |
*-----------+     +----------------------------- pulse from atrinket B
*
* 4: during this time a trinket resynchronizes after receiving a pulse in phase 0
*
* ttimeout=0 means alone
*
* dstate   | condition 		   | action(s)
* start    |                       | pattern=0, dstate=0, dtimeout=60000
*
* any      | always at state-begin | dtimeout--, ttimeout-- (stop at 0), pattern++
*
* 0	   | nDTIMEOUT & nRECV     | 
* 0	   | nDTIMEOUT & RECV	   | dstate=4, ttimeout=65535 dtimeout=10
* 0	   | DTIMEOUT		   | dstate=1, dtimeout=10, ON
*
* 1	   | nDTIMEOUT		   | 
* 1        | DTIMEOUT		   | dstate=2, dtimeout=10, OFF
*
* 2        | nDTIMEOUT             | 
* 2        | DTIMEOUT              | dtimeout=20, dstate=3
*
* 3        | nDTIMEOUT & nRECV     | 
* 3	   | nDTIMEOUT & RECV	   | ttimeout=65535
* 3        | DTIMEOUT		   | dtimeout=60000, dstate=0
*
* 4        } nDTIMEOUT		   |
* 4        } DTIMEOUT              | dstate=1, dtimeout=10, ON
*
* if ttimeout == 0, then trinkets are unsynchronized, and the 16 bit pattern counter is
* used to generate short flashes of white light. counter bits: 
*               xxxx r100 0000 xxqq -> led r 15ms ON (after 1s, 5s, 9s, 13s, ....)
*               xxxx r100 0000 xxqq -> led r 15ms  ON (after 3s, 7s, 11s, 15s, ....)
*
* value of qq determines which constituent color LED may be on during that phase:
*	00	R1, G2
*	01	G1, B2
*	10	B1, R2
*	11	OFF
*
* if timeout !=0, then trinkets are "together", and the colour to display is derived
* from the 16 bit pattern counter which is set up to count from 0 to 6143:
*	counter value:		colour led 1	colour led 2
*	xxx0 000x xxxx xxxq     1,0,0		0,0,0
*	xxx0 001x xxxx xxxq     0,0,0		0,1,1
*	xxx0 010x xxxx xxxq     1,1,0		0,0,0
*	xxx0 011x xxxx xxxq     0,0,0		0,0,1
*	xxx0 100x xxxx xxxq     0,1,0		0,0,0
*	xxx0 101x xxxx xxxq     0,0,0		1,0,1
*	xxx0 110x xxxx xxxq     0,1,1		0,0,0
*	xxx0 111x xxxx xxxq     0,0,0		1,0,0
*	xxx1 000x xxxx xxxq     0,0,1		0,0,0
*	xxx1 001x xxxx xxxq     0,0,0		1,1,0
*	xxx1 010x xxxx xxxq     1,0,1		0,0,0
*	xxx1 011x xxxx xxxq     0,0,0		0,1,0
* the following patterns should never occur:
*	xxx1 100x xxxx xxxq     1,0,0		1,0,0
*	xxx1 101x xxxx xxxq     0,1,0		0,1,0
*	xxx1 110x xxxx xxxq     1,0,1		0,0,1
*	xxx1 111x xxxx xxxq     0,0,0		0,0,0
*
* note that at any one time at most two leds are "on" at the "same time"
* to halve the LED power consumption, a subphase q is used to quickly 
* switch between these two (at most) LEDs
*
* PFS154 pin assignment
* 
* pin	dir	func	active
* PA0	out	RGB1_R	low
* PA3	out	DEBUGIR	low
* PA4	out	RGB1_B	low
* PA5	n.c.	n.a.	n.a.
* PA6	n.c.	n.a.	n.a.
* PA7	in,p.u.	IR_IN	low
* PB0	out	RGB1_G	low	
* PB1	n.c.	n.a.	n.a.
* PB2	out	RGB2_G	low
* PB3	out	LIGHT	high
* PB4	out	IR_OUT	high
* PB5	out	RGB2_R	low
* PB6	out	RGB2_B	low
* PB7	in,p.u.	BUTTON	low
*/
/*
* PA3, PA5, PA6 and PB1 are used as debug status outputs
* PB1: 1Hz blinking from the main loop
* PA3: LOW when the trinket is transmitting an IR pulse
* PA6: LOW when the trinket is in the "long" flashlight state
*/






#include <stdint.h>
#include <device.h>
#include <calibrate.h>











/*******************************************************************************
* configure/calibrate system clock source
*/
unsigned char _sdcc_external_startup(void)
{
	// use the IHRC oscillator, target 4MHz
	PDK_SET_SYSCLOCK(SYSCLOCK_IHRC_4MHZ);
	// calibrate for 4MHz operation @ 3000 mVolt
	EASY_PDK_CALIBRATE_IHRC(4000000,3000);
	
	// watchdog is disabled by default
	
	
	/* MISC register bit settings
	 * 7: 0 reserved
	 * 6: 0 reserved
	 * 5: 1 FAST wake-up
	 * 4: 0 disable VDED/2 bias voltage generator
	 * 3: 0 reserved
	 * 2: 0 disable LVR
	 * 1: 0 WDT 8192
	 * 0: 0 WDT 8192
	 */
	 MISC=0x20;
	 
	return 0;   // keep SDCC happy
}



/*******************************************************************************
* NOTE: A large part (most) of the functionality is implemented in an interrupt.
*/


/******************************************************************************* 
* Global variables and definitions
*/

volatile uint8_t dstate; 	// the together/alone communication state machine
volatile uint16_t dtimeout; 	// the timeout counter for the detection states
volatile uint16_t ttimeout; 	// the timeout counter for the together/alone detection
volatile uint16_t pattern; 	// the pattern counter that determines what is shown on the RGB leds

volatile uint8_t fstate; 	// the state of the flashlight state machine
volatile uint16_t ftimeout; 	// the timeout counter for the flashlight

volatile uint8_t buttonshift;	// the state of the button as read from the pin during the last 8 cycles
volatile uint8_t buttonstate;	// the state of the button after debouncing
volatile uint8_t IRrecshift;	// the state of the IR receiver as read from the pin during the last 8 cycles
volatile uint8_t IRrecstate;	// the previous state & edge of the IR receiver after debouncing


uint8_t PA_next;		// variable used to compute next value for PA to prevent glitches
uint8_t PB_next;		// variable used to compute next value for PB to prevent glitches
#define R1_ON	PA_next &= 0x7e
#define G1_ON	PB_next &= 0x7e
#define B1_ON	PA_next &= 0x6f
#define R2_ON	PB_next &= 0x5f
#define G2_ON	PB_next &= 0x7b
#define B2_ON	PB_next &= 0x3f

/******************************************************************************* 
* Timer interrupt setup
*
* T16 can be clocked from several sources, use a clock divider and can generate
* an interrupt when a certain bit (8..15) changes. If T16 is NOT clocked from
* SYSCLOCK and IHRC is enabled, then it can wake the CPU from sleep.
* We assume the IHRC is calibrated to 16MHz, then dividing by 64 will produce
* a 250 KHz input clock to T16. After 256 clock pulses bit 8 will toggle, which
* is almost once a millisecond. To make it exactly once a millisecond, we should
* preload T16 (which is an up-counter) with the value 6.
* 
*/

// configure timer
void setup_ticks() {
	T16M = (uint8_t)(T16M_CLK_IHRC | T16M_CLK_DIV64 | T16M_INTSRC_8BIT);
	T16C=6;
	INTEN |= INTEN_T16;
}



// interrupt service routine - does all the work
void interrupt(void) __interrupt(0)
{
	if (INTRQ & INTRQ_T16)
	{
		INTRQ &= ~INTRQ_T16; // Mark as processed
		T16C=134;

		// generic stuff that needs to be done every ms
		
		pattern++;
		if (pattern >= 6144) { pattern=0; }
		
		if (dtimeout) dtimeout--;
		if (ttimeout) ttimeout--;
		if (ftimeout) ftimeout--;
		
		// debounce the button
		buttonshift = buttonshift << 1;
		if (PB & 0x80) buttonshift |= 1;
		if (buttonshift==0xff) buttonstate=0;
		if (buttonshift==0x00) buttonstate=1;
		
		// debounce the IR receiver and detect edge
		IRrecshift = IRrecshift << 1;
		if (PA & 0x80) { IRrecshift |= 1; }
		if ((IRrecshift & 0x1f)==0x1f) { IRrecstate = (IRrecstate << 1) & 0x02; }
		if ((IRrecshift & 0x1f)==0x00) { IRrecstate = ((IRrecstate << 1) & 0x02) | 1; }
		// arriving here, IRrecstate can have one of 4 values:
		// 0: IR signal inactive - no change
		// 1: IR signal going active edge -> this requires action!
		// 2: IR signal going inactive edge
		// 3: IR signal active - no change 
		
		
		// handle the state machines
		switch (fstate) {
			case 0:
				if (buttonstate) {
					// switch flashlight on (by setting PB3 high), and all other leds off
					PA |= 0b00010001;	
					PB |= 0b01101101;	

					fstate=1;
					ftimeout=500;
				}
				break;
			case 1:
				if (buttonstate==0) {
					fstate=2;
					ftimeout+=9500;
				} else if (ftimeout==0) {
					fstate=5;
				}
				break;
			case 2:
				if (buttonstate==1) {
					fstate=3;
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
					ftimeout=500;
				} else if (ftimeout==0) {
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
					fstate=0;
				}
				break;
			case 3:
				if (buttonstate==0) {
					fstate=0;
				} else if (ftimeout==0) {
					fstate=4;
					// switch flashlight on (by setting PB3 high), and all other leds off
					PA |= 0b00010001;	
					PB |= 0b01101101;	
					ftimeout=500;
				}				
				break;
			case 4: if (buttonstate==0) {
					fstate=0;
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
				} else if (ftimeout==0) {
					fstate=3;
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
					ftimeout=500;
				}
				break;
			case 5:
				if (buttonstate==0) {
					PA &= 0x3f; // flashlight debug signal on
					fstate=6;
					ftimeout=50000;
				}
				break;
			case 6:
				if (buttonstate==1) {
					PA |= 0x40; // debug flashlight signal off
					fstate=3;
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
					ftimeout=500;
				} else if (ftimeout==0) {
					PA |= 0x40; // debug flashlight signal off
					fstate=7;
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
					ftimeout=100;
				}
				break;
			case 7:
				if (ftimeout==0) {
					fstate=8;
					// switch flashlight on (by setting PB3 high), and all other leds off
					PA |= 0b00010001;	
					PB |= 0b01101101;	
					ftimeout=2000;
				}
				break;
			case 8:
				if (buttonstate==1) {
					fstate=5;
				} else if (ftimeout==0) {
					fstate=0;
					// switch off the flashlight (by setting PB3 low)
					PB &=0x77;
				}
				break;	
		}
		
		
		/* we use timer 2 to generate a  38kHz sync pulse on PB4. These settings are necessary:
		* IHRC 16MHz, 16000000/422=37.914 KHz
		* TM2C [7:4]=0010 (select IHRC); [3:2]=11 (output on PB4); [1] = 0 (period mode); [0] = 0 (do not invert);
		* TM2S [7] = 0 (8 bit resolution); [6:5]=00 (prescaler 1); [4:0]=00000 (scaler 1);
		* TM2B [7:0] -> 211
		*/

		switch (dstate) {
			case 0: // waiting for sync or timeout
				if (dtimeout==0) {
					dstate=1;
					dtimeout=10;
					// start transmitting an IR pulse of 10ms
					PA &= 0x77; // debug IR signal
					TM2C=0; // stop timer
					TM2CT=0;
					TM2B=211;
					TM2S=0; // clear the counter
					TM2C=0b00101100; // go!
				} 
				else if (IRrecstate == 0x01) { // IR pulse received!
					dstate=4;
					ttimeout=65535;
					dtimeout=50; // wait until pulse complete and originator listening again: 50 ms
				}
				break;
			case 1: // transmitting sync
				if (dtimeout==0) {
					dstate=2;
					dtimeout=25; // completely deaf for 10 ms
					// stop transmitting the IR pulse	
					PA |= 0x08; // IR debug signal
					TM2C=0; 	// stop PWM
					PB &= 0x6f; 	// make sure IR LED is off
				}
				break;
			case 2: // completely deaf
				if (dtimeout==0) {
					dtimeout=100; // listening but not resyncing for 100 ms
					dstate=3; 
				}
				break;
			case 3: // listening but not resyncing
				if (dtimeout==0) {
					dtimeout=30000; 
					dstate=0;
				} else if (IRrecstate == 0x01) { // IR pulse received
					ttimeout=65535;
				}
				break;
			case 4:	// 50ms deaf after spontaneously received sync
				if (dtimeout==0) { 
					dstate=5;
					dtimeout=10;

					// start transmitting a 10ms IR pulse
					PA &= 0x77; // debug IR signal
					TM2C=0; // stop timer
					TM2CT=0;
					TM2B=211;
					TM2S=0; // clear the counter
					TM2C=0b00101100; // go!
				}
				break;
			case 5: // transmitting sync
				if (dtimeout==0) {
					dstate=6;
					dtimeout=75; // become completely deaf for 75 ms
					// stop transmitting the IR pulse
					PA |= 0x08; // debug IR signal
					TM2C=0; 	// stop PWM
					PB &= 0x6f; 	// make sure IR LED is off
				}
				break;	
			case 6: // completely deaf
				if (dtimeout==0) {
					dtimeout=30000; // listening but not resyncing for 100 ms
					dstate=0; 
				}
				break;

		}
		
		
		if ((PB & 0x08)==0) {// the flashlight is OFF
			// get the current state of the non-RGB-LED pins so we can compute the NEXT state without affecting any other output pins
			PA_next=(PA & 0x7F) | 0x11;
			PB_next=(PB & 0X7F) | 0x65;

			if (ttimeout) { // trinket is together
				uint8_t phase=(pattern>>9) & 0x0f;
				uint8_t subphase=pattern & 1;
				switch (phase)
				{
					case 0:
						if (subphase) { R1_ON;}
						break;
					case 1:
					
						if (subphase) { G2_ON; } else { B2_ON; }
						break;
					case 2:
						if (subphase) { R1_ON; } else { G1_ON; }
						break;
					case 3:
						if (subphase) { B2_ON; }
						break;
					case 4:
						if (subphase) { G1_ON; }
						break;
					case 5:
						if (subphase) { R2_ON; } else { B2_ON; }
						break;
					case 6:
						if (subphase) { G1_ON; } else { B1_ON; }
						break;
					case 7:
						if (subphase) { R2_ON; }
						break;
					case 8:
						if (subphase) { B1_ON; }
						break;
					case 9:
						if (subphase) { R2_ON; } else { G2_ON; }
						break;
					case 10:
						if (subphase) { R1_ON; } else { B1_ON; }
						break;
					case 11:
						if (subphase) { G2_ON; }
						break;
					default:
						//should never happen - off
						break;
				}
			} else { // trinket is alone 
				if ((pattern & 0x07f8)==0x0400) { // just blink RGB leds for 8 ms
					switch (pattern & 3) {
						case 0: R1_ON; G2_ON; break;
						case 1: G1_ON; B2_ON; break;
						case 2: B1_ON; R2_ON; break;
						case 3: break;
					}
				}
			}
			
			// implement the changes to the RGB leds, leaving the other outputs unaffected
			PA=PA_next;
			PB=PB_next;
		}
	}
}






/*******************************************************************************
* main program - consists of setup and ... nothing
*/
void main(void) {
	// Initialize hardware:
	//PA0-6 are ACTIVE LOW, so set them HIGH before making them outputs
	//PB0-2 & PB5-6 are ACTIVE LOW, so set them HIGH before making them outputs
	PA=0x7f;	
	PB=0x67;	
	
  	//ENABLE pull-ups on PA5, PA7, PB7, DISABLE on all other pins
	PAPH = 0xA0; // 10100000
	PBPH = 0x80; // 10000000
	
	// PA0-6 and PB0-6 are OUTPUTS
	PAC=0x7f;
	PBC=0x7f;
  	

	// initialize variables
	dstate=0; 	// the together/alone communication state machine
	ttimeout=0; 	// the timeout counter for the together/alone detection
	dtimeout=60000; // the timeout counter for detection states
	pattern=0; 	// the pattern counter that determines what is shown on the RGB leds

	fstate=0; 	// the state of the flashlight state machine
	ftimeout=0; 	// the timeout counter for the flashlight
	
	buttonshift=0xff; // the state of the button as read from the pin during the last 8 cycles
	buttonstate=0; // the state of the button after debouncing and inverting

	// setup and start the timer
	setup_ticks();
	INTRQ = 0;
	__engint();                     // Enable global interrupts
	
	
	
	/* we have nothing to do inside the while(1) loop as everything is basically handled in
	 * an interrupt, so we use __stopexe() to stop the processor until an interrupt occurs.
	 * So we are free to do some debugging. First thing is to toggle a LED on a spare pin
	 * once a second to have an indication that the processor is up and running AND that the
	 * interrupt actually happebs. So we count to 500 in the while(1) loop using ctr and invert
	 * the diagnostic LED pin B1 (which is normally not connected)
	 */
	 
	uint16_t ctr=0;
	
	while (1) {
		ctr++;
		if (ctr>=500) {
			PB=PB ^ 0x02; // PB1 is used
			ctr=0;
		}
		__stopexe(); // low power sleep until next timer interrupt
	}
}

