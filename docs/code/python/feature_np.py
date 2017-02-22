'''
feature_np.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/28
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Calculates energy.
'''


import numpy


def transform(info, sin, sout, sxtra, board, opts, vars):

    npin = numpy.asmatrix(sin)
    npout = numpy.asmatrix(sout)

    numpy.sum(npin, axis=0, out=npout)
    numpy.square(npout, out=npout)
    numpy.sqrt(npout, out=npout)