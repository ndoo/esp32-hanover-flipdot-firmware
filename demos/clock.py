#!/usr/bin/python

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from datetime import datetime

from os import path
from time import sleep
import numpy

import socket
import struct
import sys
import time

import cv2

import lib.imageToBinary as i2b

MULTICAST_GROUP = ('239.1.2.3', 8080)
WIDTH = 64
HEIGHT = 32
WHITE = 1
BLACK = 0

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

image = Image.new('1', (WIDTH, HEIGHT), BLACK)
draw = ImageDraw.Draw(image)
clock_font = ImageFont.truetype('fonts/KHARON4A.ttf', 8)
clock_font_small = ImageFont.truetype('fonts/fixed_01.ttf', 8)
clock_font_large = ImageFont.truetype('fonts/KHARB___.ttf', 8)

previous_second = 0

while True:
  while True:
    now = datetime.today()
    if now.second != previous_second:
      break
    time.sleep(0.1)

  t = time.strftime('%H:%M:%S')
  draw.rectangle((0, 0, WIDTH, HEIGHT), fill=BLACK, outline=BLACK)
  draw.text((0, -3), now.strftime('%A'), fill=WHITE, font=clock_font_large)
  draw.text((2, 10), '{:02d}:{:02d}:{:02d}'.format(now.hour, now.minute, now.second), fill=WHITE, font=clock_font)
  draw.text((3, 24), '{}-{:02d}-{:02d}'.format(now.year, now.day, now.month), fill=WHITE, font=clock_font_small)
  cv2_im = numpy.array(image.convert('RGB'))
  cv2_im = cv2_im[:, :, ::-1].copy() 
  sock.sendto(i2b.imageToBinary(cv2_im, 64, 32, False), MULTICAST_GROUP)
