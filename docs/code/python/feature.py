'''
feature.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/28
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Calculates energy.
'''

import math


def getSampleDimensionOut(dim, opts, vars): # redundant
    return dim


def getSampleBytesOut(bytes, opts, vars): # redundant
    return bytes


def getSampleTypeOut(type, types, opts, vars): # redundant
    return type


def transform_enter(sin, sout, sxtra, board, opts, vars): # redundant
    pass


def transform(info, sin, sout, sxtra, board, opts, vars):   

    print('time = %d ms, frame = %d, delta = %d' % (info.time, info.frame, info.delta))


    for d in range(sin.dim):
        sout[d] = 0

    for n in range(sin.num):
        for d in range(sin.dim):
            val = sin[n,d]
            sout[d] += val*val

    for d in range(sin.dim):
        sout[d] = math.sqrt(sout[d] / sin.num)   


def transform_flush(sin, sout, sxtra, board, opts, vars):  # redundant 
    pass
