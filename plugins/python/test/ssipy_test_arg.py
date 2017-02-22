'''
ssipy_test_arg.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/02/25
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Shows how to receive and manipulate a ssi array.
'''


def update (event):

    print("UPDATE\n")

    print(event)


def eboard (board):

    print("\nEBOARD\n")

    board.update(100, 50, 'string@sender', 'hello c')


def call (stream):

    print("INPUT\n")
    print("> object type = %s" % type(stream))
    print("> value type  = %s" % stream.type())
    print("> shape       = %d x %d" % stream.shape())

    print("\nMAP\n")
    for i in range(0, stream.len):
        stream[i] = i

    print(stream)

    print("\nITER\n")
    sum = 0;
    for s in stream:
        sum = sum + s
    print("> sum = %f" % sum)


def callArray (array):

    print("INPUT\n")
    print("> object type = %s" % type(array))
    print("> value type  = %s" % array.type())
    print("> len         = %d" % array.length())    

    print("\nMAP\n")
    for i in range(0, array.len):
        array[i] = i

    print(array)

    print("\nITER\n")
    sum = 0;
    for s in array:
        sum = sum + s
    print("> sum = %f" % sum)
