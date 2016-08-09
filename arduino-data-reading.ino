/**
* LiDAR microcontroller code
* By Alex Shuping
* 
* OSLRF-01 reading code from berryjam.eu/2014/06/oslrf-01
*
* Note: Because this code averages readings, it really isn't suitable for our rapid-scanning applications
*       (This might be why our readings looked terrible)
*
* LED codes (These come in the form of LongFlash -> (ShortFlash)*n -> LongFlash, where n is the message code
*   1 -- INFO  -- Setup completed successfully
*   2 -- INFO  -- Dry-run readings are being taken (waiting for readings to stabilize)
*   3 -- ERROR -- A control byte was received, but was not recognized
*   4 -- ERROR -- Control responded with invalid character while acknowledging connection
*   5 -- ERROR -- Control computer did not respond to synack within set timeout
*
* Last update: 5 August 2016
*/

#define MANUAL_READINGS    // Uncomment this line to take manual (turntable) readings, comment out for MEMS mirror readings

//Pin definitions
#define ZERO_PIN     A1      // Arduino pin tied to Zero   pin on the OSLRF
#define RETURN_PIN   A3      // Arduino pin tied to Return pin on the OSLRF
#define SYNC_PIN     2       // Arduino pin tied to Sync   pin on the OSLRF
#define BUTN_PIN     3       // Arduino pin that can be connected to ground via a button to take manual readings if MANUAL_READINGS is defined (above)
#define INDC_PIN     13      // Arduino pin connected to an LED -- for state indication

//Constant definitions
#define pi 3.1415926535      // Approximation of pi -- for trig functions

//Control character definitions
const char CHAR_SYN    = 'a';   // Connection test from control computer
const char CHAR_SYNACK = 'b';   // Response to connection test
const char CHAR_ACK    = 'c';   // Conformation from control computer
const char CHAR_NAK    = 'd';   // Invalid response from control computer
const char CHAR_DAT    = 'e';   // Data request from control comptuer
const char CHAR_DRYRUN = 'f';   // Request to take dry-run measurements from control computer

//Timeout definitions
#define SYN_TIMEOUT 1000     // Number of milliseconds to wait for a connection ack from control computer

//Level / threshold definitions
#define MAX_MIRROR_VOLTAGE 6 // Max voltage to be applied to a micromirror
#define VOLTAGE_STEP     0.1 // Voltage step / resolution for the micromirror (Each cycle, voltage changes by this much)
#define VOLTAGE_STEP_MAN   2 // Voltage step / resolution to be used when manual control is enabled

//Library definitions
#define numReadings     1    // Number of distance readings to store for averaging (Library code)
#define numReadingsSync 20   // Number of sync     readings to store for averaging (Library code)

/**
* Code from the OSLRF-01 Library
* 
* Taken (with very slight alterations) from berryjam.eu/2014/06/oslrf-01/
* 
* This code takes the average of a set number of readings of distances and sync times
* (set with numReadings and numReadingsSync respectively), averages them, and stores 
* the averages in avgDist and avgSync respectively.
*/
  int zero_val = 0;         // Value of zero   signal for current reading
  int return_val = 0;       // Value of return signal for current reading
  int sync_val_1 = 0;       // Values of sync  signal for current reading
  int sync_val_2 = 0;       // See above
  int amp = 0;              // 
  
  int zero_thresh = 120;    // Adjust these thresholds if readings are not being received or are clearly wrong
  int return_thresh = 120;  // See above
  
  float raw_distance = 0.0; // Distance calcuated directly from equation in datasheet
  float distance = 0.0;     // Distance with slight adjustments
  
  unsigned long zero_time;  // Pulse time for zero signal
  unsigned long zero_time1; //
  unsigned long zero_time2; // This variable was declared, but never used...
  
  unsigned long echo_time;  // Pulse time for return signal
  unsigned long echo_time1; // Time of echo signal rising-edge
  unsigned long echo_time2; // Time of echo signal falling-edge
  
  unsigned long sync_time_1;// Rise time for sync pin
  unsigned long sync_time_2;// Fall time for sync pin
  unsigned long sync_period;// Sync pin period
  
  // Distance average variables
  double readings[numReadings];      // the readings from the analog input
  int index = 0;                     // the index of the current reading
  double total = 0;                  // the running total
  double avgDist = 0;                // the average

  // Sync period average variables
  unsigned long readings2[numReadingsSync];   // the readings from the analog input
  int index2 = 0;                             // the index of the current reading
  unsigned long total2 = 0;                   // the running total
  unsigned long avgSync = 0;                  // the average
//End OSLRF-01 Library Code

