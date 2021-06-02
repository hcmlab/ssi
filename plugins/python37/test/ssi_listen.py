'''
ssi_event.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/08
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''


def listen_enter(opts, vars):
    pass


def update(event, board, opts, vars):    
    print('time    = %d' %event.time)
    print('dur     = %d' %event.dur)
    print('address = %s' %event.address)    
    print('state   = %d' %event.state)
    print('glue    = %d' %event.glue)    
    print('data    = %s' %str(event.data))


def listen_flush(opts, vars):
    pass