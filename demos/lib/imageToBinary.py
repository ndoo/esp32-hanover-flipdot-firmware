#!/usr/bin/python

import cv2
import numpy as np

# Convert image to monochrome bytes

def imageToBinary(cv2_im, width: int, height: int):
  
  cv2.imshow('Input', cv2_im)
  cv2.waitKey(1)

  cv2_im = cv2.resize(cv2_im, (width, height), cv2.INTER_NEAREST)
  cv2_im = cv2.cvtColor(cv2_im, cv2.COLOR_BGR2GRAY)
  cv2_im = cv2.adaptiveThreshold(cv2_im,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
            cv2.THRESH_BINARY,11,2)

  cv2.imshow('Output (8x scale)', cv2.resize(cv2_im, (width*8, height*8), cv2.INTER_NEAREST))
  cv2.waitKey(1)

  cv2_im[cv2_im >= 1] = 1

  bytes = np.packbits(cv2_im, bitorder='little').tobytes()

  return bytes
