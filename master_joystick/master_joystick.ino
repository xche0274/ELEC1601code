#include <SoftwareSerial.h>
#define RxD 7  //should be Rx 
#define TxD 6 //should be Tx 
#define ConnStatus A1 //should be A1 
#define DEBUG_ENABLED 1
#define joy_X A2 //should be VRX of joystick
#define joy_Y A3 //should be VRY of joystick 
#define button 4 //should be SW of joystick 
#define delay_manual 100 //should be appropriate delay time for manual drive 
#define instruction_size 200 //should be appropriate array size for int storing our instructions

//setting  bluetooth 
int shieldPairNumber = 3; 
boolean ConnStatusSupported = true;
String slaveName = "Slave";                  
String masterNameCmd = "\r\n+STNA=Master";   
String connectCmd = "\r\n+CONN=";            
int nameIndex = 0;
int addrIndex = 0;
String recvBuf;
String slaveAddr;
String retSymb = "+RTINQ=";   

//setting joystick value
int axisX;
int axisY;

//setting manual drive
int temp = 0;
int temp_instruction =0;
int instructions[instruction_size]; 

//index of instructions 
int index = -1; 

//setting the drive mode 
int mode = 0; 

//bluetooth connection 
SoftwareSerial BTSerial(RxD,TxD);

void setup() {
  //setting Serial 
  Serial.begin(9600);
  BTSerial.begin(9600); 

  //setting Serial for bluetooth 
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  pinMode(ConnStatus, INPUT);

  
  //setting bluetooth 
  if(ConnStatusSupported) Serial.println("Checking Master-Slave connection status.");
  if(ConnStatusSupported && digitalRead(ConnStatus)==1){
      Serial.println("Already connected to Slave - remove USB cable if reboot of Master Bluetooth required.");
  }
  else{
      Serial.println("Not connected to Slave.");
      setupBlueToothConnection();     // Set up the local (master) Bluetooth module
      getSlaveAddress();              // Search for (MAC) address of slave
      makeBlueToothConnection();      // Execute the connection to the slave
      delay(1000);                    // Wait one second and flush the serial buffers
      Serial.flush();
      BTSerial.flush();
  }

  
  //setting joystick button 
  pinMode(button, INPUT_PULLUP);
   
}



void loop() {
  if (digitalRead(button)==0){ //when joystick is pushed
    mode = mode + 1; 
    temp_instruction = 8; 
    BTSerial.write(temp_instruction);
    delay(1);
    Serial.println("changing to"+ String(mode)+"mode in 3 sec");
    delay(3000);
  }

  
  if (mode == 0){//bluetooth-pairing mode 
     char recvChar;
     bool val = true; 
     while(val)
     {
       if(BTSerial.available())   // Check if there's any data sent from the remote Bluetooth shield
       {
           recvChar = BTSerial.read();
           Serial.print(recvChar);
       }

       if(Serial.available())            // Check if there's any data sent from the local serial terminal. You can add the other applications here.
       {
           recvChar  = Serial.read();
           Serial.print(recvChar);
           BTSerial.print(recvChar);
       }
       if (digitalRead(button)==0){ 
           mode = mode + 1; 
           val = false; 
           temp_instruction = 8; 
           BTSerial.write(temp_instruction);
           delay(1);
    
           Serial.println("changing to"+ String(mode)+"mode in 3 sec");
           delay(3000);
       }
     }
  }
  else if(mode ==1){ //manual-drive-go mode
    manual_drive(); 
  }
  else if(mode ==2){ //tracking-drive-go mode 
    //do nothing
  }
  else if(mode ==3){ //found-the-target mode 
    //do nothing 
  }
  else if(mode==4){ //turn-the-car mode 
    manual_turn();
    
  }
  else if(mode ==5){ //tracking-drive-return mode 
    //do nothing
  }
  else if(mode ==6){ //manual-drive-return mode 
    for(int i =index; i>=0; i--){
      BTSerial.write(instructions[i]);
      delay(1); 
      delay(delay_manual);  
    }
    mode = mode +1; 
    temp_instruction = 8; 
    BTSerial.write(temp_instruction);
    delay(1);
    Serial.println("changing to"+ String(mode)+"mode in 3 sec");
    delay(3000);
  }
  else if(mode ==7){ //do-nothing mode 
    //do nothing 
  }
  else if(mode ==8) {//manual-drive-go-memorized mode 
    for(int i =0; i<=index; i++){
      BTSerial.write(instructions[i]);
      delay(1); 
      delay(delay_manual);  
    }
    mode = mode +1; 
    temp_instruction = 8; 
    BTSerial.write(temp_instruction);
    delay(1);
    Serial.println("changing to"+ String(mode)+"mode in 3 sec");
    delay(3000);
  }
  else if(mode==9){ //tracking-drive-go mode 
    //do nothing 
  }
  else if(mode ==10){//finished
    //do nothing
  }
  

}

