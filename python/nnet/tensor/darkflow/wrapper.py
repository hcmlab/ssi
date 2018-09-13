'''
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2018/07/10
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
'''

from darkflow.net.build import TFNet
from darkflow.defaults import argHandler
import numpy as np
import cv2


def getOptions(opts, vars):

    opts['args'] = ''
    vars['net'] = None


def transform_enter(sin, sout, sxtras, board, opts, vars):   
    
    args = ('. ' + opts['args']).split(' ')

    FLAGS = argHandler()
    FLAGS.setDefaults()
    FLAGS.parseArgs(args)
    net = TFNet(FLAGS)

    vars['net'] = net


def transform(info, sin, sout, sxtras, board, opts, vars):   

    net = vars['net']
    if net:

        img_in = np.asarray(sin)
        img_out = np.asarray(sout)

        preprocessed = net.framework.preprocess(img_in)
        feed_dict = {net.inp: [preprocessed]}
        net_out = net.sess.run(net.out, feed_dict)
        postprocessed = net.framework.postprocess(net_out[0], img_in, False)

        np.copyto(img_out, postprocessed)


def transform_flush(sin, sout, sxtras, board, opts, vars):   

    net = vars['net']
    if net:
        net.sess.close()














#cliHandler(args)



#from .defaults import argHandler #Import the default arguments
#import os
#from .net.build import TFNet


#def cliHandler(args):
#    FLAGS = argHandler()
#    FLAGS.setDefaults()
#    FLAGS.parseArgs(args)

#    # make sure all necessary dirs exist
#    def _get_dir(dirs):
#        for d in dirs:
#            this = os.path.abspath(os.path.join(os.path.curdir, d))
#            if not os.path.exists(this): os.makedirs(this)
    
#    requiredDirectories = [FLAGS.imgdir, FLAGS.binary, FLAGS.backup, os.path.join(FLAGS.imgdir,'out')]
#    if FLAGS.summary:
#        requiredDirectories.append(FLAGS.summary)

#    _get_dir(requiredDirectories)

#    # fix FLAGS.load to appropriate type
#    try: FLAGS.load = int(FLAGS.load)
#    except: pass

#    tfnet = TFNet(FLAGS)
    
#    if FLAGS.demo:
#        tfnet.camera()
#        exit('Demo stopped, exit.')

#    if FLAGS.train:
#        print('Enter training ...'); tfnet.train()
#        if not FLAGS.savepb: 
#            exit('Training finished, exit.')

#    if FLAGS.savepb:
#        print('Rebuild a constant version ...')
#        tfnet.savepb(); exit('Done')

#    tfnet.predict()


#if __name__ == '__main__':

#    args = '--demo camera --model cfg/yolo.cfg --load bin/yolo.weights --gpu 1.0 --threshold 0.1'
#    cliHandler(args)
    