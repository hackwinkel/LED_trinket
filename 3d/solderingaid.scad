#(c)2025 Theo Borm
#see LICENSE file
difference() {
    cube([100,100,7]);
    translate([41,24.5,5.5]) cube([18,50.5,5]);
    translate([43,26,-1]) cube([14,47,10]);
    translate([41,25,-1]) cube([18,15,10]);
    translate([41,63,-1]) cube([10,7,10]);
    translate([-5,-5,2]) rotate([0,-7,0]) cube([110,110,100]);
    translate([105,105,2]) rotate([0,-7,180]) cube([110,110,100]);
    
    translate([47,7,-1]) cylinder(10,2.75,2.75,$fn=100);
    translate([53,7,-1]) cylinder(10,2.75,2.75,$fn=100);
    translate([47,13,-1]) cylinder(10,2.75,2.75,$fn=100);
    translate([53,13,-1]) cylinder(10,2.75,2.75,$fn=100);
}

translate([50,0,0]) cube([10,3.5,12.5]);
translate([40,0,0]) cube([10,3.5,10]);
