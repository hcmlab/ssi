// OnlineClassifier.cpp
// author: Raphael Schwarz <raphaelschwarz@student.uni-augsbrug.de>
// created: 2017/04/12
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************
// Based on Classifier.cpp

#include "OnlineClassifier.h"

#include "base/Factory.h"
#include "ssiml/include/ModelTools.h"
#include "ioput/file/FileAnnotationWriter.h"
#include "OnlineNaiveBayes.h"
//#include <sys/time.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

    ssi_char_t OnlineClassifier::ssi_log_name[] = "online_classifier";
    ssi_char_t OnlineClassifier::ssi_log_name_static[] = "online_classifier";

    OnlineClassifier::OnlineClassifier (const ssi_char_t *file)
        : _file (0),
          _annoFile (0),
          _is_loaded(false),
          _probs (0),
		  _confidence(0.0f),
          _handler (0),
          _n_classes (0),
          _merged_sample_dimension (0),
          _consumer_sr (0),
          _consumer_dim (0),
          _consumer_byte (0),
          _consumer_num (0),
          _lastTimestamp(0),
          _timestampInkrement(1),
          _model(0),
          _n_metas (0),
          _metas (0),
          ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
          cycle_counter(0){

        if (file) {
            if (!OptionList::LoadXML (file, &_options)) {
                OptionList::SaveXML (file, &_options);
            }
            _file = ssi_strcpy (file);
        }

    }

    OnlineClassifier::~OnlineClassifier () {

        if (_file) {
            OptionList::SaveXML (_file, &_options);
            delete[] _file;
        }

        if(_model) delete _model;

        if(_probs) delete _probs;

		_confidence = 0.0f;

        if(_annoFile) fclose(_annoFile);

        m.clear();


    }

    void OnlineClassifier::consume_enter (ssi_size_t stream_in_num,
        ssi_stream_t stream_in[]) {



        if(stream_in_num > 1){
            ssi_wrn("Online learning from multiple streams not yet supported!");
            return;
        }

        if(ssi_exists(_options.model) || ssi_exists(_options.actualModel)){

             //Load existing model given by a user
            FILE *fp = NULL;
            if(ssi_exists(_options.actualModel))
                fp = ssi_fopen (_options.actualModel,"r");
            else
                fp = ssi_fopen(_options.model, "r");
            if (!fp) {
                ssi_wrn ("can't open file %s!", _options.model);
                return;
            }
            char line[SSI_MAX_CHAR], *token;

            readLine (fp, SSI_MAX_CHAR, line);
            token = (char *) strtok(line, "\t");
            token = (char *) strtok(NULL, "\n");

            fclose(fp);

            if(!initModel(token)){
                ssi_wrn("Unable to initialize model!");
                return;
            }

        } else {
            ssi_wrn("No model to load exists!")
            return;
        }

        if(_options.training) {
            if (_options.annoFileLocation == NULL) {
                ssi_err("Please enter location and filename for option annoOut!");
            }
            else { _annoFile = ssi_fopen(_options.annoFileLocation, "w"); }
            if (_options.answerValueName == NULL) ssi_err(
                    "Please enter value name of answer for option valueName!");
        }


    }

    /**
     * @brief OnlineClassifier::initModel
     * Simple function to init online learning model.
     * @param modelName Name of the learning model
     * @return true if model could be created, false if not
     */
    bool OnlineClassifier::initModel(char *modelName){

        if(ssi_strcmp(modelName, OnlineNaiveBayes::GetCreateName())){
            OnlineNaiveBayes* bayes = new OnlineNaiveBayes;

            if(ssi_exists(_options.actualModel)){
                if(bayes->load(_options.actualModel)){
                    ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Found and load actual model %s",_options.actualModel);
                    for (int i = 0; i < bayes->getClassSize(); i++)
                        _n_class_names.push_back(ssi_strcpy(bayes->_class_names[i]));
                } else {
                    goto base;
                }
            } else {
                base:
                if (bayes->load(_options.model)) {
                    ssi_msg(SSI_LOG_LEVEL_DEFAULT, "No actual model %s load base model %s instead", _options.actualModel,_options.model);
                    for (int i = 0; i < bayes->getClassSize(); i++)
                        _n_class_names.push_back(ssi_strcpy(bayes->_class_names[i]));
                } else {
                    ssi_err("No suitable model found!");
                    return false;
                }
            }

            _model = bayes;
            bayes = 0;

        } else {
            ssi_wrn("No valid online model found for name %s!\nValid name is %s",
                    modelName,
                    OnlineNaiveBayes::GetCreateName());
            return false;
        }

        _is_loaded = true;
        _n_classes = _model->getClassSize();
        _probs = new ssi_real_t[_n_classes];

        return true;
    }

    void OnlineClassifier::consume (IConsumer::info consume_info,
        ssi_size_t stream_in_num,
        ssi_stream_t stream_in[]) {

        //ssi_real_t *dataptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);

        bool result = false;

        ssi_msg(SSI_LOG_LEVEL_DEFAULT,"Start classifing");

        if(consume_info.event != NULL)
        {
            result = callModel(consume_info.time,
                           consume_info.dur,
                           stream_in_num,
                           stream_in,
                           consume_info.event->glue_id);
        }else
        {
            result = callModel(consume_info.time,
                           consume_info.dur,
                           stream_in_num,
                           stream_in,
                           1337);
        }



    }

    void OnlineClassifier::consume_flush (ssi_size_t stream_in_num,
        ssi_stream_t stream_in[]) {

    }

    bool OnlineClassifier::callModel(ssi_time_t time,
                                        ssi_time_t dur,
                                        ssi_size_t n_streams,
                                        ssi_stream_t stream_in[],
                                        ssi_size_t glue) {

        //Feature Stream.
        //Schaun wie die Daten dann verarbeitet werden.
        if (!_is_loaded) {
           return false;
        }

        bool result = false;

        result = _model->forward(stream_in[0], _n_classes, _probs, _confidence);

        if (result) {

            ssi_size_t max_ind = 0;
            ssi_real_t max_val = _probs[0];

            //get winning class
            for (ssi_size_t i = 1; i < _n_classes; i++) {
               if (_probs[i] > max_val) {
                   max_val = _probs[i];
                   max_ind = i;
               }
            }

            _handler->handleNotify(time,
                         dur,
                         _n_classes,
                         max_ind,
                         _probs,
                         _n_class_names.data(),
                         0,
                         0,
                         glue);

            ssi_msg(SSI_LOG_LEVEL_DETAIL, "recognized class %s", _n_class_names[max_ind]);

           if(_options.interval){

               if(max_val >= _options.intervalMin && max_val <= _options.intervalMax){

                   if (_options.useDecisionClass) {

                       if (ssi_strcmp(_n_class_names[max_ind], _options.decisionClass))
                       {
                           handleAndTrain(time, dur, max_ind, glue, n_streams, stream_in);
                       }

                   } else {

                       handleAndTrain(time, dur,max_ind,glue,n_streams,stream_in);

                   }

               }

           } else {

               if (max_val >= _options.confidence) {

                   if (_options.useDecisionClass) {

                       if (ssi_strcmp(_n_class_names[max_ind], _options.decisionClass))
                       {
                           handleAndTrain(time, dur, max_ind, glue, n_streams, stream_in);
                       }

                   } else {

                       handleAndTrain(time, dur,max_ind,glue,n_streams,stream_in);

                   }

               }

           }

            if (_options.console) {
                ssi_print_off("");
                for (ssi_size_t i = 0; i < _n_classes; i++) {
                   ssi_print("%s=%.2f ", _n_class_names[i], _probs[i]);
                }
                ssi_print("\n");
            }
       }

       return result;
    }

    void OnlineClassifier::handleAndTrain(ssi_time_t time,
                                          ssi_time_t dur,
                                          ssi_size_t max_ind,
                                          ssi_size_t glue,
                                          ssi_size_t n_streams,
                                          ssi_stream_t stream_in[]) {

        _handler->handle(time,
                         dur,
                         _n_classes,
                         max_ind,
                         _probs,
                         _n_class_names.data(),
                         0,
                         0,
                         glue);


        if (_options.training) {

            //Generate sample
            ssi_sample_t sample;
            sample.class_id = max_ind;
            sample.time = time;
            sample.user_id = 0;
            sample.num = n_streams;
            if(n_streams > 0) {
                sample.streams = new ssi_stream_t *[1];
                sample.streams[0] = &stream_in[0];
            }else {
                ssi_wrn("No streams to learn from");
                return;
            }
            Lock lock(_deleteLock);

            m.insert(std::pair<ssi_size_t, std::pair<ssi_sample_t, ssi_time_t>>
                             (glue,
                              std::pair<ssi_sample_t, ssi_time_t>(sample, dur)));
        }
    }

    bool OnlineClassifier::dirCheck(const ssi_char_t *path){

        FilePath fp(path);
        if(!ssi_exists_dir(fp.getDir())){
            if (!ssi_mkdir_r(fp.getDir()))
            {
                ssi_wrn("There is no way I could create the directory you want, sorry!");
            }
        }
        return true;
    }

    void OnlineClassifier::listen_enter() {


    }

    bool OnlineClassifier::update(IEvents &events,
        ssi_size_t n_new_events,
        ssi_size_t time_ms) {

        if(_options.training) {

            if (n_new_events > 0) {

                if (!_is_loaded) {
                    return false;
                }

                ssi_event_t **es = new ssi_event_t *[n_new_events];
                for (ssi_size_t i = 0; i < n_new_events; i++) {
                    es[n_new_events - 1 - i] = events.next();
                }

                for (ssi_size_t i = 0; i < n_new_events; i++) {

                    ssi_event_t *e = es[i];

                    if (e != NULL) {

                        switch (e->type) {

                            case SSI_ETYPE_STRING: {

                                char *answer = getValue(e->ptr, "{;:}", _options.answerValueName);

                                if (answer != NULL) {

                                    char *glue = getValue(e->ptr, "{;:}", "glue");

                                    //At the moment only for single ADL only
                                    if (glue != NULL) {

                                        char *eptr;
                                        int index = strtol(glue, &eptr, 10);

                                        //Test if convertion happens
                                        if (glue != eptr) {

                                            lernFromPredictionAndAnnotate(index,
                                                                          ssi_strcmp(answer,
                                                                                     "true"));

                                        } else {ssi_wrn("glue id could not be converted!"); }

                                    } else {ssi_wrn("glue id is NULL!"); }

                                } else {ssi_wrn("No value for %s", _options.answerValueName); }

                                break;

                            }

                            default: ssi_wrn("event type not supported '%s'",
                                             SSI_ETYPE_NAMES[e->type]);
                        }

                    }

                }

                delete[] es;
            }

        }

        return true;
    }



    void OnlineClassifier::lernFromPredictionAndAnnotate(const int index, bool answer) {

        ssi_msg(SSI_LOG_LEVEL_DEFAULT, "Index: %d",
                index);//e->glue_id);

        SampleList sampleList;
        sampleList.addUserName(_model->getName());

        for (ssi_char_t *className : _n_class_names) {
            sampleList.addClassName(className);
        }

        if(m.count(index) > 0) {

            //Use all samples ignored by user for given oldSampleClass
            if(_options.oldSamplesUsage){

                int oldClassId = -1;
                for(int i = 0; i < _n_class_names.size(); i++){
                    if(ssi_strcmp(_n_class_names.at(i), _options.oldSamplesClass)) {
                        oldClassId = i;
                    }
                }

                if(oldClassId < 0){
                    ssi_wrn("No class available to match %s", _options.oldSamplesClass)
                    return;
                }

                for (std::map<ssi_size_t, std::pair<ssi_sample_t, ssi_time_t >>::iterator it = m.begin();
                     it != m.end(); it++) {

                    if (it->first < index) {
                        ssi_sample_t oldSamps = it->second.first;
                        _duration = it->second.second;

                        oldSamps.class_id == oldClassId;
                        _tier[0] = 'I';
                        _label = _n_class_names[oldClassId];
                        annotate(oldSamps);
                        sampleList.addSample(&oldSamps, true);

                    }
                    else {
                        break;
                    }

                }

            }

            //Now work with sample which the user sends information
            ssi_sample_t samp = m.at(index).first;
            _duration = m.at(index).second;
            //If answer is not as we expected change sample class id
            if (!answer) {
                int tmp = getOtherClass(samp.class_id);
                if (tmp < 0) {
                    ssi_err("Multiple classsupport > 2 not yet implemented");
                    return;
                } else {
                    samp.class_id = tmp;
                }
            }

            //Label for annotation
            _label = _n_class_names[samp.class_id];

            sampleList.addSample(&samp, true);

            //Timestamp for files
            ssi_char_t buffer[64];
			long unsigned int ms = ssi_time_ms();
            snprintf(buffer, sizeof buffer, "%lu", ms);

            //Should our machine be powerfull enought to learn two samples at the exact same
            //timestamp differentiate these both with additional appendix to the timestamp
            if (_lastTimestamp != 0 &&
                ssi_strcmp(buffer, _lastTimestamp)) {
                ssi_char_t inkrement[10];
                snprintf(inkrement, sizeof inkrement, "(%d)",
                         _timestampInkrement);
                ssi_char_t *tmp = ssi_strcat(buffer, inkrement);
                snprintf(buffer, sizeof buffer, "%s", tmp);
                _timestampInkrement++;
            } else {
                _lastTimestamp = ssi_strcpy(buffer);
                _timestampInkrement = 1;
            }

            //Save sample list
            ssi_char_t *tmpPath = ssi_strcat(
                    ssi_strcat(_options.sampleOutputPath,
                               buffer),
                    ".samples");
            ModelTools::SaveSampleList(sampleList, tmpPath,
                                       File::ASCII);

            _model->train(sampleList, 0);

            //Save model
            if (dirCheck(_options.modelOutputPath)) {
                ssi_char_t *tmpPath = ssi_strcat(
                        ssi_strcat(_options.modelOutputPath,
                                   buffer),
                        ".model");
                if (!_model->save(tmpPath)) ssi_wrn(
                        "Could not save incremented model!");
                if (!_model->save(_options.actualModel)) ssi_wrn("Could not save actual model!");
            }


            for (std::map<ssi_size_t, std::pair<ssi_sample_t, ssi_time_t >>::iterator it = m.begin();
                 it != m.end(); it++) {

                if (it->first == index)
                    m.erase(m.begin(), it);
            }


        } else { ssi_wrn("No event saved for glue %d", index); }

    }

    bool OnlineClassifier::annotate(ssi_sample_t sample) {

        if(_annoFile != NULL) {
            if (_time == 0) {
                _time = sample.time;
            }

            Lock lock(_label_mutex);

            if (_label && _tier) {

                ssi_sprint(_string, "%lf;%lf;%s;#%s", _time, _time + _duration, _label, _tier);

            }

            _time = 0;
            _duration = 0;

            ssi_fprint(_annoFile, "%s\n", _string);
            fflush(_annoFile);

            return true;

        } else {

            ssi_wrn("Couldn't open file for annotation");

            return false;

        }

    }

    int OnlineClassifier::getOtherClass(const int classID){

        if(_n_classes > 2) return -1;

        return classID > 0 ? 0 : 1;

    }

    char* OnlineClassifier::getValue(const char* str, const char* delimiter, const char* value){

		char strTmp[SSI_MAX_CHAR];
        strcpy(strTmp, str);

		char valueTmp[SSI_MAX_CHAR];
        strcpy(valueTmp, value);

		char delimTmp[SSI_MAX_CHAR];
        strcpy(delimTmp, delimiter);

        char* tok = strtok(strTmp, delimTmp);
        while(tok != NULL){
            if(ssi_strcmp(tok,valueTmp))

                return strtok(NULL, delimTmp);
            tok = strtok(NULL, delimTmp);
        }

        return NULL;
    }


    void OnlineClassifier::listen_flush() {

        //releaseTrainer();
    }

    bool OnlineClassifier::setEventListener (IEventListener *listener) {

        if (!listener) {
            return false;
        }

        ssi_size_t eid, sid;

        if (_options.address[0] != '\0') {

            _event_address.setAddress(_options.address);
            sid = Factory::AddString(_event_address.getSender(0));
            eid = Factory::AddString(_event_address.getEvent(0));

        }
        else {

            ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

            sid = Factory::AddString(_options.sname);
            if (sid == SSI_FACTORY_STRINGS_INVALID_ID) {
                return false;
            }
            eid = Factory::AddString(_options.ename);
            if (eid == SSI_FACTORY_STRINGS_INVALID_ID) {
                return false;
            }

            _event_address.setSender(_options.sname);
            _event_address.setEvents(_options.ename);
        }

        _handler = new EventHandler (listener, sid, eid);

        return true;
    }

    // Class EventHandler
    OnlineClassifier::EventHandler::EventHandler(IEventListener *listener,
        ssi_size_t sid,
        ssi_size_t eid)
        : _listener(listener),
        _class_ids(0){

        ssi_event_init (_event, SSI_ETYPE_MAP, sid, eid);
        ssi_event_init(_classificationReport, SSI_ETYPE_MAP, sid,Factory::AddString("Report"));
    }

    OnlineClassifier::EventHandler::~EventHandler () {

        _n_select = 0;
        delete[] _select; _select = 0;

        ssi_event_destroy (_event);
        delete[] _class_ids; _class_ids = 0;
    }

    void OnlineClassifier::EventHandler::handle (ssi_time_t time,
        ssi_time_t duration,
        ssi_size_t n_classes,
        ssi_size_t class_index,
        const ssi_real_t *probs,
        ssi_char_t *const*class_names,
        ssi_size_t n_metas,
        ssi_real_t *metas,
        ssi_size_t glue) {

        _event.glue_id = glue;

        if (_event.tot == 0) {
            _class_ids = new ssi_size_t[n_classes];
            for (ssi_size_t i = 0; i < n_classes; i++) {
                _class_ids[i] = Factory::AddString(class_names[i]);
            }
            if (_winner_only) {
                ssi_event_adjust(_event, sizeof(ssi_event_map_t));
            } else if (_select) {
                ssi_event_adjust(_event, _n_select * sizeof(ssi_event_map_t));

                // check if indices are valid
                for (ssi_size_t i = 0; i < _n_select; i++) {
                    if (_select[i] < 0) {
                        ssi_wrn_static("index '%d' is negative and will be replaced by 0", _select[i]);
                        _select[i] = 0;
                    } else if (_select[i] >= n_classes) {
                        ssi_wrn_static("index '%d' out of range and will be replaced by 0", _select[i]);
                        _select[i] = 0;
                    }
                }

            } else {
                ssi_event_adjust(_event, n_classes * sizeof(ssi_event_map_t));
            }
        }

        ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
        if (_winner_only) {
            ssi_real_t max_val = probs[0];
            ssi_size_t max_ind = 0;
            for (ssi_size_t i = 1; i < n_classes; i++) {
                if (probs[i] > max_val) {
                    max_val = probs[i];
                    max_ind = i;
                }
            }
            ptr[0].id = _class_ids[max_ind];
            ptr[0].value = probs[max_ind];
        } else if (_select) {
            for (ssi_size_t i = 0; i < _n_select; i++) {
                ptr[i].value = probs[_select[i]];
                ptr[i].id = _class_ids[_select[i]];
            }
        } else {
            for (ssi_size_t i = 0; i < n_classes; i++) {
                ptr[i].value = probs[i];
                ptr[i].id = _class_ids[i];
            }
        }

        _event.time = ssi_cast (ssi_size_t, time * 1000 + 0.5);
        _event.dur = ssi_cast (ssi_size_t, duration * 1000 + 0.5);
        _listener->update (_event);
    }

    void OnlineClassifier::EventHandler::handleNotify (ssi_time_t time,
                                                 ssi_time_t duration,
                                                 ssi_size_t n_classes,
                                                 ssi_size_t class_index,
                                                 const ssi_real_t *probs,
                                                 ssi_char_t *const*class_names,
                                                 ssi_size_t n_metas,
                                                 ssi_real_t *metas,
                                                 ssi_size_t glue) {

        _classificationReport.glue_id = glue;

        if (_classificationReport.tot == 0) {
            _class_ids = new ssi_size_t[n_classes];
            for (ssi_size_t i = 0; i < n_classes; i++) {
                _class_ids[i] = Factory::AddString(class_names[i]);
            }
            ssi_event_adjust(_classificationReport, n_classes * sizeof(ssi_event_map_t));
        }

        ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _classificationReport.ptr);

        for (ssi_size_t i = 0; i < n_classes; i++) {
          ptr[i].value = probs[i];
          ptr[i].id = _class_ids[i];
        }

        _classificationReport.time = ssi_cast (ssi_size_t, time * 1000 + 0.5);
        _classificationReport.dur = ssi_cast (ssi_size_t, duration * 1000 + 0.5);
        _listener->update (_classificationReport);
    }

    bool OnlineClassifier::readLine (FILE *fp, ssi_size_t num, ssi_char_t *string) {

        char *string_ptr = string;
        char c;
        c = getc (fp);
        while (c != '\n' && c != EOF) {
            if (c != '\r') {
                if (--num == 0) {
                    *string_ptr = '\0';
                    ssi_wrn ("input string to short");
                    return false;
                }
                *string_ptr = c;
                string_ptr++;
            }
            c = getc (fp);
        }
        *string_ptr = '\0';

        if (ferror (fp)) {
            ssi_wrn ("readLine() failed");
            return false;
        }

        return true;
    }

}
