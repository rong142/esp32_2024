void setup() {
  
  // put your setup code here, to run once:
  pinMode(36,INPUT);
  Serial.begin(115200);

}

void loop() {
  
  // put your main code here, to run repeatedly:
  int value = analogRead(36);
  //value = map(value,0,4095,100,0);
  Serial.println(value);
  delay(100);

}
