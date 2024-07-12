#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

char ssid[] = "Rong";
char password[] = "09870987";
char url[] = "https://data.moenv.gov.tw/api/v2/aqx_p_02?api_key=e8dd42e6-9b8b-43f8-991e-b3dee723a52d&limit=1000&sort=datacreationdate%20desc&format=JSON";

void setup() {

  pinMode(15,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(36,INPUT);
  Serial.begin(115200);

  Serial.begin(115200);
  delay(1000);
  Serial.print("開始連線到無線網路SSID：");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);

  Serial.println("準備連線網路");
  WiFi.begin(ssid, password);

  int tryCount = 0;
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
    if (tryCount++ >= 20){
      ESP.restart();
    }
  }
  Serial.println("連線完成");
}

void loop() {
  
  int value = analogRead(36);

  Serial.println("啟動網頁連線");
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  Serial.print("httpCode = ");
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK){
    String payload = http.getString();
    Serial.print("payload = ");

    Serial.println(payload);

    DynamicJsonDocument AQJarray(payload.length()*2);
    deserializeJson(AQJarray, payload);
    for (int i = 0; i < AQJarray["records"].size(); i++){
      if (AQJarray["records"][i]["site"] == "楠梓"){
        int kspm25 = AQJarray["records"][i]["pm25"];
        Serial.print("楠梓PM2.5 = " + kspm25);
        Serial.println(kspm25);

        if (kspm25 >= 301){
          analogWrite(15,255);
          analogWrite(2,0);
          analogWrite(4,255);
        }else if (kspm25 >= 201 && kspm25 <301){
          analogWrite(15,255);
          analogWrite(2,0);
          analogWrite(4,0);
        }else if (kspm25 >= 101 && kspm25- <201){
          analogWrite(15,255);
          analogWrite(2,255);
          analogWrite(4,0);
        }else{
          analogWrite(15,0);
          analogWrite(2,255);
          analogWrite(4,0);
        }
        
        break;
      }
    }
  }

  http.end();
  delay(10000);
}
