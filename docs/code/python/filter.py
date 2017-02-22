'''
filter.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/28
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sums up dimensions.
'''


def getSampleDimensionOut(dim, opts, vars):
    return 1


def getSampleBytesOut(bytes, opts, vars): # redundant
    return bytes


def getSampleTypeOut(type, types, opts, vars): # redundant
    return type


def transform_enter(sin, sout, sxtra, board, opts, vars): # redundant
    pass


def transform(info, sin, sout, sxtra, board, opts, vars):   

    print('time = %d ms, frame = %d, delta = %d' % (info.time, info.frame, info.delta))
    
    for n in range(sin.num):
        sout[n] = 0
        for d in range(sin.dim):
            sout[n] += sin[n,d]


def transform_flush(sin, sout, sxtra, board, opts, vars):  # redundant 
    pass
