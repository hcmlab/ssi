'''
ssi_send.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/08
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''


def getEventAddress(opts, vars):
    return 'string,mean@python'


def send_enter(opts, vars):
    pass


def consume_enter(sin, board, opts, vars):
    
    if len(sin) != 2:
        print('requires two input streams')
    else:
        board.update(100, 50, 'string@python', 'enter')
        vars['state'] = 0
        vars['sum'] = [0,] * sin[1].dim
        vars['count'] = 0
        vars['start'] = 0


def consume(info, sin, board, opts, vars): 

    if len(sin) != 2:
        pass
    
    state = 0
    for s in sin[0]:
        if s != 0:
            state = 1

    sum = vars['sum']

    if state == 1 and vars['state'] == 0:
        vars['state'] = 1
        vars['start'] = info.time

    if state == 1:
        vars['count'] += sin[1].num
        for n in range(0,sin[1].num):
            for d in range(0,sin[1].dim):
                sum[d] += sin[1][n,d]                                    

    if state == 0 and vars['state'] == 1:
        mean = [0,] * sin[1].dim
        for d in range(0,sin[1].dim):
            mean[d] = sum[d] / vars['count']
            sum[d] = 0
        time = round(1000 * vars['start'])
        dur = round(1000 * info.time) + round(1000 * info.dur) - time
        board.update(time, dur, 'mean@python', mean)
        vars['state'] = 0
        vars['count'] = 0


def consume_flush(sin, board, opts, vars):
    
    if len(sin) != 2:
        pass
        
    board.update(100, 50, 'string@python', 'flush')


def send_flush(opts, vars):
    pass

