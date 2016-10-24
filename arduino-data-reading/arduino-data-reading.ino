/**
* LiDAR microcontroller code
* By Alex Shuping
* 
* OSLRF-01 reading code from berryjam.eu/2014/06/oslrf-01
*
* Note: Because this code averages readings, it really isn't suitable for our rapid-scanning applications
*       (This might be why our readings looked terrible)
*
* State indication:
*   The module indicates state with a shift register connected to pins 10 (clock) and 11 (data). See accompanying diagram for details.
*   State identification (* = LED):
*                 3  
*                 *
*   0  1     2    R    5  6  7
*   *  *     *         *  *  *  
*   G  G     A    4    A  A  A
*                 *
*                 G
*   
*   (G = Green LED, R = Red LED, A = Amber LED)
*   
*   0 ---- Device power-on
*   1 ---- Serial connection up
*   2 ---- Received serial data
*   3 ---- Received invalid packet
*   4 ---- Received valid packet
*   
*   5 -|
*   6 -+-- Error bits
*   7 -|
*
* Last update: 1 October 2016
*/



//Pin definitions
#define   ZERO_PIN   A1      // Arduino pin tied to Zero   pin on the OSLRF
#define RETURN_PIN   A3      // Arduino pin tied to Return pin on the OSLRF
#define   SYNC_PIN    2      // Arduino pin tied to Sync   pin on the OSLRF
#define   BUTN_PIN    3      // Arduino pin that can be connected to ground via a button to take manual readings if MANUAL_READINGS is defined (above)
#define   INDC_PIN   13      // Arduino pin connected to an LED -- for state indication
#define  STATC_PIN   10      // Pin for state register clock
#define  STATD_PIN   11      // Pin for state register data

//Constant definitions
#define pi        3.1415926535 // Approximation of pi -- for trig functions
#define PAKT_SIZE 8            // Maximum size of data section of packet  

//Control character definitions
const byte LIDAR_SYN    = 0x00; // SYNc for handshake              -- Sent     to   microcontroller to establish a connection
const byte LIDAR_SYNACK = 0x01; // SYNc ACKnowledge for handshake  -- Received from microcontroller to indicate successful connection
const byte LIDAR_ACK    = 0x02; // ACKnowledge for handshake       -- Sent     to   microcontroller to complete a connection
const byte LIDAR_SET    = 0x03; // SET microcontroller setting     -- Sent     to   microcontroller to send settings
const byte LIDAR_GET    = 0x04; // GET data from microcontroller   -- Sent     to   microcontroller when data are requested
const byte LIDAR_RSP    = 0x05; // ReSPonse to data request        -- Received from microcontroller, indicates that data are being sent
const byte LIDAR_DISCON = 0x06; // DISCONnect with microcontroller -- Sent     to   microcontroller to indicate that the connection is being terminated
const byte LIDAR_DSCACK = 0x07; // DiSConnect ACKnowledge          -- Received from microcontroller to confirm that disconnection was successful
const byte LIDAR_PROBE  = 0x08; // connection PROBE                -- Sent     to   unknown serial port to determine whether a compatible microcontroller is attached
const byte LIDAR_PRBACK = 0x09; // connection PRoBe ACKnowledge    -- Received from microcontroller to confirm that it is connected over the tested serial port

//State flags -- to enable,  use state_bits |=  STATE_FLAGS
//               to disable, use state_bits &= ~STATE_FLAGS
const byte STATE_POWER       = 0x01; // The device is running -- should be enabled unless something has gone very wrong and the program cannot continue
const byte STATE_CONNECTION  = 0x02; // The device is connected to a control computer
const byte STATE_RXDATA      = 0x04; // Serial data are being received
const byte STATE_GOODPAKT    = 0x08; // A valid packet was received
const byte STATE_BADPAKT     = 0x10; // An invalid packet was received
const byte STATE_ERR1        = 0x20; // Error bit -- see error constants below
const byte STATE_ERR2        = 0x40; // ^
const byte STATE_ERR3        = 0x80; // ^
const byte STATE_ERRALL      = 0xE0; // Use this to clear the error bits -- state_bits &= ~STATE_ERRALL 

