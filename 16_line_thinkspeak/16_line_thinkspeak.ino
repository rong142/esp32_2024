#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SimpleDHT.h>
#include <HTTPClient.h>

//請修改以下參數--------------------------------------------
char ssid[] = "C220MIS";
char password[] = "misc220c220";
String Linetoken = "IDWCeCQmovSIGsjjutQ6rpXQIvMrtMLsVSouuNLDA2p";
String url = "http://api.thingspeak.com/update?api_key=1AB2XA6OP6KGE7JP";
int pinDHT11 = 4;//假設DHT11 接在腳位GPIO4，DHT11 接線圖請參考P74
//---------------------------------------------------------
SimpleDHT11 dht11(pinDHT11);
WiFiClientSecure client;
char host[] = "notify-api.line.me";

void setup() {
  Serial.begin(115200);
  //連線到指定的WiFi SSID
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //連線成功，顯示取得的IP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");

  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  client.setInsecure(); // 讀者若使用ESP32 1.0.6 版核心須加上這句避免SSL 問題
}

void loop() {

  //嘗試讀取溫濕度內容
  byte temperature = 0;
  byte humidity = 0;
  //byte light = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.println(err); delay(1000);
    return;
  }
  //讀取成功，將溫濕度顯示在序列視窗
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, ");
  Serial.print((int)humidity); Serial.print(" H, ");

  int value = analogRead(36);
  value = map(value,0,4095,100,0);
  Serial.print((int)value); Serial.println(" %");

  //開始傳送到thingspeak
  Serial.println("啟動網頁連線");
  HTTPClient http;

  //將溫度及濕度以HTTP Get參數方式補入網址後方
  String url1 = url + "&field1=" + String(temperature) + "&field2=" + String(humidity) + "&field3=" + String(value);
  
  //http client取得網頁內容
  http.begin(url1);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)      {
    //讀取網頁內容到payload
    String payload = http.getString();
    //將內容顯示出來
    Serial.print("網頁內容=");
    Serial.println(payload);
  } else {
    //讀取失敗
    Serial.println("網路傳送失敗");
  }
  http.end();

  //設定觸發LINE訊息條件為溫度超過35或濕度超過80
  if ((int)temperature >= 30 || (int)humidity >= 60 || (int)value >= 80) {
    //傳遞Line訊息
    String message = "檢測環境發生異常，請協助儘速派人查看處理，目前環境狀態：";
    message += "\n溫度=" + String(temperature) + " *C";
    message += "\n濕度=" + String(humidity) + " H";
    message += "\n亮度=" + String(value) + "  要瞎了";
    Serial.println(message);
    //連線到Line API網站
    if (client.connect(host, 443)) {      
      int LEN = message.length();
      //1.傳遞網站
      String url = "/api/notify"; //Line API網址
      client.println("POST " + url + " HTTP/1.1");
      client.print("Host: "); client.println(host);//Line API網站      
      //2.資料表頭
      client.print("Authorization: Bearer "); client.println(Linetoken);
      //3.內容格式
      client.println("Content-Type: application/x-www-form-urlencoded");
      //4.資料內容
      client.print("Content-Length: "); client.println( String((LEN + 8)) ); //訊息長度      
      client.println();      
      client.print("message="); client.println(message); //訊息內容      
      //等候回應
      delay(2000);
      String response = client.readString();
      //顯示傳遞結果
      Serial.println(response);
      client.stop();//斷線，否則只能傳5次
    }
    else {
      //傳送失敗
      Serial.println("connected fail");
    }
  }
  
  //每5秒讀取一次溫濕度
  delay(15000);
}
