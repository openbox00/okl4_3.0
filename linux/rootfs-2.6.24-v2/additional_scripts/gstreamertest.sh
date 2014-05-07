#!/bin/sh

mount -t okl4fs /dev/ram1 /okl4
mknod /okl4/intervm_test p

cat /okl4/intervm_test | wc -c > /gstreamer_output.txt

if (grep 2129980 /gstreamer_output.txt)
then echo GStreamer test result: pass
else echo GStreamer test result: fail
fi


