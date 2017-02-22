'''
ssi_ppmcc.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/02
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Calculate Pearson product-moment correlation coefficient between dimensions of a two dimensional signal.
'''


import scipy.stats
import numpy
import warnings


def getSampleDimensionOut(dim, opts, vars):

    if dim != 2:
        print('only two dimensions are supported') 

    return 2


def transform(info, sin, sout, sxtra, board, opts, vars):   

    msin = numpy.asmatrix(sin)

    warnings.filterwarnings('error')           
    try:
        p = scipy.stats.pearsonr(msin[:,0], msin[:,1])
        sout[0] = p[0]
        sout[1] = p[1]
    except:
        sout[0] = 0
        sout[1] = 0