'''
ssi_model.py
author: Dominik Schiller <schiller@hcm-lab.de>
created: 2016/06/29
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''

import random


def getOptions(opts, vars):

    opts["is_regression"] = False


def load(path, opts, vars):

    print('Load model to path: ', path)

    return path


def save(path, opts, vars):

    print('Save model from path: ', path)

    return path


def getModelType(types, opts, vars):

    return types.REGRESSION if opts["is_regression"] else types.CLASSIFICATION


def train(data, labels, opts, vars):

    print('Start training...')

    for x in data:
        print(str(x))
    for y in labels:
        print(str(y))

    print('... finished training')


def forward(data, probs_or_score, opts, vars):

    print('Forward...')

    print('Data to classify: ', str(data))    

    if opts["is_regression"]:
        probs_or_score = random.randrange(0,100)/100
        print('Score: ', probs_or_score)
    else:
        for i in range(0, probs_or_score.len):
            probs_or_score[i] = random.randrange(0,100)/100
        print('Probabilities: ', probs_or_score)
    
    conf = random.randrange(0,100)/100    
    print('Confidence: ', conf)

    return conf
