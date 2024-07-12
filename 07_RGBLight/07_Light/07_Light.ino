//#include <analogWrite.h>

void setup() {
  
  // put your setup code here, to run once:
  pinMode(15,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(36,INPUT);
  Serial.begin(115200);

}

void loop() {
  
  // put your main code here, to run repeatedly:
  int value = analogRead(36);
  //value = map(value,0,4095,100,0);
  Serial.println(value);

  if (value >= 150){
    analogWrite(15,255);
    analogWrite(2,0);
    analogWrite(4,255);
  }else if (value >= 100 && value <150){
    analogWrite(15,255);
    analogWrite(2,0);
    analogWrite(4,0);
  }else if (value >= 80 && value <100){
    analogWrite(15,255);
    analogWrite(2,255);
    analogWrite(4,0);
  }else{
    analogWrite(15,0);
    analogWrite(2,255);
    analogWrite(4,0);
  }
  
  delay(100);

}
