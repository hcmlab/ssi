'''
ssi_event.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/03/08
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Sends mean of the second stream within series of non-zero values in the first stream.
'''


from xmljson import BadgerFish
from xml.etree.ElementTree import fromstring
from json import dumps
from collections import OrderedDict


def getOptions(opts, vars):

    opts['address'] = 'event@json'
    opts['convert'] = False


def getEventAddress(opts, vars):

    return opts['address']


def listen_enter(opts, vars):

    vars['bf'] = BadgerFish(dict_type=OrderedDict,                   # pick dict class to preserve order of attributes and children
                            xml_fromstring=opts['convert'])          # convert strings if possible

    pass


def update(event, board, opts, vars):    
    
    bf = vars['bf']

    xml = str(event.data)
    data = bf.data(fromstring(xml))
    json = dumps(data, indent=2)

    #print(json)

    board.update(event.time, event.dur, opts['address'], json)



def listen_flush(opts, vars):

    pass