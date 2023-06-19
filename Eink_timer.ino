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
// 引入需要的库
#include <epd2in13.h>  //屏幕驱动
#include <TFT_eSPI.h>
#include "font.h"
#include <RtcDS1307.h>
#include <Wire.h>  //DS1307驱动
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>  //网络校时
#include <WiFiUdp.h>    //WiFi支持
#include "msg.h"
#include <ArduinoJson.h>       //JSON解析
#include <WiFiClientSecure.h>  //https请求
#include <EEPROM.h>           //EEPROM读写
//墨水屏实例
Epd ePaper;
//字体列表
#define Digi &DS_DIGI32pt7b          // 数码管字体
#define DejaVu &DejaVu_Sans_Mono_20  // 等宽字体
#define DejaVu_10 &DejaVuSansMono_110pt7b
#define DejaVu_12 &DejaVuSansMono_112pt7b
#define DejaVu_15 &DejaVuSansMono_115pt7b
#define Orbitron_32 &Orbitron_Light_32
#define PAPL &PAPL_125pt7b  // 等宽数字
#define Cour &cour_212pt7b
#define PAPL_10 &PAPL_110pt7b  // 等宽数字
#define PAPL_12 &PAPL_112pt7b  // 等宽数字
#define PAPL_15 &PAPL_115pt7b  // 等宽数字
// 变量声明
#define INK COLORED  //颜色
#define PAPER UNCOLORED
#define EPD_BUFFER 1             // Label for the black frame buffer 1
uint16_t epd_width = EPD_WIDTH;  //墨水屏长宽
uint16_t epd_height = EPD_HEIGHT;
uint8_t* framePtr = NULL;  // Pointer for the black frame buffer
int8_t limit = 5;
int FullUpdateNum = 0;
static struct rst_info *rinfo = ESP.getResetInfoPtr();//休眠唤醒信息
int RST_reason = rinfo->reason;
#include "EPD_Support.h"  //墨水屏需要
//显示变量
String temp, hump, windDir, wind, _weather, _date;  // 天气状况
int Temp_in, Humidity_in;                           // 室内温湿度
int _day, _hour, _minute, _second;                  // 时间更新
int update_flag = 0;                                // 更新标志位
float v_bat = 0;                                    // 电量检测
String JsonMsg;                                     // json串解析
#define MSG_BUFFER_SIZE (2000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<2000> Mqtt_Sub;
unsigned long lastMsg = 0;
//EEPROM存储地址
int _date_address = 1;
int _date_long = 10;
int class_address = 11;
int class_long = 18;
int temp_address = 29;
int temp_long = 2;
int humidity_address = 31;
int humidity_long = 2;