double vVert = 0;                              // Voltage that is currently being applied to the  vertical  axis of the micromirror
double vHorz = 0;                              // Voltage that is currently being applied to the horizontal axis of the micromirror

void setup() {
  Serial.begin(9600);                          // Begin communication with processing computer
  pinMode(INDC_PIN, OUTPUT);                   // Initialize feedback pin (see LED codes at top of file for more info)
  pinMode(SYNC_PIN, INPUT);                    // Initialize sync pin to take readings
  digitalWrite(13, LOW);
  
	#ifdef MANUAL_READINGS
	pinMode(BUTN_PIN, INPUT_PULLUP);             // Initialize manual input pin for taking manual readings, but only if MANUAL_READINGS is defined
	#endif

  //Libarary code
  // Sync averaging itialize routine
 for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0; 

 // Distance averaging itialize routine
 for (int thisReading2 = 0; thisReading2 < numReadingsSync; thisReading2++)
    readings2[thisReading2] = 0;      

  flash(1, INDC_PIN);                                 // To indicate setup completed successfully
}

void loop() {
  if(Serial.available()){                             // Wait for serial connection
    char c = Serial.read();                           // Read control character from serial
    if(c==CHAR_SYN){                                  // This character indicates that the control computer is establishing / verifying connection
		while(!doHandshake()){                        // Complete the handshake (see function description for more info)
			flash(5, INDC_PIN);                             // If the handshake times out, flash an errorcode and retry
			serFlush();
		}
	}else if(c==CHAR_DAT){                            // This character indicates a data request from the control computer
      #ifdef MANUAL_READINGS 
			while(digitalRead(BUTN_PIN)==HIGH){};     // If manual control is set, wait for a button press to respond with data
      #endif
	  Serial.println(vVert);                          // Send  vertical-axis  micromirror voltage
      Serial.println(vHorz);                          // Send horizontal-axis micromirror voltage
      #ifdef MANUAL_READINGS
	  vVert += VOLTAGE_STEP_MAN;                      // If manual control is set, increment vertical voltage by set amount (see #defines at top of file)
	  #else                                           // If manual control is unset, do the following:
      if(vVert == MAX_MIRROR_VOLTAGE){                //   If vertical displacement is at maximum value
  	    vVert = 0;                                    //                   Set it to zero 
	    vHorz = vHorz == MAX_MIRROR_VOLTAGE ? 0 : vHorz + VOLTAGE_STEP; // and increment the horizontal voltage (unless that is also maximum, in which case, set it to zero as well)
	  }else{
	    vVert += VOLTGAE_STEP;                         //   Otherwise, increment vertical mirror voltage by set amount (see #defines at top of file
	  }
	  #endif
	  Serial.println(avgDist);                         // Send distance measure to control computer
	  #ifdef MANUAL_READINGS
      delay(1000);                                     // Limit manual readings to one per second
      #endif
	  serFlush();
    }else if(c==CHAR_DRYRUN){                          // This character indicates a request for a "dry run" -- data are to be collected and processed from a single point (so the micromirror is not to be actuated)
      Serial.println(vVert);                           // Send  vertical-axis  micromirror voltage
      Serial.println(vHorz);                           // Send horizontal-axis micromirror voltage
      Serial.println(avgDist);                         // Send distance reading
      flash(2, INDC_PIN);                              // Flash an indication that this is a dry run.
      serFlush();
      vVert = 0;                                       // Ensure that the micromirror voltages are set at zero
	  vHorz = 0;                                       // See above
    }else{
      flash(3, INDC_PIN);                              // Flash an error message -- a character was received, but not recognized
      serFlush();
    }
  }else{
    //No control character received
  }

	doOSLRFLibraryLoop();                              // Update the library code (take readings and update averaging table)

}

/**
* void serFlush()
* 
* Flushes the serial input, discarding any messages present
* 
* PARAMS:
*   none
* 
* RETURN:
*   none
*/
void serFlush(){
  while(Serial.available()){Serial.read();}
}

/**
* void flash(unsigned char code, int outPin)
* 
* Uses the indicator LED to send a message (see top of file for LED codes)
* 
* PARAMS:
*   unsigned char code -- LED code to output
*   int outPin         -- Pin to output code on
* 
* RETURN:
*   none
*/
void flash(unsigned char code, int outPin){
  digitalWrite(outPin, HIGH);
	delay(500);
	digitalWrite(outPin, LOW);
	delay(500);
	for(unsigned char i = 0; i < code; i++){
		digitalWrite(outPin, HIGH);
		delay(250);
		digitalWrite(outPin, LOW);
		delay(250);
	}
  digitalWrite(outPin, HIGH);
	delay(500);
	digitalWrite(outPin, LOW);
	delay(500);
}

