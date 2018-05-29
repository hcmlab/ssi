'''
feature_np.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/28
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Calculates energy.
'''


import numpy


def transform(info, sin, sout, sxtras, board, opts, vars):

    npin = numpy.asmatrix(sin)
    npout = numpy.asmatrix(sout)

    numpy.sum(numpy.square(npin), axis=0, out=npout)	    
    numpy.divide(npout, numpy.tile(sin.num, (1, sin.dim)), out=npout)
    numpy.sqrt(npout, out=npout)