#!/bin/sh

#cd ffmpeg

./configure --prefix=../ffmpeg-lib --enable-shared --enable-gpl --enable-postproc --enable-avfilter --enable-avfilter-lavf --enable-w32threads --enable-small --enable-memalign-hack
#--enable-libx264 --enable-libxvid 