/*
LTC1661 DAC Control System
Version 1.2
History:
 1.1:
  + Added R (reset) command
  
 1.2:
  + p (change pinout) command
  + Code comments
  ~ Code cleanup

Written by Alex Shuping
7 July 2016
Last update: 11 July 2016
*/
#include "DAC_control.h"  //See header file for info on constants, structs, prototypes, and enums.

unsigned int curr_Vout = 0;
unsigned int sav_Vout  = 0;
mode curr_mode = E_CONSTANTVOLTAGE;
float wave_ctr = 0;
int wave_res = 10;
outPins pins_CURR;

void setup() {
  Serial.begin(57600);
  Serial.println("Listening for input. Type h for help.");
  pins_CURR = {getPin(p_CS, E_OUTPUT), getPin(p_DAT, E_OUTPUT), getPin(p_CLK, E_OUTPUT), getPin(p_ZRO, E_INPUT), getPin(p_RET, E_INPUT), getPin(p_SYN, E_INPUT)};
  initPins(pins_CURR);
}

void loop() {
  if(Serial.available()){
    parseCommand();
  }
  processADCData(curr_mode);
  //Currently nonfunctional
  int distance = getDistanceReading(pins_CURR);
  Serial.println(distance);
  
  delay(sleepTime);
}

/*
 * int getDistanceReading()
 * 
 * !!Currently nonfunctional!!
 * 
 * Retrieves a time-of-flight reading from the OSLRF-01, and converts it into a distance reading
 * 
 * Params:
 *   outPins pins -- Pins to read from
 *   
 * Return: 
 *   double       -- Calculated distance reading
 */
double getDistanceReading(outPins pins){
  unsigned long syn_period = 0;
  int base = millis();
  doAck(true, "Testing SYN\n");
  {
  while(digitalRead(getPId(pins.OSLRF_SYN)) == LOW){/*if(millis() - base > 1000){doAck(false, "Timed out...", 0); break;}*/}
  unsigned long syn_period_zero = pulseIn(pins.OSLRF_SYN, LOW);
  unsigned long syn_period_end = pulseIn(pins.OSLRF_SYN, HIGH);
  syn_period = syn_period_end - syn_period_zero;
  }
  {
  unsigned long zer_pos;
  doAck(true, "GOT SYN\n");
  
  {
  bool broke = false;
  base = millis();
  while(analogRead(getPId(pins.OSLRF_ZER)) < ANALOG_THRESH){/*if(millis() - base > 1000){doAck(false, "Timed out...", 2); break;}*/}
  unsigned long zer_start = micros();
  base = millis();
  
  while(analogRead(getPId(pins.OSLRF_ZER)) > ANALOG_THRESH){if(analogRead(getPId(pins.OSLRF_RET)) > ANALOG_THRESH){broke = true;break;} /*if(millis() - base > 1000){doAck(false, "Timed out...", 3); break;}*/}
  unsigned long zer_end = broke ? zer_start : micros();
  zer_pos = zer_end - zer_start;
  doAck(true, "GOT ZER\n");
  }
  bool broke = false;
  base = millis();
  while(analogRead(getPId(pins.OSLRF_RET)) < ANALOG_THRESH){/*if(millis() - base > 1000){doAck(false, "Timed out...", 4); break;}*/}
  unsigned long ret_start = micros();
  base = millis();
  while(analogRead(getPId(pins.OSLRF_RET)) > ANALOG_THRESH){if(analogRead(getPId(pins.OSLRF_ZER)) > ANALOG_THRESH){broke = true;break;} /*if(millis() - base > 1000){doAck(false, "Timed out...", 5); break;}*/}
  unsigned long ret_end = broke? ret_start : micros();
  unsigned long ret_pos = ret_end - ret_start;
  doAck(true, "GOT RET\n");
  return (((ret_pos - zer_pos)*1000) / (syn_period*1000)) * 18330;
  }

  
}

/*
 * void processADCData(mode m)
 * 
 * Updates the ADC with the test pattern for the given mode
 * Params: which test pattern mode to use
 * Return: none
 */
