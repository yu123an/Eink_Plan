# 关于文件夹
#### 本文件主要用来处理图片，便于在epaper进行显示
#### 我所使用的是来自于微雪的12.48寸三色墨水屏，分辨率位1304*984，尺寸较大，因此要对待显示的图片进行处理，思路如下：
#### 预处理，将图片裁剪为1280*960的大小，存入服务器；
#### 图片切割：将图片切割为256*160大小的碎片图片，便于下一步的传输
#### 格式转换，通道分离，传输
# 关于使用
#### 主要包含两部分内容，一个为`convert.sh`；对碎片图片进行格式转换等操作，另一个为`convert_big.sh`，即是对大文件进行切割，并重复调用`convert.sh`进行处理
# 关于图片处理
#### 可以参考下面的网址，给了我很大的帮助
```
https://learn.adafruit.com/preparing-graphics-for-e-ink-displays/overview
https://learn.adafruit.com/preparing-graphics-for-e-ink-displays/command-line
https://github.com/protostax/ProtoStax_ePaper_Photo_Frame/blob/4c12b26c95d131d81190f7bd03d1ed7b3653937a/processImage.sh
```
