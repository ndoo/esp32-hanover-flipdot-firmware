#!/usr/bin/python

from os import path

import socket
import struct
import sys
import time
import cv2
import numpy as np

from lib import imageToBinary as i2b

MULTICAST_GROUP = ('239.1.2.3', 8080)

WIDTH = 64
HEIGHT = 32

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

  sock.sendto(i2b.imageToBinary(cv2_im, WIDTH, HEIGHT, True, False), MULTICAST_GROUP)
  vidcap.set(cv2.CAP_PROP_POS_MSEC,(time.time()-start)*1000)
  success,cv2_im = vidcap.read()

  if not success and time.time()-start > 0:
    success = True
    start = time.time()
    vidcap.set(cv2.CAP_PROP_POS_MSEC,0)
    success,cv2_im = vidcap.read()

  time.sleep(0.05)
