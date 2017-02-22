'''
ssi_feature.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/13
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Calculates average of every channel of an image (requires numpy).
'''

import numpy


def setImageFormatIn(format, opts, vars): 

    vars['channels'] = format.channels


def getSampleDimensionOut(dim, opts, vars):

    return vars['channels']


def getSampleTypeOut(type, types, opts, vars):

    return types.FLOAT


def getSampleBytesOut(bytes, opts, vars):

    return 4
    

def transform(info, sin, sout, sxtra, board, opts, vars):   

    iin = numpy.asarray(sin)
    iout = numpy.reshape(sout, vars['channels'])

    iin.mean((0,1),out=iout)






    