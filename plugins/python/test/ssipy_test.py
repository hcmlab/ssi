'''
ssipy_test.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/02/25
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Shows how to create and manipulate a ssi stream.
'''


import ssipy
import random
from collections import OrderedDict


def call ():

    print(ssipy.ABOUT + "\n")
    print(ssipy.VERSION + "\n")

    info = ssipy.info(time=100,dur=50,frame=10,delta=5)
    print('time  = %d' %info.time)
    print('dur   = %d' %info.dur)
    print('frame = %d' %info.frame)
    print('delta = %d' %info.delta)

    types = ssipy.type()

    channel = ssipy.channel(dim=0,byte=0,type=types.UNDEF,sr=0.0)
    channel.dim = 2
    channel.type = types.FLOAT
    channel.byte = types.size(types.FLOAT)
    channel.sr = 5.0
    print(channel)

    array = ssipy.array(len=5)
    print(repr(array))
    print(type(array))

    print(array.length())
    print(array.type())

    for n in range(0, array.len):
        array[n] = random.uniform(0,100)
        print(array[n])

    for s in array:
        print(s)

    print(array)

    stream = ssipy.stream(num=5,dim=2,type=types.FLOAT,sr=100,time=10)
    print(repr(stream))
    print(type(stream))

    print(stream.length())
    print(stream.shape())
    print(stream.type())

    for n in range(0, stream.num):
        for d in range(0, stream.dim):
            stream[n,d] = random.uniform(0,100)
            print(stream[n,d])

    for s in stream:
        print(s)

    print(stream)

    event = ssipy.event(100, 50, 'empty@sender')
    print(repr(event))
    print(event)

    event = ssipy.event(100, 50, 'string@sender', 'hello world')
    print(repr(event))
    print(event)

    data = (1,2,3,4)
    event = ssipy.event(100, 50, 'tuple@sender', data)
    print(repr(event))
    print(event)

    data = [1.0,2.0,3.0,4.0]
    event = ssipy.event(100, 50, 'list@sender', data)
    print(repr(event))
    print(event)

    data = {'one':1, 'two':2, 'three':3, 'four':4}
    event = ssipy.event(100, 50, 'dict@sender', data, ssipy.CONTINUED, 0, random.random())
    print(repr(event))
    print(event)

    data = OrderedDict([('one',1), ('two',2), ('three',3), ('four',4)])
    event = ssipy.event(100, 50, 'ordered@sender', data=data, state=ssipy.COMPLETED, glue=1, prob=random.random())
    print(repr(event))
    print(event)

if __name__ == '__main__':
    print('Running...')
    call()