/**
* bool doHandshake()
* 
* Three-way handshake function (Think TCP)
* Sends a synack and waits for an ack.
* Handshake structure is as follows:
*  1. Control computer sends "syn" signal to microcontroller
*  2. Microcontroller sends "synack" signal to control computer
*  3. Control computer sends "ack" to microcontroller
* This ensures that both components are responding and can communicate data.
* 
* PARAMS:
*   none
* 
* RETURN:
*   bool -- Connection state (successful / timed out)
*/
bool doHandshake(){
  Serial.write(CHAR_SYNACK);
  serFlush();

	bool ack = false;
	for(int i = 0; i < SYN_TIMEOUT; i++){
			
		if(Serial.available()){
			if(Serial.read() == CHAR_ACK){
				ack = true; 
				break;
			}else{
				flash(4, INDC_PIN);
				Serial.println(CHAR_NAK);
			}
		}
				
		delay(1);
	}	
}	

/**
* void doOSLRFLibraryLoop()
* 
* Performs library updates
* Takes measurements from OSLRF-01, and uses them to update the averaging table
* Code taken (with very slight alterations) from berryjam.eu/2014/06/oslrf-01/
* 
* PARAMS:
*   none
* 
* RETURN:
*   none
*/
void doOSLRFLibraryLoop(){
  getSyncPeriod();      // This "Sync" period update can be done not on every loop cycle,
  AverageSyncPeriod();  // as it takes time and loses some (3) distance measurements

  // Detect Zero signal rising edge
  while ((zero_val = analogRead(ZERO_PIN)) < zero_thresh) 
  zero_time1 = micros();
  
  // Detect Return signal rising edge based on previous measurement amplitude
  while ((return_val = analogRead(RETURN_PIN)) < return_thresh) 
  echo_time1 = micros();


  // Get maximum height of Return pulse...
  amp = 0;
  while (amp <= return_val ) { 
    amp = return_val;
    return_val = analogRead(RETURN_PIN);
  }

  // Detect Return signal falling edge 
  while ((return_val = analogRead(RETURN_PIN)) > return_thresh) 
  echo_time2 = micros();

  // New Return signal threshold for next measurement, based on the new amplitude
  return_thresh = 0.13 * (float)amp + 10;                                // Pure empiric, based on observations.
  if (return_thresh < 18) return_thresh = 18;                            // Make sure that threshold is over the noise
  
  zero_time = zero_time1 + 3500;                                         // Midpoint of Zero. Full Zero signal width - 7000us, when threshold 40
  echo_time = echo_time1 + ((float)echo_time2 - (float)echo_time1)/3.0;

  raw_distance = (float)(echo_time - zero_time)/(float)avgSync * 1833.0;
  
  if (raw_distance > 1000) {}                                            // Just ignore this reading
  else{
    if (raw_distance < 220){                                             // RAW measure corrections if distance less than 100 cm
    distance = 0.725 * raw_distance - 56.208;
    }
    else distance = 1.078 * raw_distance - 134.05;                       // Empiric correction for 100cm and up
  }
  
  AverageDistanceReadings();
}

/**
* All code below this line taken (with very slight alterations) from berryjam.eu/2014/06/oslrf-01/
*/

void getSyncPeriod(){
  //  sync_period = 2*pulseIn(SYNC_PIN, LOW); // was too big about 80us, because duty cycle not perfect 50%

  // Need to optimize, as it takes full two clocks
  unsigned long sync_period1 = pulseIn(SYNC_PIN, LOW);
  unsigned long sync_period2 = pulseIn(SYNC_PIN, HIGH);
  sync_period = sync_period1 + sync_period2;

}  

void AverageSyncPeriod(){
  total2 = total2 - readings2[index2]; // subtract the last reading
  readings2[index2] = sync_period;     // Get last measure
  total2 = total2 + readings2[index2]; // add the reading to the total
  index2 = index2 + 1;                 // advance to the next position in the array
  if (index2 >= numReadingsSync)       // if we're at the end of the array...
  index2 = 0;                          // ...wrap around to the beginning
  avgSync = total2 / numReadingsSync;     // calculate the average
}

void AverageDistanceReadings(){
  total = total - readings[index];   // subtract the last reading
  readings[index] = distance;        // Get last measure
  total = total + readings[index];   // add the reading to the total
  index = index + 1;                 // advance to the next position in the array
  if (index >= numReadings)          // if we're at the end of the array...
  index = 0;                         // ...wrap around to the beginning
  avgDist = total / numReadings;     // calculate the average
}
