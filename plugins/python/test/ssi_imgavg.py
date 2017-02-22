'''
ssi_gray.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/13
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Calculate average of every channel of an image (requires opencv and numpy).
'''

import cv2
import numpy as np


def setImageFormatIn(format, opts, vars): 

    print(format)
    vars['channels'] = format.channels


def getSampleDimensionOut(dim, opts, vars):

    return vars['channels']


def getSampleBytesOut(bytes, opts, vars):

    return 4


def getSampleTypeOut(type, types, opts, vars):

    return types.FLOAT
    

def transform(info, sin, sout, sxtra, board, opts, vars):   

    ain = np.asarray(sin)
    aout = np.reshape(np.asarray(sout),vars['channels'])

    ain.mean((0,1),out=aout)





    