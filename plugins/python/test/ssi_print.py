'''
ssi_print.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/02
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Output stream(s) to the console or a file.
'''


import sys


def getOptions(opts,vars):

    opts['path'] = ''  


def consume_enter(sin, board, opts, vars):
    
    vars['fp'] = open(opts['path'], 'w') if opts['path'] else sys.stdout


def consume(info, sin, board, opts, vars): 

    print('time = %fs, dur = %fs' % (info.time, info.dur))

    for s in sin:
        vars['fp'].write(str(s) + '\n')


def consume_flush(sin, board, opts, vars):
    
    if vars['fp'] is not sys.stdout:
        vars['fp'].close()