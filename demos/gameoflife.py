#!/usr/bin/python

from os import path

import socket
import struct
import sys
import time
import seagull as sg
import cv2
import seagull.lifeforms as lf

import numpy as np

from lib import imageToBinary as i2b

MULTICAST_GROUP = ('239.1.2.3', 8080)

WIDTH = 64
HEIGHT = 32
ITERS = 900

board = sg.Board(size=(HEIGHT, WIDTH))

np.random.seed(42)
noise = np.random.choice([0,1], size=(HEIGHT,WIDTH))
custom_lf = lf.Custom(noise)
board.add(custom_lf, loc=(0,0))

sim = sg.Simulator(board)

sock = socket.socket(socket.AF_INET, # Internet
  socket.SOCK_DGRAM) # UDP
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

sim.run(sg.rules.conway_classic, iters=ITERS)

for i in range(ITERS):

  final = np.asarray(sim.get_history()[i] * 255, dtype=np.uint8)
  final = cv2.cvtColor(final, cv2.COLOR_GRAY2BGR)
  sock.sendto(i2b.imageToBinary(final, WIDTH, HEIGHT), MULTICAST_GROUP)

  time.sleep(0.05)
