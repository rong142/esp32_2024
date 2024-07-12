//本程式請使用ESP32 1.0.4版本進行燒錄

#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "FS.h"
#include "SD_MMC.h"  // SD Card ESP32
#include <EEPROM.h>  // read and write from flash memory
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>  //請先安裝PubSubClient程式庫
#include "Base64.h"
#include <SimpleDHT.h>
// ------ 以下修改成你的設定 ------
int vFlip = 0;    //1=上下翻轉
int hMirror = 0;  //1=左右翻轉
char* ssid = "C220MIS";
char* password = "misc220c220";
char* MQTTServer = "mqttgo.io";   //免註冊MQTT伺服器
int MQTTPort = 1883;              //MQTT Port
char* MQTTUser = "";              //不須帳密
char* MQTTPassword = "";          //不須帳密
char* MQTTTopicPic1 = "Godzilla/class205/image";    //推播主題1:即時影像
long MQTTLastPublishTime;         //此變數用來記錄推播時間
long MQTTPublishInterval = 200;  //推撥1秒2張

//鏡頭
sensor_t* s;

//------ 以下修改成你腳位 ------
int pinDHT11 = 13;//DHT11
SimpleDHT11 dht11(pinDHT11);
int pinGLED = 15;//綠色LED
int pinYLED = 2;
int pinRLED = 4;
int pinLight = 33;
int pinFan = 14;

// ------ 以下修改成你MQTT設定 ------
//char* MQTTServer = "mqttgo.io";//免註冊MQTT伺服器
//int MQTTPort = 1883;//MQTT Port
//char* MQTTUser = "";//不須帳密
//char* MQTTPassword = "";//不須帳密
//推播主題1:推播溫度(記得改Topic)
char* MQTTPubTopic1 = "Godzilla/class205/temp";
//推播主題2:推播濕度(記得改Topic)
char* MQTTPubTopic2 = "Godzilla/class205/humi";
//訂閱主題1:改變LED燈號(記得改Topic)
char* MQTTSubTopic1 = "Godzilla/class205/Gled";
char* MQTTSubTopic2 = "Godzilla/class205/Yled";
char* MQTTSubTopic3 = "Godzilla/class205/Rled";
char* MQTTvalue = "Godzilla/class205/value"; //光敏電阻
char* MQTTFan = "Godzilla/class205/Fan";
//
long MQTTLastPublishTime_DHT;//此變數用來記錄推播時間
long MQTTPublishInterval_DHT = 10000;//每10秒推撥一次

WiFiClient WifiClient;
PubSubClient MQTTClient(WifiClient);


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector 取消電流不穩時，重開機
  Serial.begin(115200);
  pinMode(pinLight, OUTPUT);
  pinMode(pinGLED, OUTPUT);//綠色LED燈
  pinMode(pinYLED, OUTPUT);
  pinMode(pinRLED, OUTPUT);
  pinMode(pinFan, OUTPUT);
  setupCam();
  pinMode(0, INPUT_PULLUP);  //INPUT, INPUT_PULLUP,INPUT_PULLDOWN
  pinMode(4,OUTPUT);
  //digitalWrite(4,LOW);
}

int i = 0;  //檔案序號
void loop() {
  //如果Wifi連線中斷，則重啟Wifi連線
  if (WiFi.status() != WL_CONNECTED) WiFiConnect();
  //如果MQTT連線中斷，則重啟MQTT連線
  if (!MQTTClient.connected()) MQTTConnecte();

  if ((millis() - MQTTLastPublishTime) >= MQTTPublishInterval) {
    String result = SendImageMQTT_Base64();
    Serial.println(result);
    MQTTLastPublishTime = millis();  //更新最後傳輸時間
    //Serial.print(MQTTLastPublishTime); 
  }

//如果距離上次傳輸已經超過10秒，則Publish溫溼度
  if ((millis() - MQTTLastPublishTime_DHT) >= MQTTPublishInterval_DHT ) {
    //讀取溫濕度
    byte temperature = 0;
    byte humidity = 0;
    while (temperature==0 && humidity==0){
      ReadDHT(&temperature, &humidity);
    }
    //ReadDHT(&temperature, &humidity);

    int value = analogRead(pinLight);
    value = map(value,0,4095,100,0);
    Serial.print((int)value);
    Serial.println(" %");
    
    // ------ 將DHT11溫度送到MQTT主題 ------
    MQTTClient.publish(MQTTPubTopic1, String(temperature).c_str());
    MQTTClient.publish(MQTTPubTopic2, String(humidity).c_str());
    MQTTClient.publish(MQTTvalue, String(value).c_str());
    Serial.println("溫溼度已推播到MQTT Broker");
    MQTTLastPublishTime_DHT = millis(); //更新最後傳輸時間
  }
  
  MQTTClient.loop();  //更新訂閱狀態
  delay(1000);
}

