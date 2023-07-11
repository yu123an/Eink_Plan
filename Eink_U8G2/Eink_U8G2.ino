/*
墨水屏驱动下的时钟与日历小摆件
*/
/*
 *
RST REASON
0 unknown
1 normal boot
2 reset pin
3 software reset
4 watchdog reset
 */
// 引入的库
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h> //图形绘制
#include <U8g2_for_Adafruit_GFX.h>
#include <RtcDS1307.h>
#include <Wire.h> //DS1307驱动
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>        //网络校时
#include <WiFiUdp.h>          //WiFi支持
#include "msg.h"              //敏感信息
#include <ArduinoJson.h>      //JSON解析
#include <WiFiClientSecure.h> //https请求
#include "Adafruit_EEPROM_I2C.h"
#include "Adafruit_FRAM_I2C.h" //EEPROM读写
#include "ClosedCube_SHT31D.h" //SHT30

// 字体文件
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/DS_DIGI_150pt7b.h>
#include <Fonts/MakoP_230pt7b.h>
#include "gb2312.c"
#include "logo.h"

// 颜色声明
uint16_t PAPER = GxEPD_WHITE;
uint16_t INK = GxEPD_BLACK;

// 变量声明
#define EEPROM_ADDR 0x50                               // EEPROM地址
static struct rst_info *rinfo = ESP.getResetInfoPtr(); // 休眠唤醒信息
int RST_reason = rinfo->reason;
String temp, humidity, weather0, weather1, moon0, moon1, sunrise, sunset, end_day, day_name; // 天气状况
int Temp_in, Humidity_in, moonphase;                                                         // 室内温湿度
int _day, _hour, _minute, _second, weekday;                                                  // 时间更新
float v_bat = 0;                                                                             // 电量检测
String JsonMsg;                                                                              // json串解析
int long_msg;
#define MSG_BUFFER_SIZE (2000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<2000> Mqtt_Sub;
unsigned long lastMsg = 0;
String week[] = {"日", "一", "二", "三", "四", "五", "六"};
// 实例化
RtcDS1307<TwoWire> Rtc(Wire);
WiFiUDP ntpUDP;
WiFiClient espClient;                                            // WiFi
NTPClient timeClient(ntpUDP, "ntp.ntsc.ac.cn", 8 * 3600, 60000); // DS1302
ClosedCube_SHT31D sht3xd;
Adafruit_EEPROM_I2C i2ceeprom;
// 2.9寸
//  GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
//  4.2寸
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12)); // GDEW042T2 400x300, UC8176 (IL0398)
U8G2_FOR_ADAFRUIT_GFX ug;

