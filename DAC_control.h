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

#ifndef DAC_CONTROL_H
#define DAC_CONTROL_H



//Definitions
#define delicious_pi  3.1415926535  //Used for sine wave / trig calculations

#define sleepTime     10   //Time to sleep between data gathering. Should be zero in most cases

#define updateAReg    9   //ADC header code -- Update register A (Horizontal mirror tilt)
#define updateBReg    10  //ADC header code -- Update register B (Vertical   mirror tilt)
#define doSleep       15  //ADC header code -- Enter sleep mode
#define doWakey       1   //ADC header code -- Exit sleep mode

#define p_DAT         13   //Default pin for ADC data transmission
#define p_CLK         12   //Default pin for ADC clock signal
#define p_CS          11   //Default pin for ADC chip select
#define p_ZRO         A0  //Default pin for OSLRF-01 zero signal
#define p_RET         A1  //Default pin for OSLRF-01 return signal
#define p_SYN         2   //Defualt pin for OSLRF-01 sync signal

#define p_RESET       3   //Set this pin high to reset the Arduino.

#define ANALOG_THRESH 40  //Threshold of detection for OSLRF_ZER and OSLRF_RET signals



//Data Structures
enum mode{            //Type of data to send to the ADC
  E_CONSTANTVOLTAGE,  //  Send a constant voltage
  E_SINEWAVE,         //  Send values that form a sine wave
  E_TRIANGLEWAVE,     //  Send values that form a triangle wave
  E_SLEEPING          //  The ADC is asleep, so don't send any values
};

enum pinTestResult{   //Whether the requested pin can be used for I/O
  E_SUCCESS,          //  The pin can be used
  E_FAIL_OUTOFRANGE,  //  There is no pin with the number given
  E_FAIL_SERIAL       //  That pin is reserved for serial I/O, and cannot be used
};

enum pMode{
  E_INPUT = 0,
  E_OUTPUT = 1
};

typedef unsigned char pin;

struct outPins{
  pin CS;         //Chip Select for ADC
  pin DAT;        //Data input for ADC
  pin CLK;        //Clock input for ADC
  pin OSLRF_ZER;  //Zero output for OSLRF-01
  pin OSLRF_RET;  //Return signal output for OSLRF-01
  pin OSLRF_SYN;  //Sync signal output for OSLRF-01
};



//Function Prototypes
void parseCommand();
void processADCData(mode);
double getDistanceReading(outPins pins);

void updateDAC(unsigned int, unsigned char, int, int, int);
void writeDataToDAC(unsigned char, int, int);

void doAck(bool, String, int);
void doAck(bool, int);
void doAck(bool, String);
void doAck(bool);

void doVoltageSet(int, int, outPins);
void setPin(int, outPins*);

void serialClear();

pin getPin(unsigned char, pMode);

pMode getPMode(pin);

void initPins(outPins);

#endif