//manual-drive-go function
void manual_drive(){
  index = index +1; 
  axisX = map(analogRead(joy_X), 0, 1023, -512, 511); //should be setted according to the joystick 
  axisY = map(analogRead(joy_Y), 0, 1023, -512, 511); //should be setted according to the joystick 
  temp = sqrt(3)*axisX;

  //stop
  if(axisY<=-20&&axisY>=-30&&axisX<=0&& axisX>=-10){//should be setted according to the joystick stop
    temp_instruction=0;
    Serial.println("[0]STOP");
  }
  //go 
  //go forward 
  else if((axisY>=temp && axisY>-1*temp)){
    temp_instruction=1;
    Serial.println("[1]GO FORWARD");
  }
  //go forward right 
  else if((axisY<temp && axisY>=0) or (axisY>=-1*temp &&axisY<0)){
    temp_instruction=2;
    Serial.println("[2]GO FORWARD RIGHT");
  }
  //go forward left
  else if((axisY<-1*temp && axisY>=0) or (axisY>=temp &&axisY<0)){
    temp_instruction=3;
    Serial.println("[3]GO FORWARD LEFT");
  }
  //back
  //go backward
  else if(axisY<temp &&axisY<-1*temp){
    temp_instruction=4;
    Serial.println("[4]GO BACKWARD");
  }
  instructions[index]=temp_instruction; 
  BTSerial.write(temp_instruction);
  delay(1); //time spent on send 
  delay(delay_manual);  
}

//turn-the-car mode 
void manual_turn(){
  axisX = map(analogRead(joy_X), 0, 1023, -512, 511); //should be setted according to the joystick 
  axisY = map(analogRead(joy_Y), 0, 1023, 511, -512); //should be setted according to the joystick 
  temp = sqrt(3)*axisX;

  //stop
  if(axisY<=-20&&axisY>=-30&&axisX<=0&& axisX>=-10){//should be setted according to the joystick stop
    temp_instruction=0;
    Serial.println("[0]STOP");
  }
  //go 
  //go forward 
  else if((axisY>=temp && axisY>-1*temp)){
    temp_instruction=1;
    Serial.println("[1]GO FORWARD");
  }
  //go forward right 
  else if((axisY<temp && axisY>=0) or (axisY>=-1*temp &&axisY<0)){
    temp_instruction=2;
    Serial.println("[2]GO FORWARD RIGHT");
  }
  //go forward left
  else if((axisY<-1*temp && axisY>=0) or (axisY>=temp &&axisY<0)){
    temp_instruction=3;
    Serial.println("[3]GO FORWARD LEFT");
  }
  //back
  //go backward
  else if(axisY<temp &&axisY<-1*temp){
    temp_instruction=4;
    Serial.println("[4]GO BACKWARD");
  }
  BTSerial.write(temp_instruction);
  delay(1); //time spent on send 
  delay(delay_manual); 
}

//setting bluetooth 
void setupBlueToothConnection()
{
    Serial.println("Setting up the local (master) Bluetooth module.");
    masterNameCmd += shieldPairNumber;
    masterNameCmd += "\r\n";
    BTSerial.print("\r\n+STWMOD=1\r\n");      // Set the Bluetooth to work in master mode
    BTSerial.print(masterNameCmd);            // Set the bluetooth name using masterNameCmd
    BTSerial.print("\r\n+STAUTO=0\r\n");      // Auto-connection is forbidden here
    BTSerial.flush();
    delay(2000);                                     // This delay is required
    BTSerial.print("\r\n+INQ=1\r\n");         // Make the master Bluetooth inquire
    BTSerial.flush();
    delay(2000);                                     // This delay is required
    Serial.println("Master is inquiring!");
}


void getSlaveAddress()
{
    slaveName += shieldPairNumber;
    Serial.print("Searching for address of slave: ");
    Serial.println(slaveName);
    slaveName = ";" + slaveName;   // The ';' must be included for the search that follows
    char recvChar;
    while(1)
    {
        if(BTSerial.available())
        {  
            recvChar = BTSerial.read();
            recvBuf += recvChar;
                        
            nameIndex = recvBuf.indexOf(slaveName);   // Get the position of slave name
            
            if ( nameIndex != -1 )   // ie. if slaveName was found
            {
                addrIndex = (recvBuf.indexOf(retSymb,(nameIndex - retSymb.length()- 18) ) + retSymb.length());   // Get the start position of slave address
                slaveAddr = recvBuf.substring(addrIndex, nameIndex);   // Get the string of slave address
                Serial.print("Slave address found: ");
                Serial.println(slaveAddr);
                break;  // Only breaks from while loop if slaveName is found
            }
        }
    }
}


void makeBlueToothConnection()
{
    Serial.println("Initiating connection with slave.");
    char recvChar;
    connectCmd += slaveAddr;
    connectCmd += "\r\n";
    int connectOK = 0;       // Flag used to indicate succesful connection
    int connectAttempt = 0;  // Counter to track number of connect attempts
    do
    {
        Serial.print("Connect attempt: ");
        Serial.println(++connectAttempt);
        BTSerial.print(connectCmd);   // Send connection command
        recvBuf = "";
        while(1)
        {
            if(BTSerial.available())
            {
                recvChar = BTSerial.read();
                recvBuf += recvChar;
        
                if(recvBuf.indexOf("CONNECT:OK") != -1)
                {
                    connectOK = 1;
                    Serial.println("Connected to slave!");
                    BTSerial.print("Master-Slave connection established!");
                    break;
                }
                else if(recvBuf.indexOf("CONNECT:FAIL") != -1)
                {
                    Serial.println("Connection FAIL, try again!");
                    break;
                }
            }
        }
    } while (0 == connectOK);
}
