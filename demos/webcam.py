#!/usr/bin/python

from os import path

import socket
import struct
import sys
import time
import cv2

from lib import imageToBinary as i2b

MULTICAST_GROUP = ('239.1.2.3', 8080)

WIDTH = 96
HEIGHT = 32

vidcap = cv2.VideoCapture(cv2.CAP_DSHOW)

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)


while True:
  success,cv2_im = vidcap.read()
  if success: sock.sendto(i2b.imageToBinary(cv2_im, WIDTH, HEIGHT, True, True), MULTICAST_GROUP)
  time.sleep(0.1)