'''
ssi_nnet.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/02
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Neural network.
'''

import sys
if not hasattr(sys, 'argv'):
    sys.argv  = ['']
import tensorflow as tf


def getOptions(opts,vars):

    opts['path'] = './mnist_convolutional'  
    opts['address'] = 'predict@net'   


def getEventAddress(opts, vars):

    return opts['address']


def consume_enter(sin, board, opts, vars):

    print("create model")    

    module = __import__('mnist_convolutional_net')
    [x,y,keep_prob] = module.getModel()

    sess = tf.InteractiveSession()

    print("restore variables %s" % opts['path'])

    saver = tf.train.Saver()    
    saver.restore(sess, opts['path'])

    vars['sess'] = sess
    vars['y'] = y
    vars['x'] = x
    vars['keep_prob'] = keep_prob;

    pass


def consume(info, sin, board, opts, vars): 

    sess = vars['sess']
    x = vars['x']
    y = vars['y']
    keep_prob = vars['keep_prob']

    digit = [0,] * sin[0].dim
    for n in range(0,sin[0].num):        
        for d in range(0,sin[0].dim):
            digit[d] = sin[0][n,d]

    result = sess.run(tf.argmax(y, 1), {x: [digit], keep_prob : 1.0}) 
    board.update(round(1000*info.time), round(1000*info.dur), opts['address'], str(result))


def consume_flush(sin, board, opts, vars):
    
    sess = vars['sess']
    sess.close()