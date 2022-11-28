/* By Julien Foucher
*  
*  Reference i used to learn and start this work:
*https://create.arduino.cc/projecthub/electropeak/use-an-ir-remote-transmitter-and-receiver-with-arduino-1e6bc8?ref=search&ref_id=REMOTE%20IR&offset=51 
*/     
#include <IRremote.h>     
int RECV_PIN =6;     
int pinVout=32;    
IRrecv irrecv(RECV_PIN);     
decode_results results; 
//---------------------------------//
//---------------------------------//
int EN_X    =  38;      // using the x stepper driver on the ramps 1.4 to move Z axis! 
//int EN_Y  =  A2;
//int EN_Z  =  A8;
int X_DIR   =  A1;      // using the x stepper driver on the ramps 1.4 to move Z axis!
//int Y_DIR =  A7;     
//int Z_DIR =  48;   
int X_STP   =  A0;      // using the x stepper driver on the ramps 1.4 to move Z axis!   
//int Y_STP =  A6;  
//int Z_STP =  46;
int estop_Z =   3;
int zStart=true; 
int zStepsQty;
int zPos;
int maxZ=16000;
int minZ=0;
int stepsCount=0;
int vitesseZ;
int const vitZdefault=400; // vitZdelay is a delay ex.: 80=fast 400=slower       
int actionState=1; /*  (1) Initialisation && Z_home
                    *  (2) Waiting order
                    *  (3) Activate motion++
                    *  (4) Activate motion--
                    *  (5) Home Z to max
                    *  (6) None...
                    *  (7-8)Up/Down Routine 
                    */   
  
    //-----------------------------------//
    //-----------------------------------//
  
void setup(){     
  Serial.begin(9600);     
  irrecv.enableIRIn();     
  pinMode(pinVout,OUTPUT);
  digitalWrite(pinVout,HIGH); 
  vitesseZ=vitZdefault; 
   
 
     //---------------------------------//
     //---------------------------------// 
  pinMode(X_DIR, OUTPUT); pinMode(X_STP, OUTPUT);
 // pinMode(Y_DIR, OUTPUT); pinMode(Y_STP, OUTPUT);
 // pinMode(Z_DIR, OUTPUT); pinMode(Z_STP, OUTPUT);
  pinMode(EN_X, OUTPUT);
  digitalWrite(EN_X, LOW);
  pinMode(estop_Z,INPUT); 
     //----------------------------------//
     //----------------------------------//
  }     
void loop(){

   int lol = digitalRead(estop_Z);     
 
 //---------IR Remote activated command -----------//
  z_Home(); 
    moveUpIf();
     moveDownIf();
      resetHomeZ();
        ZToMaz();
          up();
            down();
 //------------IR Case/fonction--------------------//
  if (irrecv.decode(&results)){     
    int value = results.value;     
    Serial.println(value);
    
    switch(value)
  { 
//--------"Move to Z-Max (Left/vol-)"-------------------//    
     case 8925:       
         actionState=3; //chose a motion
           zStepsQty=maxZ; //number of steps to count
              break; 
//----------"Move Down 1/4(Right/vol+)"---------------------//          
     case 765: 
         actionState=4; //chose a motion
           zStepsQty=4000; //number of steps to count
             break;   
//--------Move Up 1/4(Play/Pause)---------------------//            
     case -15811:         
         actionState=3; //choose a motion
           zStepsQty=4000; //number of steps to count
             break;
//-------"Home to Z-Min (ch-)"-----------------------------//          
     case -23971: 
         actionState=1;
           zStart=true;
             break;
//-------"VitesseZ up +10 steps (+)"--------------//          
     case -22441:   
         if (vitesseZ>=90){ 
           vitesseZ=vitesseZ-10;}    // vitesse is a delay so substraction from delay = addition to speed
             break;
//-------"vitesseZ Down -10 steps (-)"----------------//          
     case -8161: 
         if (vitesseZ <=2400){ 
           vitesseZ=vitesseZ+10;}
             break;
//--------"vitesseZ to Default (EQ)"--------------//          
     case -28561: 
           vitesseZ=vitZdefault;
             break;
//--------"Home Z-Max (ch+)"------------------------------//          
      case -7651: 
         actionState=5; //choose a motion
               break;

//--------"Up/Down routine (ch)"--------------------//          
      case 25245: //Keypad(0)  
         actionState=7;
         zStepsQty=16000;//choose a motion
               break;
  }            
        
           
      irrecv.resume();  // Get ready to receive again
       /*
    * button    lrft      (8925)     ch-   (-23971)    1   (12495)   6   (23205) 
    *           right       (765)      ch    (25245)     2   (6375)    7   (17085)
    *           play/pause (-15811)   ch+   (-7651)     3   (31365)   8   (19125)
    *           -          (-8161)    100+  (-26521)    4   (4335)    9   (21165)  
    *           +          (8925)     200+  (-20401)    5   (14535)   0   (26775) <
    *           EQ      (-28561)
    */      
  } 
//----------------------------------------------------------//  
//--------Actuate those... ...before ending loop------------//
//----------------------------------------------------------//    
}
//----------------------------------------------------------//  
//----------------------------------------------------------//  
void step(boolean dir, byte dirPin, byte stepperPin, int steps,int vit)
  {
     digitalWrite(dirPin, dir);
     for (int long i = 0; i < steps; i++) { // count number of steps until = to steps value
      digitalWrite(stepperPin, HIGH);
        delayMicroseconds(10);              //delay(b): time stepperPin is HIGH or (delayMicroseconds(vit); 
         digitalWrite(stepperPin, LOW);
          delayMicroseconds(vit);           //delay(c): variable delay to adjust speed
  }
  }
  
