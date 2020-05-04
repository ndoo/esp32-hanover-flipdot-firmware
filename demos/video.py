#!/usr/bin/python

from PIL import Image   # install with `pip install pillow`
from os import path
from time import sleep

import socket
import struct
import sys
import time
import cv2

multicast_group = ('239.1.2.3', 8080)

# IMAGE CONVERSION ---------------------------------------------------------

# Currently handles two modes of image conversion for specific projects:
# 1. Convert bitmap (2-color) image to PROGMEM array for Adafruit_GFX
#    drawBitmap() function.
# 2. Convert color or grayscale image to 5/6/5 color PROGMEM array for
#    NeoPixel animation (e.g. CircuitPlaygroundMakeBelieve project).

def imageToBinary(im):
  im = im.convert("1")
  im = im.transpose(Image.ROTATE_180)
  pixels = im.load()
  binarystr = 0

  for y in range(im.size[1]):
    for x in range(im.size[0]):
      binarystr = binarystr << 1
      if pixels[x, y] > 0:
        binarystr += 1

  return binarystr.to_bytes(256, byteorder='little')

if len(sys.argv) != 2:
  print("Usage: " + sys.argv[0] + " [video file]")
  exit(1)

vidcap = cv2.VideoCapture(sys.argv[-1])
success,cv2_im = vidcap.read()

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

count = 0
start = time.time()

while success:
  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

  cv2_im = cv2.resize(cv2_im, (64, 32), cv2.INTER_LANCZOS4)
  cv2.imshow('Output', cv2.resize(cv2_im, (cv2_im.shape[1] * 16, cv2_im.shape[0] * 16), cv2.INTER_NEAREST))

  cv2_im = cv2.cvtColor(cv2_im, cv2.COLOR_BGR2GRAY)
  cv2_im = cv2.adaptiveThreshold(cv2_im,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
            cv2.THRESH_BINARY,11,2)

  sock.sendto(imageToBinary(Image.fromarray(cv2_im)), multicast_group)

  vidcap.set(cv2.CAP_PROP_POS_MSEC,(time.time()-start)*1000)
  success,cv2_im = vidcap.read()

  if not success and time.time()-start > 0:
    success = True
    start = time.time();
    vidcap.set(cv2.CAP_PROP_POS_MSEC,0)
    success,cv2_im = vidcap.read()

  sleep(0.2)