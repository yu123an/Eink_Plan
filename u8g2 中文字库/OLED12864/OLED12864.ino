

#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, /* clock=*/ A1, /* data=*/ A0, /* reset=*/ U8X8_PIN_NONE);   // clock=*/ SCL, /* data=*/ SDA  接口定义



void setup() {
  u8g2.begin();//构造U8G2初始化
  u8g2.enableUTF8Print();//启用 UTF8打印
 
}

void loop() {
    u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x12_t_symbols);  // 更新的字库--选择字库
    u8g2.setCursor(0, 40);//确定坐标（文字左下角的x（128）坐标，左下的y（64）坐标）
    u8g2.print("卡号错误");
    u8g2.setFont(u8g2_font_cursor_tr);  // 更新的字库--选择字库
    u8g2.setCursor(5, 60);
    u8g2.print(0);     
    u8g2.setCursor(110, 60);
    u8g2.print(0); 
  } while ( u8g2.nextPage() );

}
