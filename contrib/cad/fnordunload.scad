module elrdcyl(
   w, // width of cylinder
   d, // depth of cylinder
   h1,// straight height of cylinder
   h2 // height of rounded top
   ) {
   intersection(){
     union(){
       scale([w/2,d/2,1])cylinder(r=1,h=h1,$fn=100);  // cylinder
       translate([0,0,h1])scale([w/2,d/2,h2])sphere(r=1,$fn=100);  // top
     }
     scale([w/2,d/2,1])cylinder(r=1,h=h1+h2,$fn=100); // only needed if h2>h1 
   }
}
difference(){
    union(){
        difference(){
            translate([-50,-0,1]){
            cube([100,30,30]);
            }
            elrdcyl(80,90,10,30);
 
        }
        difference() {
            translate([0,0,1]){
                elrdcyl(90,100,05,40);
            }
           // translate([50,10,-1]){
                elrdcyl(80,90,10,30);
           // }

        }
    }
    translate([-50,-100,0]){
            cube([100,100,100]);
    }
                translate([-45,30,5]){
                rotate ([90,0,0]) cylinder (h = 200, r=2, center = true, $fn=100);
            }   
             translate([-45,30,25]){
                rotate ([90,0,0]) cylinder (h = 200, r=2, center = true, $fn=100);
            }   
             translate([45,30,5]){
                rotate ([90,0,0]) cylinder (h = 200, r=2, center = true, $fn=100);
            }   
             translate([45,30,25]){
                rotate ([90,0,0]) cylinder (h = 200, r=2, center = true, $fn=100);
            }  
}
