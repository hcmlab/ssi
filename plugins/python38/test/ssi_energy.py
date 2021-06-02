'''
ssi_energy.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/02
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Returns energy of a signal (dimensionwise or overall)
'''

import math


def getOptions(opts,vars):

    opts['global'] = False


def getSampleDimensionOut(dim, opts, vars):

    return 1 if opts['global'] else dim


def getSampleTypeOut(type, types, opts, vars): 

    if type != types.FLOAT and type != types.DOUBLE:  
        print('types other than float and double are not supported') 
        return types.UNDEF

    return type


def transform(info, sin, sout, sxtra, board, opts, vars): 

    if opts['global']:
        
        sout[0] = 0

        for val in sin:            
            sout[0] += val*val

        sout[0] = math.sqrt(sout[0] / sin.len)  

    else:

        for d in range(0,sin.dim):
            sout[d] = 0

        for n in range(0,sin.num):
            for d in range(0,sin.dim):
                val = sin[n,d]
                sout[d] += val*val

        for d in range(0,sin.dim):
            sout[d] = math.sqrt(sout[d] / sin.num)    

