'''
ssi_send.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/08
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''

import numpy
from collections import OrderedDict


def getEventAddress(opts, vars):
    return 'empty,string,tuple,map@python'


def send_enter(opts, vars): # redundant
    pass


def consume(info, sin, board, opts, vars): 

    npin = numpy.asmatrix(sin[0])
    mean = numpy.mean(npin, axis=0)

    time_ms = round(1000 * info.time)
    dur_ms = round(1000 * info.dur)
	
    board.update(time_ms, dur_ms, 'empty@python', state=board.COMPLETED)
    board.update(time_ms, dur_ms, 'string@python', str(mean));
    board.update(time_ms, dur_ms, 'tuple@python', (mean[0,0], mean[0,1]));
    board.update(time_ms, dur_ms, 'map@python', OrderedDict([('x',mean[0,0]), ('y',mean[0,1])]));


def send_flush(opts, vars): # redundant
    pass


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
