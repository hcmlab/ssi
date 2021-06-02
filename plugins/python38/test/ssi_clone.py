'''
ssi_clone.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/02/25
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Clones the input stream to the output stream.
'''


def print_stream(s):

    print("> object type = %s" % type(s))
    print("> value type  = %s" % s.type())
    print("> shape       = %d x %d" % s.shape())    
    print("> len         = %d" % s.len) 
    print("> num         = %d" % s.num) 
    print("> sr          = %f" % s.sr) 
    print("> tot         = %d" % s.tot) 
    print("> byte        = %d" % s.byte) 


def getOptions(opts, vars):
    pass


def getSampleNumberOut(num, opts, vars):
    return num


def getSampleDimensionOut(dim, opts, vars):
    return dim


def getSampleBytesOut(bytes, opts, vars):
    return bytes


def getSampleTypeOut(type, types, opts, vars):
    return type


def transform_enter(sin, sout, sxtra, board, opts, vars):

    print('in:')
    print_stream(sin)

    print('out:')
    print_stream(sout)


def transform(info, sin, sout, sxtra, board, opts, vars):   

    print('time = %fs, dur = %fs, frame = %d, delta = %d' % (info.time, info.dur, info.frame, info.delta))

    for i in range(0,sin.len):
        sout[i] = sin[i]


def transform_flush(sin, sout, sxtra, board, opts, vars):   
    pass
