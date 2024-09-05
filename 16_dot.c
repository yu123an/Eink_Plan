#include <Arduino.h>
#include "AiEsp32RotaryEncoder.h"
#include <msg.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // WS2812驱动
#endif
#include <Wire.h>
#include "RTClib.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h> //网络校时
#include <WiFiUdp.h>   //WiFi支持
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>  //JSON解析
#include <PubSubClient.h> //MQTT
#include <Ticker.h>       //定时器
#include "Adafruit_FRAM_I2C.h"
#define LED_NUM 256
#define LED 15
// 编码器
#define ROTARY_ENCODER_A_PIN 14
#define ROTARY_ENCODER_B_PIN 12
#define ROTARY_ENCODER_BUTTON_PIN 13
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
// EEPROM地址
#define EEPROM_ADDR 0x50
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (25000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<25000> Mqtt_Sub; // JSON解析缓存
int value = 0;
String JsonMsg;
int color_r = 255, color_g = 36, color_b = 234;
int gif_num;
int light = 10;
int Sun_rise_hour, Sun_rise_minute, Sun_set_hour, Sun_set_minute;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// 构造对象
Adafruit_NeoPixel WS(LED_NUM, LED, NEO_GRB + NEO_KHZ800); // WS2812
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "1.openwrt.pool.ntp.org", 3600 * 8, 60000);
RTC_PCF8563 rtc;
WiFiClient espClient;
Ticker time_update;
Ticker msg_update;
Ticker ticker;
Adafruit_EEPROM_I2C i2ceeprom;
uint8_t number[10][8] = {
0x00,0x00,0xFC,0x0F,0x0C,0x0C,0xFC,0x0F,
0x00,0x00,0x0C,0x0C,0xFC,0x0F,0x00,0x0C,
0x00,0x00,0xDC,0x0F,0xCC,0x0C,0xFC,0x0C,
0x00,0x00,0x1C,0x0E,0xCC,0x0C,0x3C,0x0F,
0x00,0x00,0xFC,0x01,0x80,0x01,0xFC,0x0F,
0x00,0x00,0xFC,0x0E,0xCC,0x08,0xCC,0x0F,
0x00,0x00,0xFC,0x0F,0xCC,0x08,0xDC,0x0F,
0x00,0x00,0x0C,0x00,0x8C,0x0F,0xFC,0x0F,
0x00,0x00,0x3C,0x0F,0xCC,0x0C,0x3C,0x0F,
0x00,0x00,0xFC,0x0C,0xCC,0x08,0xFC,0x0F,};
//数字输出
void draw_X(int x, int num, int r, int g, int b) {
  for (int X = 0; X < 8; X++) {
    for (int Y = 0; Y < 8; Y++) {
      WS.setPixelColor(8 * (x + 1) + 8 * (X - 1) + Y, ((number[num][X] >> Y) & 0x01) * r, ((number[num][X] >> Y) & 0x01) * g, ((number[num][X] >> Y) & 0x01) * b);
    }
  }
}
// 编码器相关
void rotary_onButtonClick()
{
  static unsigned long lastTimePressed = 0;
  // ignore multiple press in that time milliseconds
  if (millis() - lastTimePressed < 500)
  {
    return;
  }
  lastTimePressed = millis();
  Serial.print("button pressed ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}
void rotary_loop()
{
  // dont print anything unless value changed
  if (rotaryEncoder.encoderChanged())
  {
    Serial.print("Value: ");
    Serial.println(rotaryEncoder.readEncoder());
  }
  if (rotaryEncoder.isEncoderButtonClicked())
  {
    rotary_onButtonClick();
  }
}
void IRAM_ATTR readEncoderISR()
{
  rotaryEncoder.readEncoder_ISR();
}
// 读写EEPROM
void write_eeprom(int addr, String velue)
{
  int lenth = velue.length();
  for (int a = 0; a < lenth; a++)
  {
    EEPROM.write(addr + a, velue[a]);
  }
}
String read_eeprom(int addr, int lenth)
{
  String Text;
  for (int a = 0; a < lenth; a++)
  {
    Text += char(EEPROM.read(addr + a));
  }
  return Text;
  Serial.print(Text);
}
// 联网支持
void get_net(String web, bool isdecode)
{
  HTTPClient http;
  if (http.begin(espClient, web))
  {
    Serial.println("HTTPclient setUp done!");
  }
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      // Serial.println(payload);
      if (isdecode)
      {
        deserializeJson(Mqtt_Sub, payload);
      }
      else
      {
        JsonMsg = payload;
      }
      // Serial.println(payload);
    }
  }
  else
  {

    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
// http字符串解析
void JsonEecoed(String json)
{
  deserializeJson(Mqtt_Sub, json);
}
// 主函数
void setup()
{
  EEPROM.begin(200);
  Wire.begin(5, 4);
  
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  bool circleValues = false;
  rotaryEncoder.setBoundaries(0, 255, circleValues);
  rotaryEncoder.setAcceleration(250);
  ticker.attach_ms(500, rotary_loop);
  Wire.setClock(1000000);
  WS.begin(); // WS2812驱动使能
  Serial.begin(115200);
  if (i2ceeprom.begin(0x50))
  { // EEProm使能
    Serial.println("Found I2C EEPROM");
  }
  else
  {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1)
      delay(10);
  }
  WiFi.begin(ssid, password); // 连接网络
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  { // 等待连接
    delay(500);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  // get_net();
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  timeClient.begin(); // 获取网络时间
  timeClient.update();
if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }

 rtc.adjust(DateTime(2024, 9, 3, timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds()));
 
  rtc.start();

  // get_net(gif_msg, 1);
  // gif_num = i2ceeprom.read(0);
  // String gif_name = Mqtt_Sub["name"].as<String>();
  // if (gif_num != Mqtt_Sub["num"].as<int>())
  // {
  //   Serial.println("new gif");
  //   gif_num = Mqtt_Sub["num"].as<int>();
  //   i2ceeprom.write(0, gif_num);
  //   for (int num = 0; num < gif_num; num++)
  //   {
  //     get_net(gif_addr + gif_name + "--" + String(num) + "_bin.log", 1);
  //     for (int i = 0; i < 768; i++)
  //     {
  //       i2ceeprom.write(num * 768 + i + 2, Mqtt_Sub[i]["n"]);
  //     }
  //   }
  // }
  // else
  // {
  //   Serial.println("the same gif");
  // }
}
void loop()
{
  WS.setBrightness(light);
  // for (int mm = 0; mm < 10; mm++)
  // {
  //   int num_gif = i2ceeprom.read(0);
  //   for (int num = 0; num < num_gif; num++)
  //   {
  //     for (int i = 0; i < 256; i++)
  //     {
  //       WS.setPixelColor(i, i2ceeprom.read(num * 768 + 2 + i * 3 + 2), i2ceeprom.read(num * 768 + 2 + i * 3 + 1), i2ceeprom.read(num * 768 + 2 + i * 3 + 0));
  //     }
  //     WS.show();
  //     //  delay(1000 / num_gif);
  //     // ESP.wdtFeed();
  //     delay(20);
  //   }
  // }
  DateTime now = rtc.now();
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
  draw_X(0,now.hour() / 10,100,200,0);
  draw_X(8,now.hour() % 10,100,200,0);
   draw_X(16,now.minute() / 10,10,200,0);
  draw_X(24,now.minute() % 10,10,200,0);
  //   draw_X(16,now.second() / 10,10,200,0);
  // draw_X(24,now.second() % 10,10,200,0);
  WS.show();
  WS.show();
  delay(998);
}
