'''
ssipy_test_arg.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/02/25
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Shows how to receive and manipulate a ssi array.
'''


def types (stream_type, model_type):

    print("TYPES")

    print(stream_type.UNDEF)
    print(stream_type.CHAR)
    print(stream_type.UCHAR)
    print(stream_type.SHORT)
    print(stream_type.USHORT)
    print(stream_type.INT)
    print(stream_type.UINT)
    print(stream_type.LONG)
    print(stream_type.ULONG)
    print(stream_type.FLOAT)
    print(stream_type.DOUBLE)

    print(model_type.CLASSIFICATION)
    print(model_type.REGRESSION)


def update (event):

    print("UPDATE")

    print(event)

    print()


def eboard (board):

    print("EBOARD")

    board.update(100, 50, 'string@sender', 'hello c')

    print()


def call (stream):

    print()
    print("INPUT")    
    print("> object type = %s" % type(stream))
    print("> value type  = %s" % stream.type())
    print("> shape       = %d x %d" % stream.shape())

    print("MAP")
    for i in range(0, stream.len):
        stream[i] = i

    print(stream)

    print("ITER")
    sum = 0;
    for s in stream:
        sum = sum + s
    print("> sum = %f" % sum)

    print()


def callArray (array):

    print()
    print("INPUT")
    print("> object type = %s" % type(array))
    print("> value type  = %s" % array.type())
    print("> len         = %d" % array.length())    

    print("MAP")
    for i in range(0, array.len):
        array[i] = i

    print(array)

    print("ITER")
    sum = 0;
    for s in array:
        sum = sum + s
    print("> sum = %f" % sum)

    print()
