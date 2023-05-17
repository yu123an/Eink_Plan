
/*
墨水屏驱动下的时钟与日历小摆件
*/
// 引入需要的库
#include <epd2in13.h>  //屏幕驱动
#include <TFT_eSPI.h>
Epd ePaper;
//字体列表
#define Digi &DS_DIGI32pt7b          // 数码管字体
#define DejaVu &DejaVu_Sans_Mono_20  // 等宽字体
#define DejaVu_10 &DejaVuSansMono_110pt7b
#define DejaVu_12 &DejaVuSansMono_112pt7b
#define DejaVu_15 &DejaVuSansMono_115pt7b
#define Orbitron_32 &Orbitron_Light_32
#define PAPL &PAPL_125pt7b           // 等宽数字
#define Cour &cour_212pt7b
#define PAPL_10 &PAPL_110pt7b           // 等宽数字
#define PAPL_12 &PAPL_112pt7b           // 等宽数字
#define PAPL_15 &PAPL_115pt7b           // 等宽数字
// 变量声明
#define INK COLORED  //颜色
#define PAPER UNCOLORED
#define EPD_BUFFER 1             // Label for the black frame buffer 1
uint16_t epd_width = EPD_WIDTH;  //墨水屏长宽
uint16_t epd_height = EPD_HEIGHT;
uint8_t* framePtr = NULL;  // Pointer for the black frame buffer
int8_t limit = 5;
int FullUpdateNum = 0;
#include "EPD_Support.h"  //墨水屏需要
//显示变量
String temp, hump, windDir, wind, _weather, _date;  // 天气状况
int Temp_in, Humidity_in;                           // 室内温湿度
int _day, _hour, _minute, _second;                  // 时间更新
int update_flag = 0;                                // 更新标志位
float v_bat = 0;                                    // 电量检测
String JsonMsg;                                     // json串解析
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
// int countdown_x = ;
// int countdown_y = ;
// int countdown_lx = ;
// int countdown_ly = ;
// int weather_x = ;
// int weather_y = ;
// int weather_lx = ;
// int weather_ly = ;

//对象实例化
TFT_eSPI glc = TFT_eSPI();
TFT_eSprite Eink = TFT_eSprite(&glc);
void setup() {
  Serial.begin(115200);
  SPI.pins(14,-1,13,-1);
  SPI.begin();
  //全刷
  if (ePaper.Init(lut_full_update) != 0) {
    Serial.print("ePaper init failed");
    while (1) yield();  // Wait here until re-boot
  }
  Serial.println("\r\n ePaper initialisation OK");
  // 屏幕检测
  Eink.setColorDepth(1);
  framePtr = (uint8_t*)Eink.createSprite(EPD_WIDTH, EPD_HEIGHT);
  Serial.println("\r\nInitialisation done.");
  Eink.setRotation(3);
  Eink.fillSprite(PAPER);
  Eink.setFreeFont(Digi);
  Eink.setTextColor(INK);
  Eink.drawString("23:47" , time_x, time_y);
  //Eink.drawRect(time_x, time_y, time_lx, time_ly, INK);
  Eink.setFreeFont(DejaVu_10);
  Eink.fillRect(classdown_x,classdown_y,classup_lx,classup_ly, PAPER);
  Eink.drawString("21 22 23 31 32 33 Ll",classdown_x,classdown_y);
  Eink.drawString("T:13/26C",temp_x,temp_y);
  Eink.drawString("H:83/99%",humidity_x,humidity_y);
  Eink.drawString("2023/05/17",date_x + 8,date_y + 10);
  Eink.setTextColor(PAPER);
  Eink.fillRect(classup_x,classup_y,classup_lx,classup_ly, INK);
  Eink.drawString("A1 A2 B2 B3 C1 C2 C3",classup_x,classup_y);
  //Eink.drawRect(temp_x, temp_y, temp_lx, temp_ly, INK);
  Eink.drawLine(0, temp_y - 2, 250, temp_y - 2, INK);
  Eink.drawLine(0, classdown_y + classup_ly, 250, classdown_y + classup_ly, INK);
  Eink.drawLine(time_x + time_lx, classdown_y + classup_ly,time_x + time_lx, temp_y - 2,INK);
  Eink.setFreeFont(Orbitron_32);
  Eink.setTextColor(INK);
  Eink.drawString("136",time_x + time_lx + 10,classdown_y + classup_ly);
  updateDisplay();
}

void loop() {
  delay(4000);
  Serial.println("2222222222");
}
