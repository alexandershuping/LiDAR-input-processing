const int p_flash = 12;

void setup() {
  Serial.begin(9600);
  pinMode(p_flash, OUTPUT);
}

void loop() {
  if(Serial.available()){
      Serial.write(0x01);
      for(int i = 0; i < 8; i++){
        Serial.write(0);
      }
      while(Serial.available()){
        Serial.read();
      }
      digitalWrite(p_flash, HIGH);
  }
}
