void setup() {
  // put your setup code here, to run once:
  pinMode(15,OUTPUT);
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:

  for (int i = 0 ; i <= 255 ; i++){
    analogWrite(15,i);
    delay(20);
  }
  
  delay(300);

  for (int i = 255 ; i >= 0 ; i--){
    analogWrite(15,i);
    delay(20);
  }

  delay(1000);
  
  //analogWrite(15,10);

}
