//#include "MIDIUSB.h"

/* CONSTANTS DECLARATION */
const int pressure1 = 0;  //A0 input --> knob sensor
const uint8_t pressSwitch1 = 1; //A1 --> reset button
const uint8_t pressSwitch2 = 2; //A2 --> change mode button
const uint8_t button = 8;
const float increaseStep = 0.1;
const uint8_t maxIncrease = 120;
const int T1 = 300; // Threshold of switch 1
const int T2 = 25;  // Threshold of switch 2
const uint8_t maxControllers = 2; // Number of knobs

// VARIABLE DECLARATION
uint8_t knobValue1=0;
uint8_t knobValue2 = 0;
uint8_t controller = 0;
uint16_t speedCoefficient = 0;
uint16_t increase=0;

bool send = true;
bool pressing = true;
bool thres1 = false;
bool thres2 = false;

double val=0,val0=0,val14=0;
double valDiff=0;  
double sumAvg=0;


void setup() {
  Serial.begin(31250);
  //pinMode(rightClick, INPUT); //Digital rightClick input
  //pinMode(button, INPUT); //Reset button for knob
}

void loop() {
  //readMidi();
  //readResetButton();
  readPressureSwitch2(); // Read if change of controller is needed
  readPressureSwitch1(); // Read if reset of knob is needed

  readPressure();        // Read pressure1 value
  turning();             // Turn the knob
  if(pressing){          // Send controlMessage only when pressing
    //controlChange(0,controller,knobValue);   // Send control message
    if(controller == 0){
    controlChangeMidi(0,controller, knobValue1);
    }
    else
    controlChangeMidi(0,controller, knobValue2);    
  }
  //delay(50);
 
}

/* ---------------------- ARDUINO UNO FUNCTIONS -------------*/

// Send a control message
void controlChangeMidi(byte channel, byte control, uint8_t value){
  Serial.write(0xB0);
  Serial.write(control);
  Serial.write(value);
  //delay(100);
}
/*
void readResetButton(){
  if(digitalRead(button) == HIGH){
    knobValue = 0;
    //controlChange(0,controller,knobValue);
    if (controller == 0)
      controlChangeMidi(0,controller,knobValue1);
    else
      controlChangeMidi(0,controller,knobValue2);
  }
}*/

// Button used to reset value of knob -> when crossing threshold T1 reset knob value to 0.
void readPressureSwitch1(){
  int val = 0;
  int windowSize = 20;
  for (int i=0; i<windowSize; i++){
    val += analogRead(pressSwitch1);
  }
  val /= windowSize;
  //Serial.println(val);
  if(val > T1 && thres1 == false){
    thres1 = true;
    
    if (controller == 0){
      knobValue1 = 0;
      controlChangeMidi(0,controller,knobValue1);
    }
    else{
      knobValue2 = 0;
      controlChangeMidi(0,controller,knobValue2);
    }
  }
  if(val < T1 && thres1 == true){
    thres1 = false;
  }    
}

// Button used to change mode/controller --> when crossing threshold T2 change the controller number to the next one
void readPressureSwitch2(){
  int val = analogRead(pressSwitch2);
  if(val > T2 && thres2 == false){
    thres2 = true;
    controller = (controller + 1)%maxControllers; //Normalizes controller iteration in a loop: from 0 to maxControllers-1 (1 - maxControllers in the CC value)
    
  }
  if(val < T2 && thres2 == true){
    thres2 = false;
  }    
}

// Read pressure sensor value and adjust the increase of the knob value based on the amount of pressure applied
void readPressure(){
  // First smooth the signal using a moving window average
  int movWindow = 20;
  for(int i=0;i<15;i++){
      /*Get smooth signal*/
      for(int j=0; j<movWindow;j++)
      {
        val = analogRead(pressure1);
        sumAvg+=val;
      }
      val=sumAvg/movWindow;
      sumAvg=0;      
      if(i==0) val0=val;
      if(i==14) val14=val;
  }
  // Map the applied pressure proportionally to the maximum increase of the knob value: this is like a speed coefficient (how fast should the knob turn):
  speedCoefficient = map((int)(val14), 0, 1023, 0, maxIncrease);
  
  // Compute increase of knob value (increaseStep can be adjusted to tune the sensitivity of the increase):
  increase = (uint8_t) speedCoefficient*increaseStep;

  // When the signal drops by a 30% we assume no pressure is being applied anymore
  if(((val14-val0)/val0) < -0.3){
      pressing=false;      
  }
  
  // Turn on pressing flag when increase of pressure is detected after no pressure is applied
    if(pressing == false && (val14 - val0)>0) {
    pressing = true;
  }
}

// Set the knob value based on the increase determined by the pressure read
void turning(){
  if (pressing){
    // Here it would be nicer to work with an array of knob values and use the variable controller as an index for it. This way we would not need as many knobValue variables and if clauses as controllers needed
    if (controller == 0)
      knobValue1 = knobValue1 + increase;
    else
      knobValue2 = knobValue2 + increase;

    if (knobValue1 > 127){
      knobValue1 = 127;
    }
    if (knobValue2 > 127){
      knobValue2 = 127;
    }

  }
}

// For this we need another MIDI connection to the Rx port of the Arduino: DOES NOT WORK YET!!
/*
void readMidi(){
  int header = Serial.read();
  //Serial.println(header);
  if (header == 0xB0){
    int controller = Serial.read();
    int value = Serial.read();
    knobValue = value; // Read knobValue from software and update it here
  }  
}*/

/* ------------------------ ARDUINO LEONARDO FUNCTIONS -----------------------*/
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
/*
void controlChange(byte channel, byte control, byte value) {

  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();

}*/

/*
void readMIDI(){
  midiEventPacket_t receivedPacket;
  receivedPacket = MidiUSB.read();
  if (receivedPacket.header == 0x0B){
    int8_t value = receivedPacket.byte3; //value
  }

}
*/

/* ---------------- OLD FUNCTIONS --------------*/

/*
void switchChannel(){
  if (digitalRead(rightClick) == 'HIGH'){
    controller = controller + 1;
  }
}
*/

/* Old read pressure
void readPressure(){
  int movWindow = 20;
  for(int i=0;i<15;i++){
      for(int j=0; j<movWindow;j++){
        val = analogRead(pressure1);
        sumavg+=val;
      }
      val=sumavg/movWindow;
      sumavg=0;      
      if(i==0) val0=val;
      if(i==14) val14=val;
    }
    if((val14-val0)>0)
      valDiff= (val14-val0)*10;
    else
      valDiff= 0;

  // valDiff = valDiff/5;
  //Serial.println(valDiff);
  //potValue = (uint8_t) (map((int) valFilt, 0, 1023, 0, 127));
}*/






