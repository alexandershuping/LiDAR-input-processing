#define ZERO_PIN     A1  // Arduino pin tied to Zero   pin on the OSLRF.
#define RETURN_PIN   A3  // Arduino pin tied to Return pin on the OSLRF.
#define SYNC_PIN     2   // Arduino pin tied to Sync   pin on the OSLRF.

#define pi 3.1415926535
#define p_advnc 3

//Wasteful library code
  int zero_val = 0;
  int return_val = 0;
  int sync_val_1 = 0;
  int sync_val_2 = 0;
  int amp = 0;
  
  int zero_thresh = 120;
  int return_thresh = 120;
  
  float raw_distance = 0.0;
  float distance = 0.0;
  
  unsigned long zero_time;
  unsigned long zero_time1;
  unsigned long zero_time2;
  
  unsigned long echo_time;
  unsigned long echo_time1;
  unsigned long echo_time2;
  
  unsigned long sync_time_1;
  unsigned long sync_time_2;
  unsigned long sync_period;

  
  // Distance average variables
  const int numReadings = 20;
  int readings[numReadings];      // the readings from the analog input
  int index = 0;                  // the index of the current reading
  int total = 0;                  // the running total
  int avgDist = 0;                // the average

  // Sync period average variables
  const int numReadings2 = 40;
  unsigned long readings2[numReadings2];      // the readings from the analog input
  int index2 = 0;                             // the index of the current reading
  unsigned long total2 = 0;                   // the running total
  unsigned long avgSync = 0;                  // the average
//Wasteful library code

const char eot = 60; //ASCII character for End of Transmission -- sent by Arduino at end of data packet
const char enq = 61; //ASCII character for Enquiry             -- sent to test connection
const char ack = 62; //ASCII character for Acknowledge         -- Received in response to enq
const char nln = 63;//ADCII character for line feed           -- sent to get data 

double i = 0;
double j = 0;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(SYNC_PIN, INPUT);
  pinMode(p_advnc, INPUT_PULLUP);
  digitalWrite(13, LOW);

  //Libarary code
  // Sync averaging itialize routine
 for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0; 

 // Distance averaging itialize routine
 for (int thisReading2 = 0; thisReading2 < numReadings2; thisReading2++)
    readings2[thisReading2] = 0;      
}

void loop() {
  if(Serial.available()){
    char c = Serial.read();
    if(c=='e'){
      Serial.write('a');
      serFlush();
    }else if(c=='d'){
      while(digitalRead(p_advnc)==HIGH){};
      int nt = (int)(i*10);
      String st = (String)nt;
      Serial.print(st);
      Serial.print('\n');
      nt = (int)(j*10);
      st = (String)nt;
      Serial.print(st);
      Serial.print('\n');
      i++;
      nt = (int)avgDist*10;
      st = (String)nt;
      Serial.print(st);
      Serial.print('\n');
      delay(1000);
      serFlush();
    }else{
      serFlush();
    }
  }

  getSyncPeriod();      // This "Sync" period update can be done not on every loop cycle,
  AverageSyncPeriod();  // as it takes time and looses some (3) distance measurements

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
  return_thresh = 0.13 * (float)amp + 10;         // Pure empiric, based on observations.
  if (return_thresh < 18) return_thresh = 18;     // Make sure that threshold is over the noise
  
  zero_time = zero_time1 + 3500;                  // Midpoint of Zero. Full Zero signal width - 7000us, when threshold 40
  echo_time = echo_time1 + ((float)echo_time2 - (float)echo_time1)/3.0;

  raw_distance = (float)(echo_time - zero_time)/(float)avgSync * 1833.0;
  
  if (raw_distance > 1000) {}       // Just ignore this reading
  else{
    if (raw_distance < 220){ // RAW measure corrections if distance less than 100 cm
    distance = 0.725 * raw_distance - 56.208;
    }
    else distance = 1.078 * raw_distance - 134.05; // Empiric correction for 100cm and up
  }
  
  AverageDistanceReadings();
}


void serFlush(){
  while(Serial.available()){Serial.read();}
}

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
  total2 = total2 + readings2[index2];  // add the reading to the total
  index2 = index2 + 1;                 // advance to the next position in the array
  if (index2 >= numReadings2)          // if we're at the end of the array...
  index2 = 0;                          // ...wrap around to the beginning
//  avgSync = 1;
  avgSync = total2 / numReadings2;     // calculate the average
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
