#!/usr/bin/python

from os import path

import socket
import struct
import sys
import time
import cv2

from lib import imageToBinary as i2b

MULTICAST_GROUP = ('239.1.2.3', 8080)

WIDTH = 128
HEIGHT = 32

if len(sys.argv) != 2:
  print("Usage: " + sys.argv[0] + " [image file]")
  exit(1)

cv2_im = cv2.imread(sys.argv[-1])

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)

sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
sock.sendto(i2b.imageToBinary(cv2.bitwise_not(cv2_im), WIDTH, HEIGHT, False, False), MULTICAST_GROUP)
