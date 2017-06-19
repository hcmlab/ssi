'''
ssi_nnet.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/02
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Neural network.
'''

import numpy
import theano
import six.moves.cPickle as pickle


def load_model(path):

    print("load model: " + path)

    f = open(path,'rb')
    model = pickle.load(f)
    f.close()

    return model


def load_predict(model):

    predict_model = theano.function(
        inputs=[model.input],
        outputs=model.y_pred,
        allow_input_downcast=True)

    return predict_model


def getOptions(opts,vars):

    opts['path'] = 'model.pkl'  
    opts['address'] = 'predict@net'


def getEventAddress(opts, vars):
    return opts['address']


def consume_enter(sin, board, opts, vars):

    model = load_model(opts['path'])    
    predict = load_predict(model)

    vars['model'] = model
    vars['predict'] = predict


def consume(info, sin, board, opts, vars): 

    model = vars['model']
    predict = vars['predict']

    digit = [0,] * sin[0].dim

    for n in range(0,sin[0].num):        
        for d in range(0,sin[0].dim):
            digit[d] = sin[0][n,d]

        result = predict(numpy.asmatrix(digit))
        #print(result)        
        board.update(round(1000*info.time), round(1000*info.dur), opts['address'], str(result))


def consume_flush(sin, board, opts, vars):
    
    pass