'''
consumer.py
author: Johannes Wagner <wagner@hcm-lab.de>

Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Output stream(s) to the console or a file.
'''


import sys


def getOptions(opts, vars):

    opts['path'] = ''  


def consume_enter(sins, board, opts, vars):
    
    vars['fp'] = open(opts['path'], 'w') if opts['path'] else sys.stdout


def consume(info, sins, board, opts, vars): 

    print('time = %f s, dur = %f s' % (info.time, info.dur))

    for s in sins:
        vars['fp'].write(str(s) + '\n')


def consume_flush(sins, board, opts, vars):
    
    if vars['fp'] is not sys.stdout:
        vars['fp'].close()