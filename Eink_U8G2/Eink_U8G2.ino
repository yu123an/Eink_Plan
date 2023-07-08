// 库引用
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>  //图形绘制
#include <U8g2_for_Adafruit_GFX.h>

//字体文件
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/DS_DIGI_150pt7b.h>
#include <Fonts/MakoP_230pt7b.h>
#include "gb2312.c"

//颜色声明
uint16_t PAPER = GxEPD_WHITE;
uint16_t INK = GxEPD_BLACK;

//屏幕声明
//2.9寸
// GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
// 4.2寸
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12));  // GDEW042T2 400x300, UC8176 (IL0398)

U8G2_FOR_ADAFRUIT_GFX ug;

//绘制主屏幕
void draw_HomePage() {
  ug.setForegroundColor(INK);
  ug.setBackgroundColor(PAPER);
  ug.setFont(chinese_gb2312);
  // ug.setFont(u8g2_font_helvR14_tf);
  // uint16_t textwidth = ug.getUTF8Width("String");
  // uint16_t texta = ug.getFontAscent();
  // uint16_t textd = ug.getFontDescent();
  // uint16_t textheight = texta - textd;
  //局刷
  // display.setPartialWindow();
  display.firstPage();
  do {
    display.fillScreen(PAPER);
    // ug.print("你是苹果啊！！！");
    // 绘制边框
    display.drawLine(220, 5, 220, 75, INK);//时间与倒计时
    display.drawLine(5, 75, 395, 75, INK);//上方水平
    display.drawLine(5,250,395,250,INK);//下方水平
    display.drawLine(150, 75, 150, 250, INK);//待办事件右方
    //绘制时间
    display.setFont(&DS_DIGI_150pt7b);
    display.setCursor(5, 70);
    display.setTextColor(INK);
    display.print("23:46");
    // display.drawRect(5,5,210,70,INK);
    // 绘制倒计时与倒数日
    ug.setCursor(220 + 10, 25);
    ug.print("距离");
    ug.setCursor(220 + 10, 45);
    ug.print("小高考");
    ug.setCursor(220 + 10, 65);
    ug.print("还剩");
    // display.drawLine(265 + 10,5,265 + 10,75,INK);
    // display.drawRect(225,5,170,70,INK);
    display.setFont(&MakoP_230pt7b);
    display.setCursor(280, 60);
    display.setTextColor(INK);
    display.print("023");
    //绘制日程与待办
    ug.setCursor(5, 100);
    ug.print("今日待办事项：");
    for(int i = 0;i<7;i++){
      ug.setCursor(5,120 + 20 * i);
      ug.print("这是第" + String(i) + "件待办事件；");
     // display.drawLine(5,122 + 20 * i,150,122 + 20 * i, INK);
    //  display.drawRect(5,80,145,165,INK);
    }
    //绘制天气事件
    ug.setCursor(10, 270);
    ug.print("鹤壁");
    ug.setCursor(5, 290);
    ug.print("星期五");
    display.drawLine(50,255,50,295,INK);
    ug.setCursor(55, 270);
    ug.print("温度：13/24℃");
    ug.setCursor(55, 290);
    ug.print("湿度：56/55%");
    display.drawLine(150,255,150,295,INK);
    ug.setCursor(155, 270);
    ug.print("日出：13：24");
    ug.setCursor(155, 290);
    ug.print("日落：17：09");
    display.drawLine(240,255,240,295,INK);
    ug.setCursor(245, 270);
    ug.print("晴间多云/峨眉月");
    ug.setCursor(245, 290);
    ug.print("大雨转小雨/满月");
    display.drawLine(355,255,355,295,INK);
  } while (display.nextPage());
}
void setup() {
  Serial.begin(115200);
  //加载屏幕
  display.epd2.init(14, 13, 115200, true, 20, false);  // define or replace SW_SCK, SW_MOSI
  ug.begin(display);                                   // connect u8g2 procedures to Adafruit GFX
  display.setRotation(2);                              //屏幕方向；2.9选3；4.2选2或4
  draw_HomePage();
}

void loop() {
  // // put your main code here, to run repeatedly:
  // display.setFont(&DS_DIGI_150pt7b);
  // for (int i = 0; i < 20; i++) {
  //   display.setPartialWindow(5,5,210,70);
  //   display.firstPage();
  //   do {
  //     display.setCursor(5,70);
  //     display.setTextColor(INK);
  //     display.print("23:" + String(i / 10) + String(i % 10));
  //   } while (display.nextPage());
  //   delay(6000);
  // }
  delay(40000);
  Serial.println("aaaaaaaaa");
}
