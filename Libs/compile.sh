#!/bin/sh

cd ffmpeg

./configure --prefix=../ffmpeg-lib --enable-gpl --enable-postproc --enable-avfilter --enable-avfilter-lavf --enable-pthreads --enable-small --enable-memalign-hack
#--enable-libx264 --enable-libxvid 