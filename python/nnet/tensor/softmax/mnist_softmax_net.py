import sys
if not hasattr(sys, 'argv'):
    sys.argv  = ['']
import tensorflow as tf


def getModel ():

    x = tf.placeholder(tf.float32, [None, 784])
    W = tf.Variable(tf.zeros([784, 10]))
    b = tf.Variable(tf.zeros([10]))
    y = tf.matmul(x, W) + b

    return [x,y]