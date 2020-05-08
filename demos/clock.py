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
WIDTH = 128
HEIGHT = 32
WHITE = 1
BLACK = 0

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

image = Image.new('1', (WIDTH, HEIGHT))
draw = ImageDraw.Draw(image)
clock_font = ImageFont.truetype('fonts/PixelOperator8.ttf', 8)
clock_font_large = ImageFont.truetype('fonts/EuropeanTeletextNuevo.ttf', 16)

def suffix(d):
    return 'th' if 11<=d<=13 else {1:'st',2:'nd',3:'rd'}.get(d%10, 'th')

def custom_strftime(format, t):
    return t.strftime(format).replace('{S}', str(t.day) + suffix(t.day))

previous_second = -1

while True:
  time.sleep(datetime.today().microsecond / 1000000)
  now = datetime.today()

  draw.rectangle((0, 0, WIDTH, HEIGHT), fill=WHITE, outline=WHITE)

  text = now.strftime('%H:%M')
  w, h = draw.textsize(text, clock_font_large)
  draw.text(((WIDTH - w) / 2, 4), text, fill=BLACK, font=clock_font_large)

  text = custom_strftime('%A, {S} %b', now)
  w, h = draw.textsize(text, clock_font)
  draw.text(((WIDTH - w) / 2, 20), text, fill=BLACK, font=clock_font)

  cv2_im = numpy.array(image.convert('RGB'))
  cv2_im = cv2_im[:, :, ::-1].copy() 
  sock.sendto(i2b.imageToBinary(cv2_im, WIDTH, HEIGHT, False), MULTICAST_GROUP)
