
/*接线
 * ST7920/LCD12864---arduino uno
 * VCC-------------5V
 * GND-------------GND
 * PSB----------------GND
 * E------------------2 SCK
 * R/W----------------3 MOSI
 * R/S----------------4 SS/CS
 * 
 * 程序效果： lcd屏幕显示 “我在人群中看见你" "我看见你在人群中”并且闪烁
 */

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
//#ifdef U8X8_HAVE_HW_I2C
//#include <Wire.h>
//#endif

//初始化接口
U8G2_ST7920_128X64_2_SW_SPI u8g2(U8G2_R0, /* clock=*/ 2 /* A4 */ , /* data=*/ 3 /* A2 */, /* CS=*/ 4 /* A3 */, /* reset=*/ U8X8_PIN_NONE);
//注意PSB一定要接GND

unsigned long sj;
int jsq=0;

void setup(void) {
  u8g2.begin();//初始化
  u8g2.enableUTF8Print(); // 使print支持UTF8字集
  //u8g2.setFont(u8g2_font_wqy13_t_chinese5);// 字库可以在初始化里统一使用，也可以在显示函数（u8g2.firstPage();）前面定义即将显示的字体
  //u8g2.setFont(u8g2_font_ncenB14_tr);
  //u8g2.setFont(u8g2_font_unifont_t_chinese2);
  //u8g2.setFont(u8g2_font_unifont_t_symbols);
  //u8g2.setFont(u8g2_font_ncenB10_tr); 
  sj = millis();
}

void loop(void) {
  u8g2.setFont(u8g2_font_unifont_t_chinese1);  //使用这个字库里面要包含"我在人群中看见你" "我看见你在人群中"要不然不能显示，建议自己写字库这样可以节约内存。
  u8g2.setFontDirection(0);//旋转当前显示 参数:0123
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 15);//在这个位置显示下面内容
    u8g2.print("我在人群中看见你");//打印 "我在人群中看见你"
    u8g2.setCursor(0, 30);
    u8g2.print("我看见你在人群中");   // Chinese "Hello World" 
    u8g2.setCursor(56, 48);
    u8g2.print(jsq);   // Chinese "Hello World" 
  } while ( u8g2.nextPage() );

  if(millis()-sj>= 1000){
    sj = millis();
    jsq++;
  }

}