void processADCData(mode m){
  switch(m){
    case E_CONSTANTVOLTAGE:
      updateDAC(curr_Vout, updateAReg, pins_CURR);
      break;
    case E_SINEWAVE:
      curr_Vout = abs(sin(wave_ctr) * (double)sav_Vout);
      wave_ctr += 1.0 / (double)wave_res;
      if(wave_ctr > delicious_pi){wave_ctr = 0;}
      updateDAC(curr_Vout, updateAReg, pins_CURR);
      break;
    case E_TRIANGLEWAVE:
      curr_Vout = wave_ctr > (delicious_pi / 2) ? (delicious_pi - wave_ctr) * ((double)sav_Vout * 2 / delicious_pi) : wave_ctr * ((double)sav_Vout * 2 / delicious_pi);
      wave_ctr += 1.0 / (double)wave_res;
      if(wave_ctr > delicious_pi){wave_ctr = 0;}
      updateDAC(curr_Vout, updateAReg, pins_CURR);
      break;
    case E_SLEEPING:
      delay(100);
      break;
    default:
      doAck(false, "Mode is in invalid state, switching to mode 0. State was: ", curr_mode);
      curr_mode = E_CONSTANTVOLTAGE;
      break;
  }
}

/*
 * void parseCommand()
 * Reads a command from the serial port.
 * Params: none
 * Return: none
 */
void parseCommand(){
  char re = Serial.read();                               //Read command
    
    if(re=='s'){                                         //Sleep command
      curr_mode = E_SLEEPING;
      updateDAC(curr_Vout, doSleep, pins_CURR);
      doAck(true, "Sleeping. \n");


      
    }else if(re=='w'){                                   //Wake command
      curr_mode = E_CONSTANTVOLTAGE;
      updateDAC(curr_Vout, doWakey, pins_CURR);
      doAck(true, "Waking. \n");


      
    }else if(re=='v'){                                   //Set voltage command
      int i = Serial.parseInt();
      if(i>=0 && i<1024){
        doAck(true, "Setting voltage to: ", i);
        curr_Vout = i;
        sav_Vout = i;
      }else{
        doAck(false, "Voltage not in range 0-1023: ", i);
      }


      
    }else if(re=='m'){                                   //Mode change command
      char mod;
      doAck(true, "Mode change activated. Enter new mode.\n");
      do{
        mod = Serial.read();                             //Read new mode command
      }while(mod < 32);
      switch(mod){
        case 'c':                                        //Switch to Constant-Voltage mode
          curr_mode = E_CONSTANTVOLTAGE;
          doAck(true, "Entered constant-voltage mode.\n");
          curr_Vout = sav_Vout;
          break;
        case 's':                                        //Switch to Sine-Wave mode
          if(curr_mode == E_CONSTANTVOLTAGE){sav_Vout = curr_Vout;}
          curr_mode = E_SINEWAVE;
          doAck(true, "Entered sine-wave mode.\n");
          break;
        case 't':                                        //Switch to Triangle-Wave mode
          if(curr_mode = E_CONSTANTVOLTAGE){sav_Vout = curr_Vout;}
          curr_mode = E_TRIANGLEWAVE;
          doAck(true, "Entered triangle-wave mode.\n");
          break;
        default:
          doAck(false, "Invalid mode.\n");
      }


      
    }else if(re=='h'){                                   //Help commnand
    doAck(true, "LTC1661 DAC Control System\n");
    Serial.println("Written by Alex Shuping");
    Serial.print("Current Pin Output: CS = ");
    Serial.print(getPId(pins_CURR.CS));
    Serial.print(" | CLK = ");
    Serial.print(getPId(pins_CURR.CLK));
    Serial.print(" | DAT = ");
    Serial.println(getPId(pins_CURR.DAT));
    Serial.println("Command list: \nh -- this help text \nm -- set mode: \n  c -- constant-voltage mode\n  s -- sine-wave mode\n  t -- triangle-wave mode\nv -- voltage for constant-voltage mode\n  syntax: vn, where n is a value from 0-1023.\n          new voltage will be (n/1024) * Vref of the chip.\ns -- enter sleep mode\nw -- wake from sleep mode\nr -- resolution of sin/triangle wave\n  syntax: rn, where n is a number greater than zero.\n          Each clock cycle, the counter for the wave will increase by 1/n.\nR -- Resets the Arduino.\np -- Change pinout");

  
  
  }else if(re=='r'){                                      //Wave resolution command
    int r = Serial.parseInt();
    if(r<=0){
      doAck(false, "Cannot parse zero or sub-zero value: ", r);
    }else{
      wave_res = r;
      doAck(true, "Wave resolution set to: ", r);
    }


    
  }else if(re=='R'){                                      //Arduino reset command
    doAck(true, "RESETTING...\n");
    delay(100);
    digitalWrite(p_RESET, HIGH);



  }else if(re=='p'){                                      //Pinset command
    doAck(true, "Pinset mode activated. Input new pin for CS: ");
    setPin(0, &pins_CURR);
    doAck(true, "Success. Input new pin for CLK: ");
    setPin(1, &pins_CURR);
    doAck(true, "Success. Input new pin for DAT: ");
    setPin(2, &pins_CURR);
    doAck(true, "Success. Input new pin for ZRO: ");
    setPin(3, &pins_CURR);
    doAck(true, "Success. Input new pin for RET: ");
    setPin(4, &pins_CURR);
    doAck(true, "Success. Input new pin for SYN: ");
    setPin(5, &pins_CURR);
    initPins(pins_CURR);
    doAck(true, "Success.\n");
    
    
    
  } else{
    doAck(false, "Command not recognized\n");
  }
    serialClear();
}