// 功能函数
// 获取SHT30温湿度
void get_sht30(String text, SHT31D result)
{
  if (result.error == SHT3XD_NO_ERROR)
  {
    Temp_in = result.t;
    Humidity_in = result.rh;
  }
  else
  {
    Serial.print(text);
    Serial.print(": [ERROR] Code #");
    Serial.println(result.error);
  }
}
// 更新时间
void time_update()
{
  timeClient.begin();
  timeClient.update();
  _day = timeClient.getDay();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  _second = timeClient.getSeconds();
  if (_minute != 0)
  {
    Serial.println("成功获取时间，准备更新时间。。。。");
    Rtc._SetDateTime(_second, _minute, _hour);
  }
}
// 联网支持
void get_net(String web)
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
      // if (isdecode) {
      //   deserializeJson(Mqtt_Sub, payload);
      // } else {
      //   JsonMsg = payload;
      // }
      JsonMsg = payload;
      Serial.println(payload);
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
// 连接互联网
void connect_net()
{
  WiFi.begin(ssidhome, passhome); // 连接网络
  int i = 0;
  Serial.print("Connecting to ");
  Serial.print(ssidhome);
  while (WiFi.status() != WL_CONNECTED)
  { // 等待连接
    delay(500);
    Serial.print('.');
    i++;
    if (i > 40)
    {
      Serial.println("联网失败，结束联网");
      break;
    }
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  // get_net();
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}
// 获取电量电压
void get_bat()
{
  analogRead(A0);
  double _bat = 0;
  for (int i = 0; i < 8; i++)
  {
    _bat += analogRead(A0);
  }
  //_bat > 3;
  v_bat = _bat * 5 / 1024 / 8;
}
// EEPROM读写
void write_eeprom(int addr, String string)
{
  int lenth = string.length();
  // i2ceeprom.write(addr,string,lenth);
  for (int a = 0; a < lenth; a++)
  {
    i2ceeprom.write(addr + a, string[a]);
  }
}
String read_eeprom(int addr, int lenth)
{
  String string;
  // i2ceeprom.read(addr,string,lenth);
  for (int a = 0; a < lenth; a++)
  {
    // i2ceeprom.write(addr + a,string[a]);
    string += char(i2ceeprom.read(addr + a));
  }
  return string;
}
// 绘制主屏幕
void draw_HomePage()
{
  // 读取天气信息
  long_msg = i2ceeprom.read(0);
  String New_msg = read_eeprom(1, long_msg);
  Serial.println("开始绘制屏幕");
  Serial.println(New_msg);
  deserializeJson(Mqtt_Sub, New_msg);
  end_day = Mqtt_Sub["day"].as<String>();
  day_name = Mqtt_Sub["day_name"].as<String>();
  weekday = Mqtt_Sub["week"].as<int>();
  temp = Mqtt_Sub["weather"]["temp"].as<String>();
  humidity = Mqtt_Sub["weather"]["humidity"].as<String>();
  weather0 = Mqtt_Sub["weather"]["weather0"].as<String>();
  moon0 = Mqtt_Sub["weather"]["moon0"].as<String>();
  weather1 = Mqtt_Sub["weather"]["weather1"].as<String>();
  moon1 = Mqtt_Sub["weather"]["moon1"].as<String>();
  sunrise = Mqtt_Sub["weather"]["sunrise"].as<String>();
  sunset = Mqtt_Sub["weather"]["sunset"].as<String>();
  moonphase = Mqtt_Sub["weather"]["moon"].as<int>() % 10;
  // 获取日程待办信息
  long_msg = i2ceeprom.read(300) * 200 + i2ceeprom.read(301);
  New_msg = read_eeprom(302, long_msg);
  Serial.println(New_msg);
  deserializeJson(Mqtt_Sub, New_msg);
  int totle = Mqtt_Sub["num"].as<int>();
  // Serial.println(end_day);
  // Serial.println(moon0);
  get_sht30("Periodic Mode", sht3xd.periodicFetchData());
  Rtc.Begin(); // DS1307时间读写
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
  ug.setForegroundColor(INK);
  ug.setBackgroundColor(PAPER);
  ug.setFont(chinese_gb2312);
  // 局刷
  //  display.setPartialWindow();
  display.setFullWindow();
  display.setRotation(2); // 屏幕方向；2.9选3；4.2选2或4
  display.firstPage();
  do
  {
    display.fillScreen(PAPER);
    // ug.print("你是苹果啊！！！");
    // 绘制边框
    display.drawLine(220, 5, 220, 75, INK);   // 时间与倒计时
    display.drawLine(5, 75, 395, 75, INK);    // 上方水平
    display.drawLine(5, 250, 395, 250, INK);  // 下方水平
    display.drawLine(150, 75, 150, 250, INK); // 待办事件右方
    // 绘制时间
    Serial.println("开始绘制时间");
    display.setFont(&DS_DIGI_150pt7b);
    display.setCursor(5, 70);
    display.setTextColor(INK);
    display.print(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10));
    // display.drawRect(5,5,210,70,INK);
    // 绘制倒计时与倒数日
    Serial.println("开始绘制倒计时");
    ug.setCursor(220 + 10, 25);
    ug.print("距离");
    ug.setCursor(220 + 10, 45);
    ug.print(day_name);
    ug.setCursor(220 + 10, 65);
    ug.print("还剩");
    // display.drawLine(265 + 10,5,265 + 10,75,INK);
    // display.drawRect(225,5,170,70,INK);
    display.setFont(&MakoP_230pt7b);
    display.setCursor(280, 60);
    display.setTextColor(INK);
    display.print(end_day);
    // 绘制日程与待办
    ug.setCursor(5, 100);
    ug.print("今日待办事项：");
    for (int i = 0; i < totle; i++)
    {
      ug.setCursor(5, 120 + 20 * i);
      ug.print(String(i + 1) + "." + Mqtt_Sub["list"][i]["no"].as<String>());
      // display.drawLine(5,122 + 20 * i,150,122 + 20 * i, INK);
      //  display.drawRect(5,80,145,165,INK);
    }
    // 绘制天气事件
    Serial.println("开始绘制天气");
    ug.setCursor(10, 270);
    ug.print("鹤壁");
    ug.setCursor(5, 290);
    ug.print("星期" + week[weekday]);
    display.drawLine(50, 255, 50, 295, INK);
    ug.setCursor(55, 270);
    ug.print("温度：" + String(Temp_in) + "/" + temp + "℃");
    ug.setCursor(55, 290);
    ug.print("湿度：" + String(Humidity_in) + "/" + humidity + "%");
    display.drawLine(150, 255, 150, 295, INK);
    ug.setCursor(155, 270);
    ug.print("日出：" + sunrise);
    ug.setCursor(155, 290);
    ug.print("日落：" + sunset);
    display.drawLine(240, 255, 240, 295, INK);
    ug.setCursor(245, 270);
    ug.print(weather0 + "/" + moon0);
    ug.setCursor(245, 290);
    ug.print(weather1 + "/" + moon1);
    display.drawLine(350, 255, 350, 295, INK);
    Serial.println("绘制天气结束");
    // 绘制电量
    // display.drawRect(360,255,30,20,INK);
    // display.fillRect(360, 275, 30, 20, INK);
    // 绘制月相图标
    switch (moonphase)
    {
    case 0:
      display.drawInvertedBitmap(353, 253, _moon0, 42, 42, INK);
      break;
    case 1:
      display.drawInvertedBitmap(353, 253, _moon1, 42, 42, INK);
      break;
    case 2:
      display.drawInvertedBitmap(353, 253, _moon2, 42, 42, INK);
      break;
    case 3:
      display.drawInvertedBitmap(353, 253, _moon3, 42, 42, INK);
      break;
    case 4:
      display.drawInvertedBitmap(353, 253, _moon4, 42, 42, INK);
      break;
    case 5:
      display.drawInvertedBitmap(353, 253, _moon5, 42, 42, INK);
      break;
    case 6:
      display.drawInvertedBitmap(353, 253, _moon6, 42, 42, INK);
      break;
    case 7:
      display.drawInvertedBitmap(353, 253, _moon7, 42, 42, INK);
      break;
    default:
      display.drawInvertedBitmap(353, 253, _moon4, 42, 42, INK);
      break;
    }
    // 绘制小时天气
    //
    // int max = i2ceeprom.read(600);
    // int min = i2ceeprom.read(601);
    // for (int i = 0; i < 23; i++)
    // {
    //   display.drawLine(175 + 10 * i, 90 + 165 * (max - i2ceeprom.read(602 + i)) / (max - min), 175 + 10 + 10 * i, 90 + 165 * (max - i2ceeprom.read(603 + i)) / (max - min), INK);
    // }
    display.drawInvertedBitmap(155, 80, _deer, 240, 149, INK);
    //
  } while (display.nextPage());
  // } else {
  //   Serial.println("进入局刷模式");
  //   display.setFont(&DS_DIGI_150pt7b);
  //   display.setPartialWindow(0, 0, 216, 72);
  //   display.firstPage();
  //   do {
  //     // 绘制时间
  //     display.setCursor(5, 70);
  //     display.setTextColor(INK);
  //     display.print(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10));
  //   } while (display.nextPage());
  // }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(5, 4);
  if (i2ceeprom.begin(0x50))
  { // you can stick the new i2c addr in here, e.g. begin(0x51);
    Serial.println("Found I2C EEPROM");
  }
  else
  {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1)
      delay(10);
  }
  Rtc.Begin(); // DS1307时间读写
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  sht3xd.begin(0x44);
  if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR)
    Serial.println("[ERROR] Cannot start periodic mode");
  RtcDateTime now = Rtc.GetDateTime();
  if ((RST_reason == 0) || (now.Minute() == 2))
  {
    // EN复位
    connect_net(); // 连接网络
    // 更新时间
    if (WiFi.status() == WL_CONNECTED)
    {
      time_update();
      /*
      获取天气，日历，倒计时，温湿度
      get_net(web_hf, 1);
      _date = Mqtt_Sub["updateTime"].as<String>().substring(0, 10);
      temp = Mqtt_Sub["now"]["temp"].as<String>();
      humidity = Mqtt_Sub["now"]["humidity"].as<String>();
      write_eeprom(_date_address, _date);
      write_eeprom(humidity_address, humidity);
      write_eeprom(temp_address, temp);
      EEPROM.commit();*/
      get_net(todolist);
      long_msg = JsonMsg.length();
      i2ceeprom.write(300, long_msg / 200);
      i2ceeprom.write(301, long_msg % 200);
      write_eeprom(302, JsonMsg);
      get_net(web_weather);
      long_msg = JsonMsg.length();
      i2ceeprom.write(0, long_msg);
      write_eeprom(1, JsonMsg);
      get_net(web_hourweather);
      deserializeJson(Mqtt_Sub, JsonMsg);
      i2ceeprom.write(600, Mqtt_Sub["max"].as<int>());
      i2ceeprom.write(601, Mqtt_Sub["min"].as<int>() - 1);
      for (int i = 0; i < 24; i++)
      {
        i2ceeprom.write(602 + i, Mqtt_Sub["hourly"][i]["no"].as<int>());
      }
    }
  }
  else
  {
    // 其他复位源
  }
  display.epd2.init(14, 13, 115200, false, 20, false); // define or replace SW_SCK, SW_MOSI
  ug.begin(display);                                   // connect u8g2 procedures to Adafruit GFX
  display.setRotation(2);                              // 屏幕方向；2.9选3；4.2选2或4

  if ((now.Minute() % 10 == 2) || RST_reason == 0)
  {
    Serial.println("进入全刷模式");
    draw_HomePage();
  }
  else
  {
    Serial.println("进入局刷模式");
    display.setFont(&DS_DIGI_150pt7b);
    display.setPartialWindow(5, 5, 210, 70);
    display.firstPage();
    do
    {
      // 绘制时间
      display.setCursor(5, 70);
      display.setTextColor(INK);
      display.print(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10));
    } while (display.nextPage());
  }
  display.powerOff();
  ESP.deepSleep((61 - now.Second()) * 1000000);
}

void loop()
{
}
