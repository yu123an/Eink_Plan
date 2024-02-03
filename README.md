# Eink_Plan
# 电子墨水屏计划
### 关于引脚使用
在edpif.h文件中有注明
通过`SPI.pins(CLK,MISO,MOSI,SS)`调整SPI引脚
```
#if defined(ESP32)
#define RST_PIN         21
#define DC_PIN          14
#define CS_PIN          8
#define BUSY_PIN        47
// ESP8266
#else
#define RST_PIN         2
#define DC_PIN          0
#define CS_PIN          15
#define BUSY_PIN        12
//#define BUSY_PIN        4
#endif
```
### 关于局刷和全刷
在edp2in13文件中有说注明
```
const unsigned char lut_full_update[] =
{// 原来的
  // 0x66, 0x66, 0x26, 0x04, 0x55, 0xaa, 0x08, 0x91, 0x11,
  // 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x19,
  // 0x0a, 0x0a, 0x5e, 0x1e, 0x1e, 0x0a, 0x39, 0x14, 0x00,
  // 0x00, 0x00, 0x02
    0x66, 0x66, 0x26, 0x04, 0x55, 0xaa, 0x08, 0x91, 0x11, 0x88,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x19, 0x19, 0x0a, 0x0a, 0x5e, 0x1e, 0x1e, 0x0a, 0x39, 0x14,
  0x00, 0x00, 0x00,
};

const unsigned char lut_partial_update[] =
{
  // 原来的
  // 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    0x18, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00

};
```
### 关于U8G2库和GxEPD库的使用和声明
以下是我使用的两种尺寸屏幕，分别是2.9寸和4.2寸，引脚定义在下面；
调整SPI引脚
在
```
C:\Users\aa\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2\variants\generic\common.h
```
在屏幕初始化中，true要改为false，不然在深度休眠中局刷会出现问题
 ```
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 12)); // GDEW042T2 400x300, UC8176 (IL0398)
// display.epd2.init(14, 13, 115200, true, 20, false); // define or replace SW_SCK, SW_MOSI
display.epd2.init(14, 13, 115200, false, 20, false); // define or replace SW_SCK, SW_MOSI 
```
## 绘制图片
通过PCtoLCD进行取模

![取模参数截图](https://github.com/yu123an/Eink_Plan/blob/main/image/%E5%8F%96%E6%A8%A1%E5%8F%82%E6%95%B022.png)

绘制函数为
```
display.drawInvertedBitmap(155,80,logo,128,128,INK);
```
## U8G2库中4.2寸的LUT要做调整
以下是来自TFT-espi提供的LUT表
```
//复制过来的
const unsigned char GxEPD2_420::lut_20_vcom0_full[] PROGMEM =
{
0x00, 0x17, 0x00, 0x00, 0x00, 0x02,        
0x00, 0x17, 0x17, 0x00, 0x00, 0x02,        
0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,        
0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};
const unsigned char GxEPD2_420::lut_21_ww_full[] PROGMEM ={
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};
const unsigned char GxEPD2_420::lut_22_bw_full[] PROGMEM ={
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      
};

const unsigned char GxEPD2_420::lut_24_bb_full[] PROGMEM ={
0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
             
};

const unsigned char GxEPD2_420::lut_23_wb_full[] PROGMEM ={
0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            
};
```

## 三色墨水屏色阶的实现
```
void draw_3_shadow(int x, int y, int xl, int yl, int color)
{
   for (int ix = x; ix < x + xl; ix++)
  {
    for (int iy = y; iy < y + yl; iy++)
    {
      display.drawPixel(ix, iy, (ix % 2 == 0) &&(iy % 2 == 0 ) ? color : PAPER);
    }
  }
}
void draw_2_shadow(int x, int y, int xl, int yl, int color)
{
   for (int ix = x; ix < x + xl; ix++)
  {
    for (int iy = y; iy < y + yl; iy++)
    {
      display.drawPixel(ix, iy, ((ix % 2 == 0) &&(iy % 2 != 0 ) ||(ix % 2 != 0) &&(iy % 2 == 0 )) ? color : PAPER);
    }
  }
}
void draw_1_shadow(int x, int y, int xl, int yl, int color)
{
   for (int ix = x; ix < x + xl; ix++)
  {
    for (int iy = y; iy < y + yl; iy++)
    {
      display.drawPixel(ix, iy, (ix % 2 == 0) || (iy % 2 == 0 ) ? color : PAPER);
    }
  }
}
```
![色阶显示](https://github.com/yu123an/Eink_Plan/blob/main/image/20240204_001936.jpg)https://github.com/yu123an/Eink_Plan/blob/main/image/20240204_001936.jpg)
