pic=$1
convert $pic -crop 256x160 /chwhsen/bitmap_to_epaper/out/out.jpg
for (( i = 0;i<30;i++ ))
do
/chwhsen/bitmap_to_epaper/convert.sh /chwhsen/bitmap_to_epaper/out/out-${i}.jpg 30 ${i}
done