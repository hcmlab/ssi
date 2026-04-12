'''
ssi_asr.py
author: Johannes Wagner <wagner@hcm-lab.de>
created: 2016/06/01
Copyright (C) University of Augsburg, Lab for Human Centered Multimedia

Speech recognition.
'''

import numpy
import speech_recognition as sr


def getOptions(opts,vars):

    opts['engine'] = 'google'  
    opts['key'] = ''
    opts['address'] = 'predict@asr'
    opts['language'] = 'en-US'	


def getEventAddress(opts, vars):
    return opts['address']


def consume_enter(sin, board, opts, vars):
    pass


def consume(info, sin, board, opts, vars): 

    stream = numpy.asarray(sin[0])
    bytes = stream.tobytes()
    audio = sr.AudioData(bytes, int(sin[0].sr), int(sin[0].byte))    
    r = sr.Recognizer()
    engine = opts['engine']
    key = opts['key']
    lang = opts['language']

    result = ''

    if engine == 'sphinx':
        # recognize speech using Sphinx
        try:
            result = r.recognize_sphinx(audio, language=lang)
        except sr.UnknownValueError:
            print("Sphinx could not understand audio")
        except sr.RequestError as e:
            print("Sphinx error; {0}".format(e))

    if engine == 'google':
        # recognize speech using Google Speech Recognition
        try:
            # for testing purposes, we're just using the default API key
            # to use another API key, use `r.recognize_google(audio, key="GOOGLE_SPEECH_RECOGNITION_API_KEY")`
            # instead of `r.recognize_google(audio)`
            result = r.recognize_google(audio, language=lang)
        except sr.UnknownValueError:
            print("Google Speech Recognition could not understand audio")
        except sr.RequestError as e:
            print("Could not request results from Google Speech Recognition service; {0}".format(e))

    if engine == 'wit.ai':
        # recognize speech using Wit.ai
        WIT_AI_KEY = key 
        # Wit.ai keys are 32-character uppercase alphanumeric strings
        try:
            result = r.recognize_wit(audio, key=WIT_AI_KEY, language=lang)
        except sr.UnknownValueError:
            print("Wit.ai could not understand audio")
        except sr.RequestError as e:
            print("Could not request results from Wit.ai service; {0}".format(e))

    if engine == 'bing':
        # recognize speech using Microsoft Bing Voice Recognition
        BING_KEY = key 
        # Microsoft Bing Voice Recognition API keys 32-character lowercase hexadecimal strings
        try:
            result = r.recognize_bing(audio, key=BING_KEY, language=lang)
        except sr.UnknownValueError:
            print("Microsoft Bing Voice Recognition could not understand audio")
        except sr.RequestError as e:
            print("Could not request results from Microsoft Bing Voice Recognition service; {0}".format(e))

    if engine == 'api.ai':
        # recognize speech using api.ai
        API_AI_CLIENT_ACCESS_TOKEN = key 
        # api.ai keys are 32-character lowercase hexadecimal strings
        try:
            result = r.recognize_api(audio, client_access_token=API_AI_CLIENT_ACCESS_TOKEN, language=lang)
        except sr.UnknownValueError:
            print("api.ai could not understand audio")
        except sr.RequestError as e:
            print("Could not request results from api.ai service; {0}".format(e))

    if engine == 'ibm':
        # recognize speech using IBM Speech to Text
        [IBM_USERNAME, IBM_PASSWORD] = key.split('@')
        # IBM Speech to Text usernames are strings of the form XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
        # IBM Speech to Text passwords are mixed-case alphanumeric strings
        try:
            result = r.recognize_ibm(audio, username=IBM_USERNAME, password=IBM_PASSWORD, language=lang)
        except sr.UnknownValueError:
            print("IBM Speech to Text could not understand audio")
        except sr.RequestError as e:
            print("Could not request results from IBM Speech to Text service; {0}".format(e))

    if result != '':
        time = int(info.time * 1000)
        dur = int(info.dur * 1000)
        board.update(time, dur, opts['address'], result)


def consume_flush(sin, board, opts, vars):
    
    pass