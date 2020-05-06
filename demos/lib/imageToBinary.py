#!/usr/bin/python

import cv2
import numpy as np

# Convert image to monochrome bytes

def imageToBinary(cv2_im, width: int, height: int, adaptive: bool = False, debug_imshow: bool = False):
  
  if debug_imshow: cv2.imshow('Input', cv2_im)
  cv2.waitKey(1)

  cv2_im = cv2.resize(cv2_im, (width, height), cv2.INTER_NEAREST)
  cv2_im = cv2.cvtColor(cv2_im, cv2.COLOR_BGR2GRAY)
  if adaptive:
    cv2_im = cv2.adaptiveThreshold(cv2_im,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
            cv2.THRESH_BINARY,11,2)
  else:
    (ret, cv2_im) = cv2.threshold(cv2_im,127,255,cv2.THRESH_BINARY)

  cv2_im = cv2.bitwise_not(cv2_im)
  
  if debug_imshow: cv2.imshow('Output (8x scale)', cv2.resize(cv2_im, (width*8, height*8), cv2.INTER_NEAREST))
  cv2.waitKey(1)

  cv2_im[cv2_im >= 1] = 1

  bytes = np.packbits(cv2_im, bitorder='little').tobytes()

  return bytes