//坐标布局
int time_x = 3;
int time_y = 33;
int time_lx = 140;
int time_ly = 50;
int classup_x = 5;
int classup_y = 3;
int classup_lx = 240;
int classup_ly = 17;
int classdown_x = 5;
int classdown_y = 20;
int date_x = 5;
int date_y = 86;
int temp_x = 150;
int temp_y = 86;
int temp_lx = 140;
int temp_ly = 17;
int humidity_x = 150;
int humidity_y = 106;
int humidity_lx = 140;
int humidity_ly = 17;
//对象实例化
TFT_eSPI glc = TFT_eSPI();
TFT_eSprite Eink = TFT_eSprite(&glc);
RtcDS1307<TwoWire> Rtc(Wire);
WiFiUDP ntpUDP;
WiFiClient espClient;  // WiFi
NTPClient timeClient(ntpUDP, "ntp.ntsc.ac.cn", 8 * 3600, 60000);
//功能函数
// 更新时间
void time_update() {
  timeClient.begin();
  timeClient.update();
  _day = timeClient.getDay();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  _second = timeClient.getSeconds();
  if (_minute != 0) {
    Serial.println("成功获取时间，准备更新时间。。。。");
    Rtc._SetDateTime(_second, _minute, _hour);
  }
}
void get_net(String web, bool isdecode) {
  HTTPClient http;
  if (http.begin(espClient, web)) {
    Serial.println("HTTPclient setUp done!");
  }
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      if (isdecode) {
        deserializeJson(Mqtt_Sub, payload);
      } else {
        JsonMsg = payload;
      }
      Serial.println(payload);
    }
  } else {

    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
// http字符串解析
void JsonEecoed(String json) {
  deserializeJson(Mqtt_Sub, json);
}
// 连接互联网
void connect_net() {
  WiFi.begin(ssidhome, passhome);  // 连接网络
  int i = 0;
  Serial.print("Connecting to ");
  Serial.print(ssidhome);
  while (WiFi.status() != WL_CONNECTED) {  // 等待连接
    delay(500);
    Serial.print('.');
    if (i > 40) {
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
void drawHomePage(){
    Rtc.Begin();  // DS1307时间读写
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  // 绘制顶部时间栏
  RtcDateTime now = Rtc.GetDateTime();
  _date = read_eeprom(_date_address,_date_long);
  temp = read_eeprom(temp_address,temp_long);
  if((now.Minute() % 10 == 0) || RST_reason == 0){
    if (ePaper.Init(lut_full_update) != 0) {
    Serial.print("ePaper init failed");
    while (1) yield();  // Wait here until re-boot
  }
  Serial.println("\r\n ePaper initialisation OK");
  }else{
    if (ePaper.Init(lut_partial_update) != 0) {
      Serial.print("ePaper init failed");
      while (1) yield();  // Wait here until re-boot
      Serial.println("\r\n ePaper initialisation OK");
    }
  }
  // 屏幕检测
  Eink.setColorDepth(1);
  framePtr = (uint8_t*)Eink.createSprite(EPD_WIDTH, EPD_HEIGHT);
  Serial.println("\r\nInitialisation done.");
  Eink.setRotation(3);
  Eink.fillSprite(PAPER);
  Eink.setFreeFont(Digi);
  Eink.setTextColor(INK);
  Eink.setTextDatum(0);
  Eink.drawString(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10) , time_x, time_y);
  //Eink.drawRect(time_x, time_y, time_lx, time_ly, INK);
  Eink.setFreeFont(DejaVu_10);
  Eink.fillRect(classdown_x,classdown_y,classup_lx,classup_ly, PAPER);
  Eink.drawString("21 22 23 31 32 33 Ll",classdown_x,classdown_y);
  Eink.drawString("T:" + temp +"℃",temp_x,temp_y);
  Eink.drawString("H:83/99%",humidity_x,humidity_y);
  Eink.drawString(_date,date_x + 8,date_y + 10);
  Eink.setTextColor(PAPER);
  Eink.fillRect(classup_x,classup_y,classup_lx,classup_ly, INK);
  Eink.drawString("A1 A2 B2 B3 C1 C2 C3",classup_x,classup_y);
  //Eink.drawRect(temp_x, temp_y, temp_lx, temp_ly, INK);
  Eink.drawLine(0, temp_y - 2, 250, temp_y - 2, INK);
  Eink.drawLine(0, classdown_y + classup_ly, 250, classdown_y + classup_ly, INK);
  Eink.drawLine(time_x + time_lx, classdown_y + classup_ly,time_x + time_lx, temp_y - 2,INK);
  Eink.setFreeFont(Orbitron_32);
  Eink.setTextColor(INK);
  Eink.setTextDatum(4);
  Eink.drawString("136",(250 + time_x + time_lx)/2,time_y + time_ly / 2);
  // Eink.loadFont(siyuan_20);
  // Eink.setCursor(40,80);
  // Eink.print("苹果");
  updateDisplay();
}
// 读写EEPROM
void write_eeprom(int addr,String velue){
  int lenth = velue.length();
  for(int a = 0;a<lenth;a++){
    EEPROM.write(addr + a,velue[a]);
    //delay(50);
  }
  EEPROM.commit();
}
String read_eeprom(int addr,int lenth){
  String Text;
  for(int a = 0;a<lenth;a++){
   Text +=  char(EEPROM.read(addr + a));
   //delay(50);
   
  }
  return Text ;
  Serial.print(Text);
}
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  Wire.begin(5,4);
  Rtc.Begin();  // DS1307时间读写
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
   if ((RST_reason == 0) || (now.Minute() == 0 ))
  {
    //EN复位
  connect_net();//连接网络
  // 更新时间
  if (WiFi.status() == WL_CONNECTED) {
    time_update();
  }
  //获取天气，日历，倒计时，温湿度
  get_net(web_hf,1);
  _date = Mqtt_Sub["updateTime"].as<String>().substring(0,10);
  write_eeprom(_date_address,_date);
  temp = Mqtt_Sub["now"]["temp"].as<String>();
  write_eeprom(temp_address,temp);
  }else{
    //其他复位源
  }
drawHomePage();
 ESP.deepSleep((61 - now.Second()) * 1000000);
}

void loop() {
 
}
