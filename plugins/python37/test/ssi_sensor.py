'''
ssi_sensor.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/10
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Generates saw and sine waves.
'''

import math


def getOptions(opts,vars):
    
    opts['sr'] = 10    
    opts['dim'] = 1


def getChannelNames(opts, vars):
    
    return {'saw':'a saw wave', 
            'sine':'a sine wave'}


def initChannel(name, channel, types, opts, vars):

    if name == 'saw':

        channel.dim = 1
        channel.type = types.DOUBLE
        channel.sr = 100

    elif name == 'sine':

        channel.dim = opts['dim']
        channel.type = types.FLOAT
        channel.sr = opts['sr']

    else:
        print('unkown channel name')


def connect(opts, vars):
    pass

        
def read(name, sout, reset, board, opts, vars):    

    time = sout.time
    delta = 1.0 / sout.sr

    if name == 'saw':

        for n in range(0, sout.num):
            for d in range(0, sout.dim):
                sout[n,d] = time - math.floor(time)
            time += delta

    elif name == 'sine':

        for n in range(0, sout.num):
            for d in range(0, sout.dim):
                sout[n,d] = math.sin(2*math.pi*time)
            time += delta

    else:

        print('unkown channel name')


def disconnect(opts, vars):
    pass