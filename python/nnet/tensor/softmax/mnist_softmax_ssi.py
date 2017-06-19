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


def create_model():
      
    module = __import__('mnist_softmax_net')
    [x,y] = module.getModel()

    return [x,y]


def getOptions(opts,vars):

    opts['path'] = './mnist_softmax'  
    opts['address'] = 'predict@net'   


def getEventAddress(opts, vars):

    return opts['address']


def consume_enter(sin, board, opts, vars):

    print("create model")    

    [x,y] = create_model()
    sess = tf.InteractiveSession()

    print("restore variables %s" % opts['path'])

    saver = tf.train.Saver()    
    saver.restore(sess, opts['path'])

    vars['sess'] = sess
    vars['y'] = y
    vars['x'] = x

    pass


def consume(info, sin, board, opts, vars): 

    sess = vars['sess']
    x = vars['x']
    y = vars['y']

    digit = [0,] * sin[0].dim
    for n in range(0,sin[0].num):        
        for d in range(0,sin[0].dim):
            digit[d] = sin[0][n,d]

    result = sess.run(tf.argmax(y, 1), {x: [digit]}) 
    board.update(round(1000*info.time), round(1000*info.dur), opts['address'], str(result))


def consume_flush(sin, board, opts, vars):
    
    pass