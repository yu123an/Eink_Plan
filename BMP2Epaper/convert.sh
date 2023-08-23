# ͼƬת  
#     ͼƬ  С  256*160 ķ Χ  
# 5 * 256 * 6 * 160 == 1280 * 960
#!/bin/bash

# *************************************************** 
#   This is a script for
#   a Digital ePaper Photo Frame using Raspberry Pi B+, Waveshare ePaper Display and ProtoStax enclosure
#   --> https://www.waveshare.com/product/modules/oleds-lcds/e-paper/2.7inch-e-paper-hat-b.htm
#   --> https://www.protostax.com/products/protostax-for-raspberry-pi-b
#
#   It uses ImageMagick to tranform a regular image file into a tri-color version
#   suitable for displaying on a 264x176 Waveshare ePaper Display, resizing the image,
#   using a limited (3-color) palette and Floyd Steinberg Dithering, and splitting up
#   the image into black and white and black and red components for the ePaper library to display
#      ɫ  
#convert xc:red xc:white xc:black +append palette.gif
#   תͼƬ
#   ȡͼ    Ϣ   ֱ  ʣ ƫ    
pic=$1
totle=$2
in=$3
convert -flip $pic  /chwhsen/bitmap_to_epaper/pic_${in}.bmp
convert /chwhsen/bitmap_to_epaper/pic_${in}.bmp  /chwhsen/bitmap_to_epaper/resized_${in}.bmp 
offset=$(xxd   -i -s 10 -l 1 /chwhsen/bitmap_to_epaper/resized_${in}.bmp | grep '^ ')
empty=$(cat /chwhsen/bitmap_to_epaper/empty.log)
image_x1=$(($(xxd   -i -s 0x12 -l 1 /chwhsen/bitmap_to_epaper/resized_${in}.bmp | grep '^ ')))
image_y1=$(($(xxd   -i -s 0x16 -l 1 /chwhsen/bitmap_to_epaper/resized_${in}.bmp | grep '^ ')))
image_x2=$(($(xxd   -i -s 0x13 -l 1 /chwhsen/bitmap_to_epaper/resized_${in}.bmp | grep '^ ')))
image_y2=$(($(xxd   -i -s 0x17 -l 1 /chwhsen/bitmap_to_epaper/resized_${in}.bmp | grep '^ ')))
image_x=`expr $image_x2 \* 256 + $image_x1`
image_y=`expr $image_y2 \* 256 + $image_y1`
# Remap the resized image into the colors of the palette using
# Floyd Steinberg dithering (default)
# Resulting image will have only 3 colors - red, white and black
#convert input.jpg -dither FloydSteinberg -define dither:diffusion-amount=85% -remap eink-3color.png BMP3:output.bmp
convert /chwhsen/bitmap_to_epaper/resized_${in}.bmp -remap /chwhsen/bitmap_to_epaper/eink-3color.bmp /chwhsen/bitmap_to_epaper/result_${in}.bmp


# Replace all the red pixels with white - this
# isolates the white and black pixels - i.e the "black"
# part of image to be rendered on the ePaper Display
convert -fill white -opaque red /chwhsen/bitmap_to_epaper/result_${in}.bmp -monochrome /chwhsen/bitmap_to_epaper/result_black_${in}.bmp 
#convert /chwhsen/bitmap_to_epaper/result_black.bmp -remap /chwhsen/bitmap_to_epaper/eink-3color.bmp BMP3:/chwhsen/bitmap_to_epaper/result_black.bmp
# Similarly, Replace all the black pixels with white - this
# isolates the white and red pixels - i.e the "red"
# part of image to be rendered on the ePaper Display
convert -fill white -opaque black /chwhsen/bitmap_to_epaper/result_${in}.bmp -monochrome /chwhsen/bitmap_to_epaper/result_red_${in}.bmp

# convert -fill black -opaque red result_red.bmp result_red_to_black.bmp
# 326 //163 
# 123 //246
#convert /chwhsen/bitmap_to_epaper/result_black.bmp -crop 326x123 /chwhsen/bitmap_to_epaper/black_cut.bmp
#convert /chwhsen/bitmap_to_epaper/result_red.bmp -crop  326x123 /chwhsen/bitmap_to_epaper/red_cut.bmp


offset=$(xxd   -i -s 10 -l 1 /chwhsen/bitmap_to_epaper/result_black_${in}.bmp | grep '^ ')
xxd -e -u -i -s ${offset} -c 16 /chwhsen/bitmap_to_epaper/result_black_${in}.bmp > /chwhsen/bitmap_to_epaper/result_black_${in}.h
xxd -e -u -i -c 16  /chwhsen/bitmap_to_epaper/result_black_${in}.bmp > /chwhsen/bitmap_to_epaper/result_blackall_${in}.h

offset=$(xxd   -i -s 10 -l 1 /chwhsen/bitmap_to_epaper/result_red_${in}.bmp | grep '^ ')
xxd -e -u -i -s ${offset} -c 16 /chwhsen/bitmap_to_epaper/result_red_${in}.bmp > /chwhsen/bitmap_to_epaper/result_red_${in}.h

xxd -e -u -ps  -s ${offset}  /chwhsen/bitmap_to_epaper/result_black_${in}.bmp | sed  ':a;N;$!ba;s/\n//g' > /var/www/html/black_img_${in}.log
xxd -e -u -ps  -s ${offset}  /chwhsen/bitmap_to_epaper/result_red_${in}.bmp | sed  ':a;N;$!ba;s/\n//g' > /var/www/html/red_img_${in}.log
echo "{\"name\":\"pic\",\"xl\":\"$image_x\",\"yl\":\"$image_y\",\"totle\":\"$totle\",\"in\":\"$in\"}" > /var/www/html/img_${in}.msg
if [ $(cat /var/www/html/black_img_${in}.log) == $empty ]
then 
echo -n "empty" > /var/www/html/black_img_${in}.log
fi
if [ $(cat /var/www/html/red_img_${in}.log) == $empty ]
then 
echo -n "empty" > /var/www/html/red_img_${in}.log
fi

