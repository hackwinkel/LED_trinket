# (c) 2025 Theo Borm
# See LICENSE FILE
module penliteholder() {
    translate([4,0,9.5]) rotate([0,90,0]) cylinder(50,7.5,7.5, $fn=100);
     translate([0,-8.5,0]) cube([58,17,13.5]);
}


module flashlightbody(d,h,c,t) {
    difference() {
        union() {
            intersection() {
                translate([-d/2,-d/2,0]) cube([d,d,h+d]);
                union() {
                    cylinder(h,d/sqrt(2)-c,d/sqrt(2)-c,$fn=100);
                    translate([0,0,h]) sphere(d/sqrt(2)-c,$fn=100);
                }
            }
            translate([-2.5,-d/2,h]) cube([5,d,d]);
            translate([-2.5,0,h+d]) rotate([0,90,0]) cylinder(5,d/2,d/2,$fn=100);
        }
        translate([-5,0,h+d]) rotate([0,90,0]) cylinder(10,4,4,$fn=50);
        intersection() {
            translate([t-d/2,t-d/2,-t]) cube([d-t-t,d-t-t,h+d]);
            translate([0,0,-t]) union() {
                cylinder(h,d/sqrt(2)-c-t,d/sqrt(2)-c-t,$fn=100);
                translate([0,0,h]) sphere(d/sqrt(2)-c-t,$fn=100);
            }
        }
        translate([0,0,8]) rotate([0,90,0]) cylinder(20,4,4,$fn=50);
    }
    translate([-d/2-0.01,-3.5,5]) rotate([0,-90,0]) linear_extrude(0.4) text("hackwinkel.nl",7, font="DejaVu Sans:style=Bold");
}


//w=wide part, n= narrow part d=diameter, h=height, o=offset, c=corner inset, t=thickness
module flashlighthead(dw,dn,hw,hn,c,t,tt) {
    difference() {
        
        // the outline consists of a neck and a head
        union() {
            //neck
            intersection() {
                translate([-dw/2,-dw/2,0]) cube([dw,dw,hw]);
                cylinder(hw,dw/sqrt(2)-c,dw/sqrt(2)-c,$fn=100);
            }
            // head
            intersection() {
                translate([-dn/2,-dn/2,0]) cube([dn,dn,hn]);
                cylinder(hn,dn/sqrt(2)-c,dn/sqrt(2)-c,$fn=100);
            }
        }
        
        //beveling around bottom
        rotate([0,0,0]) translate([0,dw/2-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,90]) translate([0,dw/2-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,180]) translate([0,dw/2-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,270]) translate([0,dw/2-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,45]) translate([0,dw/sqrt(2)-c-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,135]) translate([0,dw/sqrt(2)-c-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,225]) translate([0,dw/sqrt(2)-c-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        rotate([0,0,315]) translate([0,dw/sqrt(2)-c-1,0]) rotate([-45,0,0]) translate([-50,0,-50]) cube([100,100,100]);
        
        // remove the core to create a tube, t=thickness
        translate([0,0,5]) intersection() {
            translate([t-dn/2,t-dn/2,0]) cube([dn-t-t,dn-t-t,hn+2]);
            cylinder(hn+2,dn/sqrt(2)-c-t,dn/sqrt(2)-c-t,$fn=100);
        }
        
        // make a section of the wide part thinner to make it translucent
        translate([0,0,5]) intersection() {
            translate([tt-dw/2,tt-dw/2,0]) cube([dw-tt-tt,dw-tt-tt,hw-10]);
            cylinder(hw-10,dw/sqrt(2)-c-tt,dw/sqrt(2)-c-tt,$fn=100);
        }
        
        
        //button hole in the middle pf the neck part 
        translate([0,0,hw+(hn-hw)/2]) rotate([0,90,0]) cylinder(20,4,4,$fn=50);
        
        
        //drill-through holes for white 5mm LEDs
        translate([-3,-3,0.2]) cylinder(10,2.75,2.75,$fn=50);
        translate([3,-3,0.2]) cylinder(10,2.75,2.75,$fn=50);
        translate([-3,3,0.2]) cylinder(10,2.75,2.75,$fn=50);
        translate([3,3,0.2]) cylinder(10,2.75,2.75,$fn=50);
        
        translate([-3,-3,2.1]) cylinder(3,1,3.5,$fn=50);
        translate([3,-3,2.1]) cylinder(3,1,3.5,$fn=50);
        translate([-3,3,2.1]) cylinder(3,1,3.5,$fn=50);
        translate([3,3,2.1]) cylinder(3,1,3.5,$fn=50);
 
        //slit for the PCB
        translate([-1,-(dw-tt-tt)/2,5]) cube([2,dw-tt-tt,100]);
    }
}

/*
module button() {
    difference()
        intersection()
        translate([0,0,-4]) cylinder(10,0,20)
*/


// was 19.5, now 20mm wide
//for (i = [0:2]) {
//    for (j = [0:2]) {
        //translate([i*40,j*40,0])
            //flashlightbody(d,h,c,t)
            //flashlightbody(20,75,1.5,0.8);
//    }
//}
    
//


// part that slides into body = 1.7mm smaller than body. 
// body skin thickness= 0.8mm, 2*0.8=1.6mm, so 0.1mm for "fit"

// w=wide part, n= narrow part d=diameter, h=height, o=offset, c=corner inset, t=thickness
// flashlighthead(dw,dn,hw,hn,c,t,tt) {

// head 20mm wide, neck 18.2mm wide, head 42.5mm high, neck an extra 15mm
// thickness in the neck 1mm, thickness in the thin part 0.8mm
for (i = [0:2]) {
    for (j = [0:1]) {
        translate([i*40,j*40,0])
        flashlighthead(20,18.2,42.5,57.5,1.5,1,0.8);
    }
}
