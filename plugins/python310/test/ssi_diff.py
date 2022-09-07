'''
ssi_diff.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/02
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Returns the 1st derivative for each dimension of a signal.
'''


def transform_enter(sin, sout, sxtra, board, opts, vars):
    
    vars['hist'] = [0,] * sout.dim
    vars['first'] = True


def transform(info, sin, sout, sxtra, board, opts, vars): 

    hist = vars['hist']

    if vars['first']:
        for d in range(0,sin.dim):
            hist[d] = sin[0,d]
        vars['first'] = False
        
    for n in range(0,sin.num):
        for d in range(0,sout.dim):
            val = sin[n,d]
            sout[n,d] = val - hist[d]
            hist[d] = val