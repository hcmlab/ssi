'''
ssi_mnist.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/11
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Reads images from the mnist database.
'''

import pickle, gzip, numpy


def load (path, vars):

    print('load mnist dataset: ' + path)

    f = gzip.open(path, 'rb')
    train_set, valid_set, test_set = pickle.load(f, encoding='latin1')
    f.close()
    
    vars['data'] = test_set


def getOptions(opts,vars):
    
    opts['sr'] = 1    
    opts['width'] = 28
    opts['height'] = 28
    opts['path'] = 'mnist.pkl.gz'


def getChannelNames(opts, vars):
    
    return {'digit':'stream of handwritten digits','truth':'ground truth'}


def initChannel(name, channel, types, opts, vars):

    if name == 'digit':

        channel.dim = opts['width'] * opts['height']
        channel.type = types.FLOAT
        channel.sr = opts['sr']

    elif name == 'truth':

        channel.dim = 1
        channel.type = types.FLOAT
        channel.sr = opts['sr']

    else:

        print('unkown channel name: ' + name)


def connect(opts, vars):
    
    load(opts['path'], vars)
    vars['index_digit'] = 0
    vars['index_truth'] = 0


def read(name, sout, reset, board, opts, vars):    

    data = vars['data']

    if name == 'digit':

        if sout.len != len(data[0][0]):
            print('invalid sample dimension')
            pass

        index = vars['index_digit']

        for n in range(0, sout.num):

            if index >= len(data[0]):                
                index = 0
                        
            for i in range(0, sout.len):
                sout[i] = data[0][index][i]

            if not reset:                
                index = index + 1

        vars['index_digit'] = index

    elif name == 'truth':

        if sout.len != 1:
            print('invalid sample dimension')
            pass

        index = vars['index_truth']

        for n in range(0, sout.num):

            if index >= len(data[0]):                
                index = 0
                        
            sout[n] = data[1][index]

            if not reset:                
                index = index + 1

        vars['index_truth'] = index

    else:

        print('unkown channel name: ' + name)       

def disconnect(opts, vars):
    pass