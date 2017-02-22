'''
ssi_model.py
author: Dominik Schiller <schiller@hcm-lab.de>
created: 2016/06/29
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''

def load(path):
    print('Load model to path: ', path)
    return path

def save(path):
    print('Save model from path: ', path)
    return path

def train(trainX, trainY):
    print('Start training...')
    for x in trainX:
        print(str(x))
    for y in trainY:
        print(str(y))
    print('... finished training')

def forward(forwardX, probs):
    print('Forwarding:')

    print('Data to classify: ', str(forwardX))
    print('Return array: ', probs)

    for i in range(0, probs.len):
        probs[i] = 5

    print('Returning: ', probs)
