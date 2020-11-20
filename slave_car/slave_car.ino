#include <SoftwareSerial.h>
#include <Servo.h>
#define RxD 7 //should be RX of bluetooth 
#define TxD 6 //should be Tx of bluetooth 
#define ConnStatus A1 //should be connstatus of shield 
#define DEBUG_ENABLED 1
#define servo_r 13//should be right servo pin 
#define servo_l 12//should be left servo pin 
#define delay_manual 100 //should be appropriate delay time same as master_joystick 

//setting bluetooth
int shieldPairNumber = 3; //should be shiedl number 
boolean ConnStatusSupported = true; 
String slaveNameCmd = "\r\n+STNA=Slave";



//setting Servo 
Servo servoLeft; 
Servo servoRight;

//integer holding the drive mode  
int dir = 0; 

//mode 
int mode = 0;

//bluetooth serial
SoftwareSerial BTSerial(RxD,TxD);


void setup() {
    //seting Serial
    Serial.begin(9600);
    BTSerial.begin(9600);                   

    //setting pin for Bluetooth 
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    pinMode(ConnStatus, INPUT);

    //sounds the buzzer
    tone(4,3000, 1000); 
    delay(1000);
    
    pinMode(10, INPUT);  pinMode(9, OUTPUT);   // Left IR LED & Receiver
    pinMode(3, INPUT);  pinMode(2, OUTPUT);    // Right IR LED & Receiver

    //setting bluetooth 
    if(ConnStatusSupported) Serial.println("Checking Slave-Master connection status.");
    if(ConnStatusSupported && digitalRead(ConnStatus)==1){
        Serial.println("Already connected to Master - remove USB cable if reboot of Master Bluetooth required.");
    }
    else{
        Serial.println("Not connected to Master.");
        setupBlueToothConnection(); 
        delay(1000);                  
        Serial.flush();
        BTSerial.flush();
    }

    //setting Servo
    servoLeft.attach(servo_l); 
    servoRight.attach(servo_r);
}

void loop() {  
  if (mode ==0){//bluetooth pairing mode 
    char recvChar;
    bool val = true; 
    while(val)
    {

        if(Serial.available())            // Check if there's any data sent from the local serial terminal. You can add the other applications here.
        {
            recvChar  = Serial.read();
            Serial.print(recvChar);
            BTSerial.print(recvChar);
        }
        
        if(BTSerial.available())   // Check if there's any data sent from the remote Bluetooth shield
        {
            recvChar = BTSerial.read();
            Serial.print(recvChar);
            if(recvChar == "8"){    //should check if this works 
              mode = mode +1; 
              delay(3000); 
              val = false; 

            }
        }
        
    }
  }
  /////////////////////////////////////
  else if(mode ==1){ //manual-drive-go mode 
    if(BTSerial.available()){ 
      dir = BTSerial.read();
      delay(1); 
    }
    //changing the mode 
    if(dir == 8){
      mode = mode +1;
      delay(3000); 
    }
    else{
      manual_go(dir);
    }
  }
  //////////////////////////////////////
  else if(mode ==2){//line-tracing-go mode 
    if(BTSerial.available()){ 
      BTSerial.read();
      delay(1); 
      mode=mode+1; 
      delay(3000); 
    }
    else{ //if there is no input from the joystick 
      tracing_go(); 
    }
  }
  ///////////////////////////////////////
  else if(mode ==3){ //found-the-target mode 
    //car_stop();
    if(BTSerial.available()){
      BTSerial.read();
      delay(1); 
      mode=mode+1; 
      delay(3000); 
    }
  }
  /////////////////////////////////////
  else if (mode ==4){//turn-the-car mode 
   if(BTSerial.available()){ 
   dir = BTSerial.read();
   delay(1); 
   }
   
   //changing the mode 
   if(dir == 8){
     mode = mode +1;
     delay(3000); 
   }
   else{
     manual_go(dir);
   }
  }
  /////////////////////////////////////
  else if(mode ==5){ //tracking-drive-return mode 
    if(BTSerial.available()){
      BTSerial.read();
      delay(1); 
      mode=mode+1; 
      delay(3000); 
    }
    else{
      tracing_go();
    }
  }
  ////////////////////////////////////
  else if(mode ==6){ //manual-drive-return mode 
    if(BTSerial.available()){ //if there is input in the bluetooth 
      dir = BTSerial.read();
      delay(1); 
    }
    //changing the mode 
    if(dir == 8){
      mode = mode +1;
      delay(3000); 
    }
    else{
      manual_go(dir);
    }
  }
  ////////////////////////////////////
  else if(mode ==7){ //do-nothing mode 
    if(BTSerial.available()){
      BTSerial.read();
      delay(1); 
      mode=mode+1; 
      delay(3000); 
    }
  }
  ///////////////////////////////////
  else if(mode ==8){ //manual-drive-go-memorized mode 
    if(BTSerial.available()){ 
    dir = BTSerial.read();
    delay(1); 
    }
    //changing the mode 
    if(dir == 8){
      mode = mode +1;
      delay(3000); 
    }
    else{
      manual_go(dir);
    }
   }
  //////////////////////////////////
  else if(mode ==9) { //tracking-drive-go mode 
    if(BTSerial.available()){
      BTSerial.read();
      delay(1); 
      mode=mode+1; 
      delay(3000); 
    }
    else{
    tracing_go();
    }
  }
  ///////////////////////////////////
  else if (mode==10){
    //finished
  }
  
}