void updateDAC(unsigned int newVOut, unsigned char daFlags, outPins pins){
  unsigned int output = newVOut << 2;
  output |= (daFlags << 12);
  digitalWrite(getPId(pins.CS), HIGH);
  //DEBUG_VERBOSE:Serial.print("Data: ");
  //DEBUG_VERBOSE:Serial.println(output);
  writeDataToDAC(output, pins);
  digitalWrite(getPId(pins.CS), LOW);
}

void writeDataToDAC(unsigned int data, outPins pins){
  for(int i = 15; i >= 0; i--){
    if(((data & (1 << i)) >> i) == 1){
      //DEBUG_ULTRAVERBOSE:Serial.println("Writing 1");
      digitalWrite(getPId(pins.DAT), HIGH);
    }else{
      //DEBUG_ULTRAVERBOSE:Serial.println("Writing 0");
    }
    digitalWrite(getPId(pins.CLK), HIGH);
    delay(sleepTime);
    digitalWrite(getPId(pins.CLK), LOW);
    digitalWrite(getPId(pins.DAT), LOW);
    delay(sleepTime);
  }
}

/*
 * void doAck(bool suc, String msg)
 * Standard function for printing command status messages to the screen.
 * 
 * Overloads:
 *   doAck(bool suc, String msg)
 *   doAck(bool suc, String msg, int code)
 *   doAck(bool suc, int code)
 *   doAck(bool suc)
 *   
 * Params:
 *   bool suc   -          - whether the command was successful
 *   String msg -[OPTIONAL]- additional message about the command
 *   int code   -[OPTIONAL]- a number that can be used for additional command data
 *   
 * Return: none
 */
void doAck(bool suc, String msg){
  if(suc){
    Serial.print("Command OK. ");
  }else{
    Serial.print("Command NOT OK. ");
  }
  Serial.print(msg);
}
void doAck(bool suc, String msg, int code){
  doAck(suc, msg);
  Serial.println(code);
}
void doAck(bool suc, int code){
  String msg = "Status: ";
  doAck(suc, msg);
  Serial.println(code);
}
void doAck(bool suc){
  String msg = "Status: ";
  int code = suc ? 0 : 1;
  doAck(suc, msg);
  Serial.println(code);
}

/*
 * void doVoltageSet(int v_chA, int v_chB)
 * Sets both ADC channels to the given voltages
 * 
 * Params:
 *   int v_chA -- new voltage for channel A
 *   int v_chB -- new voltage for channel B
 *   
 * Return: none
 */
