'''
ssi_model.py
author: Dominik Schiller <schiller@hcm-lab.de>
created: 2016/06/29
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''

def load(path, vars, opts):
    print('Load model to path: ', path)
    return path

def save(path, vars, opts):
    print('Save model from path: ', path)
    return path

def train(data, labels, scores, vars, opts):
    print('Start training...')
    for x in data:
        print(str(x))
    for y in labels:
        print(str(y))
    print('... finished training')

def forward(forwardX, probs, vars, opts):
    print('Forwarding:')

    print('Data to classify: ', str(forwardX))
    print('Return array: ', probs)

    for i in range(0, probs.len):
        probs[i] = 5

    print('Returning: ', probs)
