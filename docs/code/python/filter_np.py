'''
filter.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/04/28
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sums up dimensions.
'''


import numpy


def getSampleDimensionOut(dim, opts, vars):

	return 1


def transform(info, sin, sout, sxtra, board, opts, vars):

	npin = numpy.asmatrix(sin)
	npout = numpy.asmatrix(sout)

	numpy.sum(npin, axis=1, out=npout)