void doVoltageSet(int v_chA, int v_chB, outPins pins){
    updateDAC((unsigned int) v_chA, updateAReg, pins);
    updateDAC((unsigned int) v_chB, updateBReg, pins);
}

/*
 * void setPin(int pinIndex, outPins* pins)
 * Waits for input of a new pin and pin mode through the serial port, and then writes the pin to the outPins at the pointer given
 * 
 * Params:
 *   int pinIndex -- which pin to set -- first index is zero, see struct outPins in DAC_control.h for the order of pin setting
 */
void setPin(int pinIndex, outPins* pins){
  serialClear();
  pin newPin = 0;
  int result;
  do{
    while(!Serial.available()){}
    newPin = (unsigned char)Serial.parseInt();
    
    if(newPin >= 2 && newPin <=13){
     result = E_SUCCESS;
    }else if(newPin == 1 || newPin == 0){
      result = E_FAIL_SERIAL;
    }else{
      result = E_FAIL_OUTOFRANGE;
    }
    
    if(result == E_FAIL_SERIAL){
      doAck(false, "\nPins 0 and 1 are reserved for serial communication. Input new pin: ");
    }else if(result == E_FAIL_OUTOFRANGE){
      doAck(false, "\nOut of range. Input new pin: ");
    }
    
    serialClear();
  }while(result != E_SUCCESS);
  Serial.print('\n');
  doAck(true, "Enter Pin Mode ([o]utput / [i]nput): ");
  {
    bool valid = false;
    char c = 0;
    do{
      c = Serial.read();
      if(c=='i' || c=='o'){
        valid = true;
        if(c=='i'){
          newPin = getPin(newPin, E_INPUT);
        }else{
          newPin = getPin(newPin, E_OUTPUT);
        }
      }
    }while(!valid);
  }
  switch(pinIndex){
    case 0:
      pins->CS = newPin;
      break;
    case 1:
      pins->CLK = newPin;
      break;
    case 2:
      pins->DAT= newPin;
      break;
    case 3:
      pins->OSLRF_ZER= newPin;
      break;
    case 4:
      pins->OSLRF_RET= newPin;
      break;
    case 5:
      pins->OSLRF_SYN= newPin;
      break;
  }
}

/*
 * void serialClear()
 * Clears any buffered data from the serial interface
 * Params: none
 * Return: none
 */
void serialClear(){
  while(Serial.available()){
    Serial.read();
  }
}

/*
 * pin getPin(unsigned char id, pMode mode)
 * Compresses a pin number and a mode into a single char, to save memory
 * 
 * Params:
 *   unsigned char id -- Pin number
 *   pMode mode       -- Pin is input or output
 *   
 * Returns:
 *   pin              -- compressed pin data
 */
pin getPin(unsigned char id, pMode mode){
  pin p = id;    //Set first 7 bits to id number
  p |= mode << 7;//Place mode as last
}

/*
 * pMode getPMode(pin p)
 * Gets mode bit from pin
 * 
 * Params:
 *   pin p -- Pin to read from
 * 
 * Return:
 *   pMode -- Mode of input pin
 */
pMode getPMode(pin p){
  return (p & (unsigned char)128 >> 7) ? E_OUTPUT : E_INPUT;
}

/*
 * unsigned char getPId(pin p)
 * Gets the pin number of an input pin
 * 
 * Params:
 *   pin p         -- Pin to read from
 * 
 * Return:
 *   unsigned char -- Pin number of input pin
 */
unsigned char getPId(pin p){
  return p & (unsigned char)127;
}

void initPins(outPins pins){
  pinMode(getPId(pins.CS), getPMode(pins.CS));
  pinMode(getPId(pins.DAT), getPMode(pins.DAT));
  pinMode(getPId(pins.CLK), getPMode(pins.CLK));
  pinMode(getPId(pins.OSLRF_ZER), getPMode(pins.OSLRF_ZER));
  pinMode(getPId(pins.OSLRF_RET), getPMode(pins.OSLRF_RET));
  pinMode(getPId(pins.OSLRF_SYN), getPMode(pins.OSLRF_SYN));
}