//讀取DHT11溫濕度
void ReadDHT(byte *temperature, byte *humidity) {
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(temperature, humidity, NULL)) !=
      SimpleDHTErrSuccess) {
    Serial.print("讀取失敗,錯誤訊息="); 
    Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); 
    Serial.print(SimpleDHTErrDuration(err)); 
    Serial.print(","); 
    Serial.println(millis()); 
    delay(10);
    return;
  }
  Serial.print("DHT讀取成功：");
  Serial.print((int)*temperature); 
  Serial.print(" *C, ");
  Serial.print((int)*humidity); 
  Serial.println(" H");
}

//連線到WiFi
void WiFiConnect() {
  Serial.print("開始連線到:");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  int tryCount = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (tryCount++ >= 20){
      ESP.restart(); //無法連線則重開機
    }
  }
  Serial.println("");
  Serial.print("WiFi connected,IP address:");
  Serial.println(WiFi.localIP());
}

//開始MQTT連線伺服器
void MQTTConnecte() {
  MQTTClient.setServer(MQTTServer, MQTTPort);
  MQTTClient.setCallback(MQTTCallback);
  while (!MQTTClient.connected()) {
    //以亂數為ClientID
    String MQTTClientid = "esp32-" + String(random(1000000, 9999999));
    if (MQTTClient.connect(MQTTClientid.c_str(), MQTTUser, MQTTPassword)) {
      //連結成功，顯示「已連線」。
      Serial.println("MQTT已連線");
      MQTTClient.subscribe(MQTTSubTopic1);
      MQTTClient.subscribe(MQTTSubTopic2);
      MQTTClient.subscribe(MQTTSubTopic3);
      MQTTClient.subscribe(MQTTFan);
    } else {
      //若連線不成功，則顯示錯誤訊息，並重新連線
      Serial.print("MQTT連線失敗,狀態碼=");
      Serial.println(MQTTClient.state());
      Serial.println("五秒後重新連線");
      delay(1000);
    }
  }
}

//拍照傳送到MQTT
String SendImageMQTT() {
  camera_fb_t* fb = esp_camera_fb_get();
  size_t fbLen = fb->len;
  //int ps = 512;
  //開始傳遞影像檔，批次傳
  MQTTClient.beginPublish(MQTTTopicPic1, fbLen, true);
  uint8_t* fbBuf = fb->buf;
  for (size_t n = 0; n < fbLen; n = n + 2048) {
    if (n + 2048 < fbLen) {
      MQTTClient.write(fbBuf, 2048);
      fbBuf += 2048;
    } else if (fbLen % 2048 > 0) {
      size_t remainder = fbLen % 2048;
      MQTTClient.write(fbBuf, remainder);
    }
  }
  boolean isPublished = MQTTClient.endPublish();
  esp_camera_fb_return(fb);  //清除緩衝區
  if (isPublished) {
    return "MQTT傳輸成功";
  } else {
    return "MQTT傳輸失敗，請檢查網路設定";
  }
}

