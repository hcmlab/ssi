'''
ssi_clone.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/02/25
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Rotates mnist digits.
'''

def getOptions(opts,vars):

    opts['width'] = 28
    opts['height'] = 28


def transform(info, sin, sout, sxtra, board, opts, vars):   

    width = opts['width']
    height = opts['height']

    for n in range(0, sin.num):
        for w in range(0, width):            
            for h in range(0, height):            
                sout[n,h*width+w] = sin[w*height+h]