const byte STATE_ERR_PACKETINCOMPLETE    =                           STATE_ERR1; // Received data was less than PAKT_SIZE + 1 bytes (PAKT_SIZE data bytes and 1 header byte)
const byte STATE_ERR_PAKTETINVALIDHEADER =              STATE_ERR2             ; // Packet header was out of range or reserved for microcontroller --> computer packet
const byte STATE_ERR_CONNECTFIRST        =              STATE_ERR2 | STATE_ERR1; // Recieved packet other than LIDAR_SYN or LIDAR_PROBE, but the connection is not up
const byte STATE_ERR_TIMEOUT             = STATE_ERR3                          ; // Packet request timed out -- Retry from control computer
const byte STATE_ERR_BADDISCONNECT       = STATE_ERR3 |              STATE_ERR1; // Disconnect was attempted, but timed out before LIDAR_DSCACK was received. The control computer may think that the connection is still up.
const byte STATE_ERR_BADDATA             = STATE_ERR3 | STATE_ERR2             ; // Packet is valid structurally, but meaning cannot be understood.
const byte STATE_ERR_UNUSED1             = STATE_ERR3 | STATE_ERR2 | STATE_ERR1; // Unused


byte state_bits;

//Config definitions
#define TIMEOUT 100            // Number of milliseconds to wait for a byte from the control computer
#define NUM_SETTINGS 

//Level / threshold definitions
#define MAX_MIRROR_VOLTAGE 6   // Max voltage to be applied to a micromirror
#define VOLTAGE_STEP       0.1 // Voltage step / resolution for the micromirror (Each cycle, voltage changes by this much)
#define VOLTAGE_STEP_MAN   2   // Voltage step / resolution to be used when manual control is enabled

//Library definitions
/*[LIB]*/#define numReadings      1     // Number of distance readings to store for averaging (Library code)
/*     */#define numReadingsSync 20     // Number of sync     readings to store for averaging (Library code)


class packet{

    private:
    byte _header;
    byte _data[PAKT_SIZE];

    public:
    packet(byte c, byte d[PAKT_SIZE]){
      for(int i = 0; i < PAKT_SIZE; i++){
        _data[i] = d[i];
      }
      _header = c;
    }

    packet(byte c){
      for(int i = 0; i < PAKT_SIZE; i++){
        _data[i] = 0;
      }
      _header = c;
    }

    void sendThis(){
      Serial.write(_header);
      for(int i = 0; i < PAKT_SIZE; i++){
        Serial.write(_data[i]);
      }
    }

    byte getHeader(){
      return _header;
    }
    
    byte getDataAt(int index){
      return _data[index];
    }

		int getUnsignedLong(){
      unsigned long out = 0;
	  	for(int byteindex = 0; byteindex < PAKT_SIZE; byteindex++){
		    for(int bitindex = 0; bitindex < 8; bitindex++){
		      if( (this -> _dat[byteindex] & (unsigned char)pow(2, bitindex)) != 0 ){
		        out += pow(2, byteindex * 8 + bitindex);
		      }
		    }
		  }
			
			return out;
		}
    
};

enum scanType{
  SCAN_ZIGZAG,
  SCAN_HORZ_ONLY_ZIGZAG,
  SCAN_VERT_ONLY_ZIGZAG
};

struct configOptions{

  bool     defined = false ; // Set to true when config is received from control computer
  
  scanType type            ;
  double   scanResolution  ;
	double   supplyVoltage   ;
  double   maxVoltage      ;
  bool     manualReadings  ;
  bool     continuousScans ;
  
};

configOptions globalConfig;



