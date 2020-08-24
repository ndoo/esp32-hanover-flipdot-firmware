#!/usr/bin/python

import numpy as np
import socket
import struct
from time import sleep

MULTICAST_GROUP = ('239.1.2.3', 8080)

WIDTH = 64
HEIGHT = 32

board_white = np.packbits(np.ones((WIDTH, HEIGHT), dtype=int)).tobytes()
board_black = np.packbits(np.zeros((WIDTH, HEIGHT), dtype=int)).tobytes()

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

while True:
  sock.sendto(board_white, MULTICAST_GROUP)
  sleep(3)
  sock.sendto(board_black, MULTICAST_GROUP)
  sleep(3)