//拍照傳送到MQTT Base64
String SendImageMQTT_Base64() {
  int Buffer = 4096;
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }

  char* input = (char*)fb->buf;
  char output[base64_enc_len(3)];
  String imageFile = "data:image/jpeg;base64,";
  for (int i = 0; i < fb->len; i++) {
    base64_encode(output, (input++), 3);
    if (i % 3 == 0) imageFile += String(output);
  }
  int fbLen = imageFile.length();
  MQTTClient.beginPublish(MQTTTopicPic1, fbLen, true);
  String str = "";
  for (size_t n = 0; n < fbLen; n = n + Buffer) {
    if (n + Buffer < fbLen) {
      str = imageFile.substring(n, n + Buffer);
      MQTTClient.write((uint8_t*)str.c_str(), Buffer);
    } else if (fbLen % Buffer > 0) {
      size_t remainder = fbLen % Buffer;
      str = imageFile.substring(n, n + remainder);
      MQTTClient.write((uint8_t*)str.c_str(), remainder);
    }
  }

  boolean isPublished = MQTTClient.endPublish();
  esp_camera_fb_return(fb);  //清除緩衝區
  if (isPublished) {
    return "MQTT傳輸成功";
  } else {
    return "MQTT傳輸失敗，請檢查網路設定";
  }
}

//接收到訂閱時
void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print("訂閱通知:");
  String payloadString;  //將接收的payload轉成字串
  //顯示訂閱內容
  for (int i = 0; i < length; i++) {
    payloadString = payloadString + (char)payload[i];
  }
  Serial.println(payloadString);

  if (strcmp(topic, MQTTSubTopic1) == 0) {
    Serial.println("改變燈號：" + payloadString);
    if (payloadString == "1") digitalWrite(pinGLED, HIGH);
    if (payloadString == "0") digitalWrite(pinGLED, LOW);
  }
  if (strcmp(topic, MQTTSubTopic2) == 0) {
    Serial.println("改變燈號：" + payloadString);
    if (payloadString == "1") digitalWrite(pinYLED, HIGH);
    if (payloadString == "0") digitalWrite(pinYLED, LOW);
  }
  if (strcmp(topic, MQTTSubTopic3) == 0) {
    Serial.println("改變燈號：" + payloadString);
    if (payloadString == "1") digitalWrite(pinRLED, HIGH);
    if (payloadString == "0") digitalWrite(pinRLED, LOW);
  }
  if (strcmp(topic, MQTTFan) == 0) {
    Serial.println("開啟：" + payloadString);
    if (payloadString == "1") digitalWrite(pinFan, HIGH);
    if (payloadString == "0") digitalWrite(pinFan, LOW);
  }
}

//鏡頭設定
void setupCam() {
  // #define CAMERA_MODEL_AI_THINKER
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  //                default = 20000000;
  config.xclk_freq_hz = 10000000;  //10000000 or 5000000
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  // FRAMESIZE_ + QVGA:320x240|VGA:640x480|SVGA:800x600|XGA:1024x768|SXGA:1280x1024|UXGA:1600x1200
  config.jpeg_quality = 10;            //< Quality of JPEG output. 0-63 lower means higher quality
  config.fb_count = 2;                 //Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed)

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // sensor_t * s = esp_camera_sensor_get();
  s = esp_camera_sensor_get();

  s->set_brightness(s, 1);  //亮度 -2 to 2
  s->set_contrast(s, 1);    //對比 -2 to 2
  s->set_saturation(s, 1);  //飽和, Hue 色相
  s->set_wb_mode(s, 0);     // 0: auto 自動, 1: sun 太陽, 2: cloud 雲, 3: indoors 室內
  //s->set_exposure_ctrl(s, 1);
  //s->set_aec_value(s, -2);
  //s->set_ae_level(s, 100);
  //s->set_gain_ctrl(s, 100);
  //s->set_pixformat(s, PIXFORMAT_RGB565);
  //s->set_pixformat(s, PIXFORMAT_JPEG);
  s->set_vflip(s, vFlip);              //垂直翻轉
  s->set_hmirror(s, hMirror);          //水平鏡像
  s->set_framesize(s, FRAMESIZE_QVGA);  // 640 x 480

  Serial.println("Camera Setup OK");
}
