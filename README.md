# trinket

The "trinket" is an electronic gimmick that reacts to the presence of other "trinket"s in the area, and contains a small flashlight.

It was conceived as a demonstration project and target for workshops.


## Folders

* 3D - contains 3D design files (SCAD) for the housing + a soldering aid

* Hardware - contains the schematic and PCB design files

* Workshop - Assembly instructions to go with the trinket soldering workshop

* Software - Firmware for the trinket



## Mixed soldering techniques workshop

The workshop around this badge teaches mixed Surface Mounted Device (SMD) soldering techniques. One side of the Printed circuit board is assembled using a stencil. solder paste and a hotplate for reflowing, the other side is manually populated. In addition there are a few Through-Hole components


## Low voltage or "flat battery" power supply

This trinket contains a boost converter to boost the (nominally 1.5V alkaline battery supply to 5V. This boost converter will start working at battery voltages above ~ 0.9V, but will also keep working until the battery voltage drops below ~ 0.7 V. This is well below the voltages at which alkaline batteries are normally considered empty.

WARNING: empty batteries can leak. Remove empty batteries. This is especially important for this circuit as it was designed to drain batteries as far as it can.


## Infrared communication
This trinket uses IR transmitters and receivers for inter-trinket communications. It uses the well known KISS protocol and should be compatible with other devices using this protocol.


## Flashlight mode
after a button press, 4 white leds light up.


## Blinking lights
When the trinket does not detect a fellow trinket in the neighborhood, it will slowly flash a white light. When another trinket is detected, it displays a pattern of colours.


## ultra low cost microcontrollers
The trinket contains a PADAUK PFS154-S16 microcontroller. The software/hardware *may* also work with a PMS150C-S16 and pin compatible controllers from other manufacturers (e.g. Nyquest tech), but this is untested.


## Programming prerequisites & instructions

You will need a PADAUK programmer to program the device. PADAUK microcontrollers use a proprietary programming algorithm.

Hardware and software for an open source programmer can be found here: https://github.com/free-pdk/easy-pdk-programmer-hardware and https://github.com/free-pdk/easy-pdk-programmer-software.

Follow the instructions provided on these pages.


To compile the software for the label you need:

1. Small Device C Compiler (SDCC) (https://sdcc.sourceforge.net/) version >=4

2. pdk-includes (https://github.com/free-pdk/pdk-includes)

3. easy-pdk-includes (https://github.com/free-pdk/pdk-includes)

3. This repository

Install SDCC as per instructions for your system


Make a containing directory to hold both this repository and the free-pdk  pdk-includes and easy-pdk-includes repositories and:

cd <your_dir>

git clone https://github.com/free-pdk/pdk-includes

git clone https://github.com/free-pdk/easy-pdk-includes

git clone https://github.com/hackwinkel/tag-software

cd tag-software

cd standard

make

make burn

This last command requires easypdkprog to be found in your $PATH. You can also run this command manually

Other make targets are: sizes (displays the sizes of various memory segments in the binary), clean and all (the default).


## alternative firmware

If you want to do something special with your trinket, please do so in an intelligent way, in an IR transmitting envelope that will NOT annoyingly interfere with other badges in your neighborhood:

1. Transmit IR codes once per minute at most

2. Transmit IR codes for one second at most

3. While not transmitting, listen for incoming IR signals, and act accordingly


Note: The PCB design does NOT support in-circuit reprogramming.
