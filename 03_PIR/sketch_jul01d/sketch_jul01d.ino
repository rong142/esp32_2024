void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(17,INPUT);
  //pinMode(16,OUTPUT);

  pinMode(15,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(4,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = digitalRead(17);
  if (value == HIGH){
    //Serial.println("")
    digitalWrite(4,LOW);
    digitalWrite(15,HIGH);
    Serial.println("有人ㄚㄚㄚㄚㄚ");
  }else{
    digitalWrite(15,LOW);
    digitalWrite(4,HIGH);
    Serial.println("沒人了");
    delay(1000);
  }
//  Serial.println(value);
//  delay(500);
}
