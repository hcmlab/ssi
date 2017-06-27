'''
ssi_filter.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/13
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Converts a rgb image to grayscale (requires opencv and numpy).
'''

import cv2
import numpy


def getImageFormatOut(format, opts, vars): 
    
    format.channels = 1

    return format
    

def transform(info, sin, sout, sxtras, board, opts, vars):   

    img_in = numpy.asarray(sin)
    img_out = numpy.asarray(sout)

    cv2.cvtColor(img_in, cv2.COLOR_RGB2GRAY, img_out)





    