//manual-drive-go mode 
void manual_go(int dir){
  if(dir == 0){
    car_stop();
  }
  else if(dir==1){
    forward();
  }
   else if(dir==2){
    forward_right();
  }
  else if(dir==3){
    forward_left();
  }
  else if(dir==4){
    backward();
  }
 delay(delay_manual);
}



void car_stop(){
  maneuver( 30, -25, 0);
}
void forward(){
  maneuver( 200, 200, 0);
}
void forward_right(){
  maneuver( 200, -25, 20);
}
void forward_left(){
  maneuver( 30, 200, 20); 
}
void backward(){
  maneuver( -200, -200, 20);
}



//tracking
void tracing_go(){
  int irLeft = irDetect(9, 10, 38000);       // Check for object on left
  int irRight = irDetect(2, 3, 38000);       // Check for object on right

  Serial.print(irLeft);                      // Display 1/0 no detect/detect
  Serial.print("  ");                        // Display 1/0 no detect/detect
  Serial.println(irRight);                   // Display 1/0 no detect/detect

  if((irLeft == 1) && (irRight == 1)){        // If both sides are white  
    maneuver(30, -25, 50);
    for(int i = 0; i < 3; i++){               // Repeat 5 times
      tone(4, 4000, 10);                     // Sound alarm tone
      delay(20);                             // 10 ms tone, 10 between tones
    }
    Serial.println("Find The Target.");                 
  }
  else if((irLeft == 1) && (irRight == 0)){   // If only left side is white
    maneuver(200, -25, 20);                  // Right for 20 ms
  }
  else if((irLeft == 0) && (irRight == 1)){   // If only right side is white
    maneuver(30, 200, 20);                    // Left for 20 ms
  }
  else{                                      // Otherwise, both detects black
    maneuver(200, 200, 20);                  // Forward 20 ms
  }
}

int irDetect(int irLedPin, int irReceiverPin, long frequency){
  tone(irLedPin, frequency, 8); 
  delay(1);
  int ir = digitalRead(irReceiverPin);
  delay(1);
  return ir;
}

void maneuver(int speedLeft, int speedRight, int msTime){
  // speedLeft, speedRight ranges: Backward  Linear  Stop  Linear   Forward
  //                               -200      -100......0......100       200
  servoLeft.writeMicroseconds(1500 + speedLeft);   // Set left servo speed
  servoRight.writeMicroseconds(1500 - speedRight); // Set right servo speed
  if(msTime==-1){                                   // if msTime = -1                                
    servoLeft.detach();                            // Stop servo signals
    servoRight.detach();   
  }
  delay(msTime);                                   // Delay for msTime
}




//setting bluetooth
void setupBlueToothConnection()
{
    Serial.println("Setting up the local (slave) Bluetooth module.");
    slaveNameCmd += shieldPairNumber;
    slaveNameCmd += "\r\n";
    BTSerial.print("\r\n+STWMOD=0\r\n");      
    BTSerial.print(slaveNameCmd);             
    BTSerial.print("\r\n+STAUTO=0\r\n");      
    BTSerial.print("\r\n+STOAUT=1\r\n");      
    BTSerial.flush();
    delay(2000);                                     
    BTSerial.print("\r\n+INQ=1\r\n");         
    BTSerial.flush();
    delay(2000);                                     
    Serial.println("The slave bluetooth is inquirable!");
}
