'''
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2018/09/04
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
'''

import torch
from net import Net


def getOptions(opts,vars):

    opts['no-cuda'] = False
    opts['path'] = './net.pth'  
    opts['address'] = 'predict@net'   


def getEventAddress(opts, vars):

    return opts['address']


def consume_enter(sin, board, opts, vars):

    print("create model")    

    use_cuda = not opts['no-cuda'] and torch.cuda.is_available()
    device = torch.device('cuda' if use_cuda else 'cpu')
    model = Net().to(device)
    model.load_state_dict(torch.load(opts['path']))

    vars['device'] = device
    vars['model'] = model


def consume(info, sin, board, opts, vars): 

    device = vars['device']
    model = vars['model']

    for n in range(0,sin[0].num):        

        digit = [0,] * sin[0].dim
        for d in range(0,sin[0].dim):
            digit[d] = sin[0][n,d]

        data = torch.tensor(digit).reshape(1,1,28,28).to(device)
        result = model(data).max(1)[1].item()
    
        board.update(round(1000*info.time), round(1000*info.dur), opts['address'], str(result))


def consume_flush(sin, board, opts, vars):
    
    pass