void moveUpIf()
  {
    int lol = digitalRead(estop_Z);
    
    if (lol==1 && actionState==3 ){
      if(stepsCount<zStepsQty && zPos<maxZ){
      step(true, X_DIR, X_STP, 1,vitesseZ);
        zPos++;
        stepsCount++;}
    else{
      actionState=2; // if()=false: deactivate motion state/return to waiting State
      stepsCount=0;
  }
  }
 
  }
  void moveDownIf()
  {
    int lol = digitalRead(estop_Z);
    
    if (lol==1 && actionState==4 && zPos>minZ){
      if(stepsCount<zStepsQty ){
        step(false, X_DIR, X_STP, 1,vitesseZ);
         zPos--;
          stepsCount++;}
    else{
      actionState=2; // if()=false: deactivate motion state/return to waiting State
      
      stepsCount=0;
  }
  }
 
  }

void z_Home()
  {
    int lol = digitalRead(estop_Z);
  
    if (lol==1 &&  actionState==1 && zStart==true){
     step(false, X_DIR, X_STP, 1,200);
     
  }
    if( lol !=1 && actionState==1){
     step(true, X_DIR, X_STP, 120,4800);
        actionState=2;
        zPos=0;
        stepsCount=0;
        zStart=false;
  } 
  // Z EndStop after the first z home //
    
    if( lol !=1 && zStart==false){
     step(true, X_DIR, X_STP, 120,4800);
     actionState=2;
     zPos=0;
     stepsCount=0; } 
       
 }    
  
  void ZToMaz()       // Home Z to max
  {
    int lol = digitalRead(estop_Z);
    
    if (lol==1 && actionState==5 ){
      if(zPos<=maxZ){
      step(true, X_DIR, X_STP, 1,200);
      zPos++;
      stepsCount=0;}
      
    else{
      actionState=2; // if()=false: deactivate motion state/return to waiting State
      stepsCount=0;
  }
  }
 
  }
void resetHomeZ()
  {
    int lol = digitalRead(estop_Z);
    
    if(lol!=1 && actionState==2){
      actionState=1;}
  
  }
  void up()
  {
    int lol = digitalRead(estop_Z);
     if (lol==1 && actionState==7 ){
        if(stepsCount<=zStepsQty ){
         step(true, X_DIR, X_STP, 1,vitesseZ);
         zPos++;
         stepsCount++;
       }
      
    else{
      actionState=8; // if()=false: deactivate motion state/return to waiting State
      stepsCount=0;
  }
  }
 
  }
  void down()
  {
    int lol = digitalRead(estop_Z);
      if (lol==1 && actionState==8 ){
       if(stepsCount<=zStepsQty ){
         step(false, X_DIR, X_STP, 1,vitesseZ);
         zPos--;
         stepsCount++;
       }
      
    else{
      actionState=7; 
      stepsCount=0;
  }
  }
 
  }



 
 
