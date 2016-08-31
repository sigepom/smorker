#!/bin/sh
  
PORT="8080" #ポート番号
SIZE="320x240" #画面サイズ
FRAMERATE="2" #フレームレート
export LD_LIBRARY_PATH=/usr/local/lib
/home/pi/mjpg-streamer/mjpg_streamer \
    -i "input_uvc.so -f $FRAMERATE -r $SIZE -d /dev/video0 -n" \
    -o "output_http.so -w /usr/local/www -p $PORT"