/**
* Code from the OSLRF-01 Library
* 
* Taken (with very slight alterations) from berryjam.eu/2014/06/oslrf-01/
* 
* This code takes the average of a set number of readings of distances and sync times
* (set with numReadings and numReadingsSync respectively), averages them, and stores 
* the averages in avgDist and avgSync respectively.
*/
/*[LIB]*/int zero_val = 0;         // Value of zero   signal for current reading
/*     */int return_val = 0;       // Value of return signal for current reading
/*     */int sync_val_1 = 0;       // Values of sync  signal for current reading
/*     */int sync_val_2 = 0;       // See above
/*     */int amp = 0;              // 
/*[LIB]*/ 
/*     */int zero_thresh = 120;    // Adjust these thresholds if readings are not being received or are clearly wrong
/*     */int return_thresh = 120;  // See above
/*     */ 
/*     */float raw_distance = 0.0; // Distance calcuated directly from equation in datasheet
/*[LIB]*/float distance = 0.0;     // Distance with slight adjustments
/*     */  
/*     */unsigned long zero_time;  // Pulse time for zero signal
/*     */unsigned long zero_time1; //
/*     */unsigned long zero_time2; // This variable was declared, but never used...
/*[LIB]*/
/*     */unsigned long echo_time;  // Pulse time for return signal
/*     */unsigned long echo_time1; // Time of echo signal rising-edge
/*     */unsigned long echo_time2; // Time of echo signal falling-edge
/*     */  
/*[LIB]*/unsigned long sync_time_1;// Rise time for sync pin
/*     */unsigned long sync_time_2;// Fall time for sync pin
/*     */unsigned long sync_period;// Sync pin period
/*     */  
/*     */// Distance average variables
/*[LIB]*/double readings[numReadings];      // the readings from the analog input
/*     */int index = 0;                     // the index of the current reading
/*     */double total = 0;                  // the running total
/*     */double avgDist = 0;                // the average
/*     */
/*[LIB]*/// Sync period average variables
/*     */unsigned long readings2[numReadingsSync];   // the readings from the analog input
/*     */int index2 = 0;                             // the index of the current reading
/*     */unsigned long total2 = 0;                   // the running total
/*     */unsigned long avgSync = 0;                  // the average
//End OSLRF-01 Library Code

double vVert = 0;                              // Voltage that is currently being applied to the  vertical  axis of the micromirror
double vHorz = 0;                              // Voltage that is currently being applied to the horizontal axis of the micromirror

void setup() {
  state_bits = 0;
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

  state_bits |= STATE_POWER;
  putState();
  
}

