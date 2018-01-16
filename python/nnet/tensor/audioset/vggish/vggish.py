'''
vggish.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2018/01/10
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Feature extraction using Audioset

https://github.com/tensorflow/models/tree/master/research/audioset
'''

import sys
import os

if not hasattr(sys, 'argv'):
    sys.argv  = ['']

import numpy as np
import tensorflow as tf

import vggish_input
import vggish_params
import vggish_postprocess
import vggish_slim


N_FEATURES = 128
SAMPLE_RATE = 16000


def error(text):
    print('ERROR: ' + text)


def warning(text):
    print('WARNING: ' + text)


def getOptions(opts,vars):

    opts['checkpoint'] = 'vggish_model.ckpt'
    opts['pca_params'] = 'vggish_pca_params.npz'

    vars['loaded'] = False    
    vars['sess'] = None
    vars['pproc'] = None
    vars['features_tensor'] = None
    vars['embedding_tensor'] = None

    vars['warn_reduce'] = None


def loadModel(opts,vars):

    vars['loaded'] = False

    sess = tf.Session()

    pca_path = opts['pca_params']    
    print ('load pca params "{}"'.format(pca_path))   
    if not os.path.exists(pca_path):
        error('pca params not found "{}"'.format(pca_path))
        return  
    pproc = vggish_postprocess.Postprocessor(pca_path)

    ckpt_path = opts['checkpoint']
    print ('load checkpoint "{}"'.format(ckpt_path))   
    if not os.path.exists(ckpt_path):
        error('checkpoint not found "{}"'.format(ckpt_path))
        return  

    vggish_slim.define_vggish_slim(training=False)
    vggish_slim.load_vggish_slim_checkpoint(sess, ckpt_path)
    features_tensor = sess.graph.get_tensor_by_name(vggish_params.INPUT_TENSOR_NAME)
    embedding_tensor = sess.graph.get_tensor_by_name(vggish_params.OUTPUT_TENSOR_NAME)

    vars['sess'] = sess
    vars['pproc'] = pproc
    vars['features_tensor'] = features_tensor
    vars['embedding_tensor'] = embedding_tensor
    vars['loaded'] = True
    

def getSampleDimensionOut(dim, opts, vars):

    return N_FEATURES



def getSampleTypeOut(type, types, opts, vars): 

    if type != types.FLOAT and type != types.DOUBLE:  
        print('types other than short, float and double are not supported') 
        return types.UNDEF

    return type


def transform_enter(sin, sout, sxtra, board, opts, vars): 

    if sin.sr != SAMPLE_RATE:
        warning('resample input from {} to {}'.format(sin.sr, SAMPLE_RATE))        

    loadModel(opts, vars)


def transform(info, sin, sout, sxtra, board, opts, vars): 

    input = np.asarray(sin).squeeze()
    output = np.asarray(sout)

    if not vars['loaded']:
        output.fill(0)
        return

    sess = vars['sess']
    pproc = vars['pproc']            
    features_tensor = vars['features_tensor']
    embedding_tensor = vars['embedding_tensor']

    batch = vggish_input.waveform_to_examples(input, sin.sr)

    #print(batch.shape)

    [features] = sess.run([embedding_tensor], 
                                 feed_dict={features_tensor: batch})

    # Reduce feature dimension if necessary

    if features.shape[0] > 1:    
        if not vars['warn_reduce']:
            warning('reduce feature dimension from {} to 1'.format(features.shape[0]))
            vars['warn_reduce'] = True
        features = np.mean(features, axis=0)
    
    np.copyto(output, features)


def transform_flush(sin, sout, sxtra, board, opts, vars): 

    sess = vars['sess']
    sess.close()
