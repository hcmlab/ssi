'''
ssi_gray.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/13
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Convert a rgb image to grayscale (requires opencv and numpy).
'''

import cv2
import numpy as np


def getImageFormatOut(format, opts, vars): 
    
    print(format)
    format.channels = 1
    print(format)

    return format
    

def transform(info, sin, sout, sxtra, board, opts, vars):   

    ain = np.asarray(sin)
    aout = np.asarray(sout)

    cv2.cvtColor(ain, cv2.COLOR_RGB2GRAY, aout)





    