'''
ssi_gray.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/13
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Plot image (requires opencv and numpy).
'''

import cv2
import numpy as np


def getOptions(opts,vars):

    opts['name'] = 'image'  
    opts['x'] = 0
    opts['y'] = 0
    opts['width'] = 100
    opts['height'] = 100 


def consume_enter(sin, board, opts, vars):
    
    cv2.namedWindow(opts['name'])
    cv2.moveWindow(opts['name'],opts['x'],opts['y'])    
    cv2.resizeWindow(opts['name'],opts['width'],opts['height'])


def consume(info, sin, board, opts, vars): 

    img = np.asarray(sin[0])

    cv2.imshow(opts['name'], img)
    cv2.resizeWindow(opts['name'],opts['width'],opts['height'])
    cv2.waitKey(1)


def consume_flush(sin, board, opts, vars):
    
    cv2.destroyWindow(opts['name'])




    