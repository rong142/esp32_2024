#include <WiFi.h>
//請修改以下參數--------------------------------------------
char ssid[] = "C220MIS"; //請修改為您連線的網路名稱
char password[] = "misc220c220"; //請修改為您連線的網路密碼

int Gled = 15; //宣告綠色Led在 GPIO 15
int Yled = 2;
int Rled = 4;
int Relay = 16; //繼電器
WiFiServer server(80); //宣告伺服器位在80 port

void setup()
{
  Serial.begin(115200);
  Serial.print("開始連線到無線網路SSID:");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int tryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (tryCount++ >= 20){
      ESP.restart();
      }
    delay(1000);
  }
  Serial.println("連線完成");
  server.begin();
  Serial.print("伺服器已啟動，http://");
  Serial.println(WiFi.localIP());
  pinMode(Gled, OUTPUT);
  pinMode(Yled, OUTPUT);
  pinMode(Rled, OUTPUT);
  pinMode(Relay, OUTPUT);
}

void loop()
{
  //宣告一個連線
  WiFiClient client = server.available(); //client使用者的瀏覽器
  if (client) {
    //有人連入時
    Serial.println("使用者連入");
    //-------------網頁的html部份開始--------------
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html><head><meta charset='utf-8'></head>");
    client.println("<br>");
    client.println("<h1>Server</h1>");
    //HTML超連結指令
    client.println("<a href='/Gled=ON'>開啟綠色LED</a><br>");
    client.println("<a href='/Gled=OFF'>關閉綠色LED</a><br>");
    client.println("<br>");
    client.println("<a href='/Yled=ON'>開啟黃色LED</a><br>");
    client.println("<a href='/Yled=OFF'>關閉黃色LED</a><br>");
    client.println("<br>");
    client.println("<a href='/Rled=ON'>開啟紅色LED</a><br>");
    client.println("<a href='/Rled=OFF'>關閉紅色LED</a><br>");
    client.println("<br>");
    client.println("<a href='/Relay=ON'>開啟風扇</a><br>");
    client.println("<a href='/Relay=OFF'>關閉風扇</a><br>");

    client.println("<div class='form-check form-switch'>");
    client.println("<input class='form-check-input' type='checkbox' role='switch' id='flexSwitchCheckDefault'>");
    client.println("<label class='form-check-label' for='flexSwitchCheckDefault'>Default switch checkbox input</label>");
    client.println("</div>");

    client.println("</html>");
    //-------------網頁的html部份結束--------------
    //取得使用者輸入的網址
    String request = client.readStringUntil('\r');
    Serial.println(request);
    //判斷超連結指令
    //網址內包含Gled=ON，就開啟綠燈，如果Gled=OFF，關閉綠燈
    if (request.indexOf("Gled=ON") >= 0) { digitalWrite(Gled, HIGH); } //indexOf 字串搜尋，回復找到的位置，如果沒有搜尋到，回傳-1
    if (request.indexOf("Gled=OFF") >= 0) { digitalWrite(Gled, LOW); }

    if (request.indexOf("Yled=ON") >= 0) { digitalWrite(Yled, HIGH); } //indexOf 字串搜尋，回復找到的位置，如果沒有搜尋到，回傳-1
    if (request.indexOf("Yled=OFF") >= 0) { digitalWrite(Yled, LOW); }

    if (request.indexOf("Rled=ON") >= 0) { digitalWrite(Rled, HIGH); } //indexOf 字串搜尋，回復找到的位置，如果沒有搜尋到，回傳-1
    if (request.indexOf("Rled=OFF") >= 0) { digitalWrite(Rled, LOW); }

    if (request.indexOf("Relay=ON") >= 0) { digitalWrite(Relay, HIGH); } //indexOf 字串搜尋，回復找到的位置，如果沒有搜尋到，回傳-1
    if (request.indexOf("Relay=OFF") >= 0) { digitalWrite(Relay, LOW); }
    Serial.println("完成");
    client.stop();//停止連線
  }
}