void loop() {
  if(Serial.available()){                           // Wait for serial connection
    
    bool fail = false;
    packet p = getPacket(&fail);
    if(fail){
      state_bits &= ~STATE_ERRALL;
      state_bits |= (STATE_ERR_PACKETINCOMPLETE | STATE_BADPAKT);
      putState();
    }else{
      
  if(p.getHeader() == LIDAR_SYN){                   // This character indicates that the control computer is establishing / verifying connection
    
        if(doHandshake()){ 
          state_bits |= STATE_CONNECTION;
          putState();

					if(getSettings(global_config)){ //TODO: implement
						
					}else{                                    // Bad config
					  state_bits &= ~STATE_ERRALL
						state_bits |= STATE_ERR_BADDATA;
						putState();
					}

        }else{                                      // Bad handshake
				  state_bits &= ~STATE_ERRALL
          state_bits |= STATE_ERR_HANDSHAKETIMEOUT;
					putState();
				}

  
  }else if(p.getHeader()==CHAR_DAT){                // This character indicates a data request from the control computer
      #ifdef MANUAL_READINGS 
      while(digitalRead(BUTN_PIN)==HIGH){};         // If manual control is set, wait for a button press to respond with data
      #endif
      Serial.println(vHorz);                        // Send horizontal-axis micromirror voltage
      Serial.println(vVert);                        // Send  vertical-axis  micromirror voltage
      #ifdef MANUAL_READINGS
    vHorz += VOLTAGE_STEP_MAN;                      // If manual control is set, increment horizontal voltage by set amount (see #defines at top of file)
    #else                                           // If manual control is unset, do the following:
      if(vVert == MAX_MIRROR_VOLTAGE){              //   If vertical displacement is at maximum value
        vVert = 0;                                  //                   Set it to zero 
      vHorz = vHorz == MAX_MIRROR_VOLTAGE ? 0 : vHorz + VOLTAGE_STEP; // and increment the horizontal voltage (unless that is also maximum, in which case, set it to zero as well)
    }else{
      vVert += VOLTAGE_STEP;                         //   Otherwise, increment vertical mirror voltage by set amount (see #defines at top of file
    }
    #endif
    Serial.println(avgDist);                         // Send distance measure to control computer
    #ifdef MANUAL_READINGS
      delay(1000);                                     // Limit manual readings to one per second
      #endif
    serFlush();
    }else if(p.getHeader()==CHAR_DRYRUN){                          // This character indicates a request for a "dry run" -- data are to be collected and processed from a single point (so the micromirror is not to be actuated)
      Serial.println(vHorz);                        // Send horizontal-axis micromirror voltage
      Serial.println(vVert);                        // Send  vertical-axis  micromirror voltage
      Serial.println(avgDist);                         // Send distance reading
      flash(2, INDC_PIN);                              // Flash an indication that this is a dry run.
      serFlush();
      vVert = 0;                                       // Ensure that the micromirror voltages are set at zero
    vHorz = 0;                                       // See above
    }else{
      state_bits &= ~STATE_ERRALL;
      state_bits |= STATE_ERR_PAKTINVALIDHEADER;
      serFlush();
    }
  }else{
    //No control character received
  }
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
  
  packet(LIDAR_SYNACK).sendThis();
  bool timeout = waitForSerialTimeout();

  if(timeout){
    packet(LIDAR_DISCON).sendThis();
    return false;
  }

  packet rsp = getPakt(&timeout);
  if(timeout || (rsp.getHeader() != LIDAR_ACK)){
    discon();
    return false;
  }

  return true;
    
}	

/**
 * Gets settings from the control computer
 */
bool getSettings(){

  for(int i = 0; i < NUM_SETTINGS; i++){
	  bool fail = false;
		packet p = getPakt(&fail);

		if(fail){
	    state_bits &= ~STATE_ERRALL;
      state_bits |= STATE_ERR_TIMEOUT;
		  putState();

		  state_bits &= ~STATE_CONNECTION;
		  if(!discon()){
		    state_bits &= ~STATE_ERRALL;
			  state_bits |= STATE_ERR_BADDISCONNECT;
	  	}
	    putState();
	    return false;
      
		}

    switch(i){
		case 0:
			switch(p.getUnsignedLong){
  	    case 0:
				  globalConfig.type = SCAN_ZIGZAG;
					break;
			  case 1:
			    globalConfig.type = SCAN_HORZ_ONLY_ZIGZAG;
					break
				default:
				  globalConfig.type = SCAN_VERT_ONLY_ZIGZAG;
			}

		}
	}

  if(waitForSerialTimeout()){
  }
i

  
}

/**
 * Gets a packet from the serial connection. 
 * 
 * PARAMS:
 *   bool* err -- Function sets this to false if it reads a valid packet, true otherwise
 */
packet getPakt(bool* err){
  
  state_bits |= STATE_RXDATA;
  putState();
  
  header = Serial.read();
  byte arr[PAKT_SIZE];
  
  for(int i = 0; i < PAKT_SIZE; i++){
    
    if(waitForSerialTimeout()){
      *err = 1;
      state_bits &= ~STATE_RXDATA;
      state_bits |=  STATE_BADPAKT;
      putState();
      return packet(header, arr);
    }
    
    arr[i] = Serial.read();
  }

  *err = 0;
  state_bits &= ~STATE_RXDATA;
  state_bits |=  STATE_GOODPAKT;
  putState();
  return packet(header, arr);
  
}
/*[AUX]*/bool waitForSerialTimeout(){
/*     */  for(int i = 0; i < TIMEOUT; i++){
/*     */    if(Serial.available()){
/*     */      return false;
/*     */    }
/*[AUX]*/    delay(1);
/*     */  }
/*     */  return true;
/*     */}

/**
 * Writes state information to the indicator register
 */
void putState(byte putFlags, int cPn, int dPn){
  for(int i = 0; i < 8; i++){
    
    if((putFlags & (byte)ipow2(i)) << i){
      digitalWrite(dPn, HIGH);
    }

    digitalWrite(cPn, HIGH);
    delay(CLOK_SLP);
    digitalWrite(cPn,  LOW);
    digitalWrite(dPn,  LOW);
  }
}
/*[AUX]*/void putState(){
/*     */  putState(state_bits, STATC_PIN, STATD_PIN);
/*     */}
/*     */byte ipow2(int i){
/*     */  if(i <= 0){
/*[AUX]*/    return 1;
/*     */  }
/*     */  int k = 2;
/*     */  for(int j = 0; j < i-1; j++){
/*     */    k*=2;
/*[AUX]*/  }
/*     */  return k; 
/*     */}

/**
 * Tries to disconnect safely from control computer
 */
bool discon(){
  packet(LIDAR_DISCON).sendThis();
  
  if(waitForSerialTimeout()){
    return false;
  }else{
    if(getPakt().getHeader() == LIDAR_DSCACK){
      return true;
    }
    return false;
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
/*[LIB]*/void doOSLRFLibraryLoop(){
/*     */  getSyncPeriod();      // This "Sync" period update can be done not on every loop cycle,
/*     */  AverageSyncPeriod();  // as it takes time and loses some (3) distance measurements
/*     */
/*     */  // Detect Zero signal rising edge
/*[LIB]*/  while ((zero_val = analogRead(ZERO_PIN)) < zero_thresh) 
/*     */  zero_time1 = micros();
/*     */  
/*     */  // Detect Return signal rising edge based on previous measurement amplitude
/*     */  while ((return_val = analogRead(RETURN_PIN)) < return_thresh) 
/*[LIB]*/  echo_time1 = micros();
/*     */
/*     */
/*     */  // Get maximum height of Return pulse...
/*     */  amp = 0;
/*[LIB]*/  while (amp <= return_val ) { 
/*     */    amp = return_val;
/*     */    return_val = analogRead(RETURN_PIN);
/*     */  }
/*     */
/*[LIB]*/  // Detect Return signal falling edge 
/*     */  while ((return_val = analogRead(RETURN_PIN)) > return_thresh) 
/*     */  echo_time2 = micros();
/*     */
/*     */  // New Return signal threshold for next measurement, based on the new amplitude
/*[LIB]*/  return_thresh = 0.13 * (float)amp + 10;                                // Pure empiric, based on observations.
/*     */  if (return_thresh < 18) return_thresh = 18;                            // Make sure that threshold is over the noise
/*     */  
/*     */  zero_time = zero_time1 + 3500;                                         // Midpoint of Zero. Full Zero signal width - 7000us, when threshold 40
/*     */  echo_time = echo_time1 + ((float)echo_time2 - (float)echo_time1)/3.0;
/*[LIB]*/
/*     */  raw_distance = (float)(echo_time - zero_time)/(float)avgSync * 1833.0;
/*     */  
/*     */  if (raw_distance > 1000) {}                                            // Just ignore this reading
/*     */  else{
/*[LIB]*/    if (raw_distance < 220){                                             // RAW measure corrections if distance less than 100 cm
/*     */    distance = 0.725 * raw_distance - 56.208;
/*     */    }
/*     */    else distance = 1.078 * raw_distance - 134.05;                       // Empiric correction for 100cm and up
/*     */  }
/*[LIB]*/  
/*     */  AverageDistanceReadings();
/*     */}

/**
* All code below this line taken (with very slight alterations) from berryjam.eu/2014/06/oslrf-01/
*/

/*[LIB]*/void getSyncPeriod(){
/*     */  //  sync_period = 2*pulseIn(SYNC_PIN, LOW); // was too big about 80us, because duty cycle not perfect 50%
/*     */
/*     */  // Need to optimize, as it takes full two clocks
/*     */  unsigned long sync_period1 = pulseIn(SYNC_PIN, LOW);
/*[LIB]*/  unsigned long sync_period2 = pulseIn(SYNC_PIN, HIGH);
/*     */  sync_period = sync_period1 + sync_period2;
/*     */
/*     */}  

/*[LIB]*/void AverageSyncPeriod(){
/*     */  total2 = total2 - readings2[index2]; // subtract the last reading
/*     */  readings2[index2] = sync_period;     // Get last measure
/*     */  total2 = total2 + readings2[index2]; // add the reading to the total
/*     */  index2 = index2 + 1;                 // advance to the next position in the array
/*[LIB]*/  if (index2 >= numReadingsSync)       // if we're at the end of the array...
/*     */  index2 = 0;                          // ...wrap around to the beginning
/*     */  avgSync = total2 / numReadingsSync;     // calculate the average
/*     */}

/*[LIB]*/void AverageDistanceReadings(){
/*     */  total = total - readings[index];   // subtract the last reading
/*     */  readings[index] = distance;        // Get last measure
/*     */  total = total + readings[index];   // add the reading to the total
/*     */  index = index + 1;                 // advance to the next position in the array
/*[LIB]*/  if (index >= numReadings)          // if we're at the end of the array...
/*     */  index = 0;                         // ...wrap around to the beginning
/*     */  avgDist = total / numReadings;     // calculate the average
/*     */}
