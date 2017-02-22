'''
ssipy_test_arg.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/02/25
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Shows how to convert a ssi stream to a numpy matrix.
'''

import numpy
import random

def call (stream):

    print("INPUT\n")
    print("> object type = %s" % type(stream))
    print("> value type  = %s" % stream.type())
    print("> shape       = %d x %d" % stream.shape())

    for i in range(0, stream.len):
        stream[i] = random.uniform(0,100)

    print(stream)

    print("\nNUMPY\n")
    npmat = numpy.asmatrix(stream)

    print(npmat)
    print("\n> shape  = %d x %d" % npmat.shape)
    print("> mean   = %.2f" % npmat.mean())
    print("> min    = %.2f" % npmat.min())
    print("> max    = %.2f" % npmat.max())
    npmat = npmat.reshape((stream.dim, stream.num))
    npmat.sort(axis=0)


def callArray (array):

    print("INPUT\n")
    print("> object type = %s" % type(array))
    print("> value type  = %s" % array.type())
    print("> len         = %d" % array.length())

    for i in range(0, array.len):
        array[i] = random.uniform(0,100)

    print(array)

    print("\nNUMPY\n")
    npmat = numpy.asarray(array)

    print(npmat)
    print("> mean   = %.2f" % npmat.mean())
    print("> min    = %.2f" % npmat.min())
    print("> max    = %.2f" % npmat.max())
    npmat.sort()
