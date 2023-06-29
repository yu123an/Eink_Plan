// 库引用
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>    //图形绘制
#include <U8g2_for_Adafruit_GFX.h>

//字体文件
#include <Fonts/FreeMonoBold9pt7b.h>
#include "gb2312.c"

//颜色声明
  uint16_t PAPER = GxEPD_WHITE;
  uint16_t INK = GxEPD_BLACK;

//屏幕声明
//2.9寸
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
// 4.2寸
//GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12)); // GDEW042T2 400x300, UC8176 (IL0398)

U8G2_FOR_ADAFRUIT_GFX ug;

//绘制主屏幕
void draw_HomePage(){
ug.setFontForegroundColor(INK);
ug.setFontBackgroundcolr(PAPER);
ug.setFont(chinese_gb2312);
uint8_t textwidth = ug.getUTF8Width("String");
uint8_t texta = ug.getFontAscent();
uint8_t textd = ug.getFontDescent();
uint8_t textheight = texta - textd;
display.firstPage();
do{
  display.fillScreen(PAPER);
  ug.print("String");
}
}
void setup() {
  Serial.begin(115200);
  //加载屏幕
  display.epd2.init(14, 13, 115200, true, 20, false); // define or replace SW_SCK, SW_MOSI
  ug.begin(display); // connect u8g2 procedures to Adafruit GFX
  display.setRotation(1);//屏幕方向

}

void loop() {
  // put your main code here, to run repeatedly:

}
