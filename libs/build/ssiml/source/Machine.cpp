// Machine.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/10/20
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

#include "Machine.h"
#include "base/Factory.h"
#include "base/StringList.h"
#include "ioput/wav/WavTools.h"
#include "ioput/file/FileTools.h"
#include "ElanDocument.h"
#include "signal/SignalTools.h"
#include "ModelTools.h"
#include "ElanTools.h"
#include "Trainer.h"
#include "ISOverSample.h"
#include "ISUnderSample.h"
#include "ISTrigger.h"
#include "ISSelectSample.h"
#include "ISMissingData.h"
#include "Evaluation.h"

#if __gnu_linux__
using std::min;
using std::max;
#endif

namespace ssi {                                                           

const ssi_char_t *Machine::ResampleTypeNames[3] = {"none", "under", "over"};
bool Machine::framework_is_registered = false;

bool Machine::Init (Machine::Config &config) {

	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.transformFolderName.str ());
	ssi_mkdir (sstring);

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.eventFolderName.str ());
	ssi_mkdir (sstring);

	if(config.doSnippets){
        ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.snippetFolderName.str ());
		ssi_mkdir (sstring);
	}

	ssi_print ("\nINIT > %s (%s)\n\n", config.listName.str (), config.signalName.str ());

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	ssi_time_t sr;
	ssi_stream_t from;

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

		if (config.isAudio) {

            ssi_sprint (sstring, "%s/%s/%s.wav", config.rawFolderName.str (), session, config.signalName.str ());
			ssi_print ("process > '%s'\n", sstring);
			if (!WavTools::ReadWavFile (sstring, from, true)) {
				return false;
			}

		} else {

            ssi_sprint (sstring, "%s/%s/%s", config.rawFolderName.str (), session, config.signalName.str ());
			ssi_print ("process > '%s'\n", sstring);
			if (!FileTools::ReadStreamFile (sstring, from)) {
				return false;
			}
		}
		
		if (i == 0) {
			sr = from.sr;
			ssi_sprint (starget, "%s{%.0lf}", config.signalName.str (), sr);
            ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), starget);
			ssi_mkdir (sstring);
		} else if (sr != from.sr) {
			ssi_wrn ("sample rate must not change (%lf != %lf)", from.sr, sr);
			return false;
		}

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), starget, session);

		if (!FileTools::WriteStreamFile (File::BINARY, sstring, from)) {
			return false;
		}
		

		ssi_stream_destroy (from);

	}

	config.sourceName = "";
	config.targetName = starget;
	config.triggerName = "";
	config.fusionName = "";
	config.discretize = false;

	return true;
};

bool Machine::Transform (Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, ITransformer *theTransformer) {

	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nTRANSFORM > %s (%s)\n\n", config.listName.str (), theTransformer->getName ());

    ssi_sprint (starget, "%s/%s{f%ud%u}", config.sourceName.str (), prefix, frameSize, deltaSize);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str ());
	ssi_mkdir (sstring);
	//config.targetName = sstring;

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

		ssi_stream_t from, to;

        ssi_sprint (sstring, "%s/%s/%s/%s.stream", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!config.reCalculate && ssi_exists (sstring)) {	
			ssi_print ("...skip\n");
			continue;
		}
		
        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		ssi_print ("process > '%s'\n", sstring);
		
		if (!FileTools::ReadStreamFile (sstring, from)) {
			return false;
		}
		SignalTools::Transform (from, to, *theTransformer, frameSize, deltaSize, true, true);

        ssi_sprint (sstring, "%s/%s/%s/%s.stream", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!FileTools::WriteStreamFile (File::ASCII, sstring, to)) {
			return false;
		}

		ssi_stream_destroy (from);
		ssi_stream_destroy (to);
	}

	return true;
}

bool Machine::PrepareBayesNet (Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, const ssi_char_t *tierName){

	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

    ssi_sprint (starget, "%s/%s/%s", config.rootFolder.str (), config.eventFolderName.str (), config.fusionName.str());
	ssi_mkdir(starget);

	ssi_char_t tierNameInverted[SSI_MAX_CHAR];
	ssi_sprint (tierNameInverted, "NO_%s", tierName);

	ssi_print ("\nSAMPLES > %s\n\n", config.listName.str ());

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);

        ssi_sprint (sstring, "%s/%s/%s/%s.eaf", config.rootFolder.str (), config.rawFolderName.str (), session, config.annoName.str ());
		ssi_print ("process > '%s'\n", sstring);

		ElanDocument *elanDoc = ElanDocument::Read (sstring);		 
		ElanTier tier = (*elanDoc)[tierName];

		if (tier.size () == 0) {
			continue;
		}

		ElanTier tierPack (tierName);			
		tier.pack (tierPack);
		tierPack.unify(tierName);
		ElanTier tierPackSplit (tierName);
		tierPack.split (tierPackSplit, tierPackSplit, tierNameInverted, frameSize, deltaSize, ssi_cast (ssi_size_t, 0.5 * frameSize + 0.5), 0);
		tierPackSplit.sort();

        ssi_sprint (sstring, "%s/%s/%s/%s_%s.txt", config.rootFolder.str (), config.rawFolderName.str (), session, config.fusionName.str(), tierName);
		ssi::File *tierAnno = ssi::File::CreateAndOpen(File::ASCII, File::WRITE, sstring);
		ElanTier::iterator it;
		for (it = tierPackSplit.begin(); it != tierPackSplit.end(); it++) {
			if(strcmp(it->value.str(), tierName) == 0){
				tierAnno->writeLine(tierName);
			}else{
				tierAnno->writeLine(tierNameInverted);
			}
		}
		tierAnno->close();
		

	}

	return true;
}

bool Machine::SamplesFromSingleTier(Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, const ssi_char_t *tierName, bool includeInverted, bool pack, bool useTierNamesAsLabels) {

	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_char_t tierNameInverted[SSI_MAX_CHAR];
	ssi_sprint (tierNameInverted, "NO_%s", tierName);

	ssi_print ("\nSAMPLES > %s\n\n", config.listName.str ());

    ssi_sprint (starget, "%s/%s{f%ud%u}", config.sourceName.str (), prefix, frameSize, deltaSize);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str ());
	
	if(ssi_mkdir (sstring)){
		ssi_print("\ncreated directory %s\n", sstring);
	}else{
		ssi_print("\ncould not create directory %s\n", sstring);
	}

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

		ssi_stream_t from;
		SampleList to;

        ssi_sprint (sstring, "%s/%s/%s/%s.samples", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);

		if (!config.reCalculate && ssi_exists (sstring)) {	
			ssi_print ("...skip\n");
			continue;
		}
		
        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		ssi_print ("process > '%s'\n", sstring);

		if (!FileTools::ReadStreamFile (sstring, from)) {
			return false;
		}
		
        ssi_sprint (sstring, "%s/%s/%s.eaf", config.rawFolderName.str (), session, config.annoName.str ());
		ssi_print ("split > '%s'\n", sstring);

		ElanDocument *elanDoc = ElanDocument::Read (sstring);		 
		ElanTier tier = (*elanDoc)[tierName];

		if (tier.size () == 0) {
			continue;
		}

		ElanTier tierPack (tierName);
		if (pack) {
			tier.pack(tierPack);
		}

		if (includeInverted) {
			ElanTier tierPackSplit (tierName);
			ssi_size_t tiersize = ssi_cast (ssi_size_t, from.num / from.sr) * 1000;
			ElanTier tierInverse (tierNameInverted);
			if (pack) {
				tierPack.split(tierPackSplit, tierInverse, tierNameInverted, frameSize, deltaSize, ssi_cast(ssi_size_t, 0.5 * frameSize + 0.5), tiersize);
			} else {
				tier.split(tierPackSplit, tierInverse, tierNameInverted, frameSize, deltaSize, ssi_cast(ssi_size_t, 0.5 * frameSize + 0.5), tiersize);
			}
			ElanTools::LoadSampleList (to, from, tierPackSplit, session, true);
			ElanTools::LoadSampleList (to, from, tierInverse, session, true);
		} else {
			ElanTier tierPackSplit (tierName);
			if (pack) {
				tierPack.split(tierPackSplit, frameSize, deltaSize, ssi_cast(ssi_size_t, 0.5 * frameSize + 0.5));
			} else {
				tier.split(tierPackSplit, frameSize, deltaSize, ssi_cast(ssi_size_t, 0.5 * frameSize + 0.5));
			}
			ElanTools::LoadSampleList(to, from, tierPackSplit, session, useTierNamesAsLabels);
		}

		/*ElanTier tierPackSplit (tierName);
		tierPack.split (tierPackSplit, frameSize, deltaSize, ssi_cast (ssi_size_t, 0.5 * frameSize + 0.5));
		ModelTools::LoadSampleList (to, from, tierPackSplit, session, true);

		if (includeInverted) {

			ElanTier tierPackInvert (tierNameInverted);
			ssi_size_t tiersize = ssi_cast (ssi_size_t, from.num / from.sr) * 1000;
			tierPack.invert (tierPackInvert, "", tiersize);

			ElanTier tierPackInvertSplit (tierNameInverted);
			tierPackInvert.split (tierPackInvertSplit, frameSize, deltaSize, ssi_cast (ssi_size_t, 0.5 * frameSize + 0.5));
			ModelTools::LoadSampleList (to, from, tierPackInvertSplit, session, true);

		}*/

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!ModelTools::SaveSampleList (to, sstring, File::ASCII)) {
			return false;
		}

		ModelTools::PrintInfo (to, ssiout);

		ssi_stream_destroy (from);
	}

	return true;
}

bool Machine::SamplesFromMultipleTiers(Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, ssi_size_t n_tier, const ssi_char_t *tierNames[], bool pack, bool useTierNamesAsLabels) {
	
	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nSAMPLES > %s\n\n", config.listName.str ());

    ssi_sprint (starget, "%s/%s{f%ud%u}", config.sourceName.str (), prefix, frameSize, deltaSize);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str ());
	ssi_mkdir (sstring);

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

		ssi_stream_t from;
		SampleList to;

        ssi_sprint (sstring, "%s/%s/%s/%s.samples", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);

		if (!config.reCalculate && ssi_exists (sstring)) {		
			ssi_print ("...skip\n");
			continue;
		}
		
        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		ssi_print ("process > '%s'\n", sstring);

		if (!FileTools::ReadStreamFile (sstring, from)) {
			return false;
		}
		
        ssi_sprint (sstring, "%s/%s/%s.eaf", config.rawFolderName.str (), session, config.annoName.str ());
		ssi_print ("split > '%s'\n", sstring);

		ElanDocument *elanDoc = ElanDocument::Read (sstring);		 

		for (ssi_size_t i = 0; i < n_tier; i++) {

			ElanTier tier = (*elanDoc)[tierNames[i]];

			if (tier.size () == 0) {
				continue;
			}

			ElanTier tierPack(tierNames[i]);
			if (pack) {
				tier.pack(tierPack);
			}

			ElanTier tierPackSplit (tierNames[i]);
			if (pack) {
				tierPack.split(tierPackSplit, frameSize, deltaSize, ssi_cast(ssi_size_t, 0.5 * frameSize + 0.5));
			} else {
				tier.split(tierPackSplit, frameSize, deltaSize, ssi_cast(ssi_size_t, 0.5 * frameSize + 0.5));
			}
			
			ElanTools::LoadSampleList (to, from, tierPackSplit, session, useTierNamesAsLabels);

		}

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!ModelTools::SaveSampleList(to, sstring, File::ASCII)) {
			return false;
		}

		ModelTools::PrintInfo (to, ssiout);

		ssi_stream_destroy (from);
	}

	return true;
}

bool Machine::SamplesSelectWithinTier (Config &config, const ssi_char_t *prefix, const ssi_char_t *tierName) {

	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nSAMPLES SELECT > %s, %s\n\n", config.listName.str (), tierName);	

	ssi_sprint (starget, "%s_%s", config.sourceName.str (), prefix);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str ());
	ssi_mkdir (sstring);

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		
		
        ssi_sprint (sstring, "%s/%s/%s/%s.samples", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);

		if (!config.reCalculate && ssi_exists (sstring)) {		
			ssi_print ("...skip\n");
			continue;
		}

		SampleList samples;
		
        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		ssi_print ("load > '%s'\n", sstring);

		if (!ModelTools::LoadSampleList (samples, sstring)) {
			return false;
		}
		
		samples.sort ();
		ModelTools::PrintInfo (samples, ssiout);

        ssi_sprint (sstring, "%s/%s/%s.eaf", config.rawFolderName.str (), session, config.annoName.str ());
		ssi_print ("select > '%s'\n", sstring);

		ElanDocument *elanDoc = ElanDocument::Read (sstring);		 
		ElanTier tier = (*elanDoc)[tierName];

		if (tier.size () == 0) {
			continue;
		}

		ElanTier tierPack (tierName);			
		tier.pack (tierPack);

		ElanTier::iterator iter;
		samples.reset ();
		ssi_sample_t *sample = samples.next ();
		ssi_size_t current_segment_from = 0, current_segment_to = 0;
		ssi_size_t *select = new ssi_size_t[samples.getSize ()];
		ssi_size_t n_select = 0;
		ssi_size_t index = 0;
		for (iter = tier.begin (); iter != tier.end (); iter++) {			
			if (!sample) {
				break;
			}
			while (ssi_cast (ssi_size_t, sample->time * 1000 + 0.5) <= iter->from) {				
				index++;
				if (!(sample = samples.next ())) {					
					break;
				}
			}
			if (!sample) {
				break;
			}
			while (ssi_cast (ssi_size_t, sample->time * 1000 + 0.5)  <= iter->to) {
				select[n_select++] = index;
				index++;
				if (!(sample = samples.next ())) {
					break;
				}		
			}
		}

		ISSelectSample samples_s (&samples);
		samples_s.setSelection (n_select, select);

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!ModelTools::SaveSampleList (samples_s, sstring, File::BINARY)) {
			return false;
		}

		ModelTools::PrintInfo (samples_s, ssiout);

	}

	return true;
}
	
bool Machine::SamplesFromStream (Config &config, const ssi_char_t *prefix, const ssi_char_t *className, const ssi_char_t *userName) {

	/*ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nSAMPLES FROM STREAM %s\n\n", config.listName.str ());

	ssi_sprint (starget, "%s_%s", config.sourceName.str (), prefix);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.sampleFolderName.str (), config.targetName.str ());
	ssi_mkdir (sstring);

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

		ssi_stream_t from;
		SampleList to;

        ssi_sprint (sstring, "%s/%s/%s/%s.samples", config.rootFolder.str (), config.sampleFolderName.str (), config.targetName.str (), session);

		if (!config.reCalculate && ssi_exists (sstring)) {		
			ssi_print ("...skip\n");
			continue;
		}
		
        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		ssi_print ("process > '%s'\n", sstring);

		if (!FileTools::ReadStreamFile (sstring, from)) {
			return false;
		}
		
        ssi_sprint (sstring, "%s/%s/%s/%s.eaf", config.rootFolder.str (), config.rawFolderName.str (), session, config.annoName.str ());
		ssi_print ("split > '%s'\n", sstring);

		ssi_size_t class_id = to.addClassName (className);
		ssi_size_t user_id = to.addUserName (userName);

		ssi_time_t delta = 1.0 / from.sr;
		for (ssi_size_t j = 0; j < from.num; j++) {
			ssi_sample_t *sample = new ssi_sample_t ();
			ssi_sample_create (*sample, 1, 0, 0, j * delta, 1.0f);
			ssi_stream_t *chunk = new ssi_stream_t ();
			ssi_stream_copy (from, *chunk, j, j+1);
			sample->streams[0] = chunk;
			to.addSample (sample);
		}

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.sampleFolderName.str (), config.targetName.str (), session);
		if (!ModelTools::SaveSampleList (to, sstring, File::BINARY)) {
			return false;
		}

		ModelTools::PrintInfo (to, ssiout);

		ssi_stream_destroy (from);
	}
*/
	return true;
}

bool Machine::Trigger (Config &config, const ssi_char_t *prefix, const ssi_char_t *trigger, ssi_size_t stream_index, ssi_real_t thres) {

	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nTRIGGER > %s\n\n", config.listName.str ());

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);

    ssi_sprint (starget, "%s/%s{%s,%g}", config.sourceName.str (), prefix, config.triggerName.str(), thres);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str ());
	
	if(ssi_mkdir (sstring)){
		ssi_print("\ncreated directory %s\n", sstring);
	}else{
		ssi_print("\ncould not create directory %s\n", sstring);
	}

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

        ssi_sprint (sstring, "%s/%s/%s/%s.samples", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!config.reCalculate && ssi_exists (sstring)) {		
			ssi_print ("...skip\n");
			continue;
		}

		SampleList samples;
		ssi_stream_t stream;

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		if (!ModelTools::LoadSampleList (samples, sstring)) {
			return false;
		}

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), trigger, session);
		if (!FileTools::ReadStreamFile (sstring, stream)) {
			return false;
		}		
		
		ISTrigger samples_t (&samples);
		samples_t.setTriggerStream (stream_index, stream, thres);

		ISMissingData samples_m (&samples_t);
		samples_m.setStream (stream_index);

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.targetName.str (), session);
		if (!ModelTools::SaveSampleList(samples_m, sstring, File::BINARY)) {
			return false;
		}

		ModelTools::PrintInfo (samples_t, ssiout);

		ssi_stream_destroy (stream);
	}

	return true;
}


bool Machine::Collect (Config &config, SampleList &samples) {

	ssi_char_t sstring[SSI_MAX_CHAR];

	ssi_print ("\nCOLLECT > %s\n\n", config.listName.str ());

    ssi_sprint (sstring, "%s/%s", config.rootFolder.str (), config.listName.str ());
	StringList sessions;
	FileTools::ReadFilesFromFile (sessions, sstring);	

	for (ssi_size_t i = 0; i < sessions.size (); i++) {

		const ssi_char_t *session = sessions.get (i);		
		ssi_print ("session > %s\n", session);		

        ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), session);
		if (!ModelTools::LoadSampleList (samples, sstring)) {
			return false;
		}

	}

	ModelTools::PrintInfo (samples, ssiout);

	return true;
}

bool Machine::Train (Config &config, ISamples &samples, const ssi_char_t *prefix, IModel *model, Machine::ResampleType::List type) {
	
	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nTRAIN > %s + %s\n\n", model->getName (), ResampleTypeNames[type]);
    ssi_sprint (starget, "%s/%s/%s/%s{%s}", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), prefix, ResampleTypeNames[type]);

	if(ssi_mkdir (starget)){
		ssi_print("\ncreated directory %s\n", starget);
	}else{
		ssi_print("\ncould not create directory %s\n", starget);
	}

	config.targetName = starget;

	ssi_print ("...samples\n");
	ModelTools::PrintInfo (samples, ssiout);

    ssi_sprint (sstring, "%s/class.trainer", config.targetName.str ());
	if (!config.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		return true;
	}

	ISamples *s_train = 0;
	ISOverSample s_over (&samples);	
	ISUnderSample s_under (&samples);

	switch (type) {
		case ResampleType::NONE:
			s_train = &samples;
			break;
		case ResampleType::UNDER:
			ssi_print ("...apply under sampling\n");
			if (!s_under.setUnder (ISUnderSample::RANDOM)) {
				return false;
			}
			s_train = &s_under;			
			ModelTools::PrintInfo (*s_train, ssiout);
			break;
		case ResampleType::OVER:
			ssi_print ("...apply over sampling\n");
			if (!s_over.setOver (ISOverSample::SMOTE)) {
				return false;
			}
			s_train = &s_over;
			ModelTools::PrintInfo (*s_train, ssiout);
			break;
	}	

	Trainer trainer (model);

	if (config.featSel) {
		FloatingSearch *fsearch = ssi_create(FloatingSearch, 0, true);
		fsearch->getOptions()->nfirst = config.nBest;
		trainer.setSelection(*s_train, fsearch);
	}
	if (!trainer.train (*s_train)) {
		return false;
	}

    ssi_sprint (sstring, "%s/class", config.targetName.str ());
	ssi_print ("save > '%s' \n", sstring);
	if (!trainer.save (sstring)) {
		return false;
	}
	ModelTools::SaveSampleList (*s_train, sstring, File::BINARY);

	return true;
}

bool Machine::TrainFusion (Config &config1, Config &config2, ISamples &samples, const ssi_char_t *prefix, IFusion *fusion, ssi_size_t n_models, IModel **models, ResampleType::List type) {
	
	ssi_char_t sstring[SSI_MAX_CHAR];
	ssi_char_t starget[SSI_MAX_CHAR];

	ssi_print ("\nTRAIN > %s + %s\n\n", fusion->getName (), ResampleTypeNames[type]);
    ssi_sprint (starget, "%s/%s/%s_%s_%s{%s}", config1.rootFolder.str(), config1.transformFolderName.str(), config1.signalName.str (), config2.signalName.str (), prefix, ResampleTypeNames[type]);

	if(ssi_mkdir (starget)){
		ssi_print("\ncreated directory %s\n", starget);
	}else{
		ssi_print("\ncould not create directory %s\n", starget);
	}

	config1.targetName = starget;
	config2.targetName = starget;

	ssi_print ("...samples\n");
	ModelTools::PrintInfo (samples, ssiout);

    ssi_sprint (sstring, "%s/class.trainer", starget);
	if (!config1.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		return true;
	}

	ISamples *s_train = 0;
	ISOverSample s_over (&samples);	
	ISUnderSample s_under (&samples);

	switch (type) {
		case ResampleType::NONE:
			s_train = &samples;
			break;
		case ResampleType::UNDER:
			ssi_print ("...apply under sampling\n");
			if (!s_under.setUnder (ISUnderSample::RANDOM)) {
				return false;
			}
			s_train = &s_under;			
			ModelTools::PrintInfo (*s_train, ssiout);
			break;
		case ResampleType::OVER:
			ssi_print ("...apply over sampling\n");
			if (!s_over.setOver (ISOverSample::SMOTE)) {
				return false;
			}
			s_train = &s_over;
			ModelTools::PrintInfo (*s_train, ssiout);
			break;
	}	

	Trainer trainer (n_models, models, fusion);
	if (!trainer.train (*s_train)) {
		return false;
	}

    ssi_sprint (sstring, "%s/class", starget);
	ssi_print ("save > '%s' \n", sstring);
	if (!trainer.save (sstring)) {
		return false;
	}
	
	ModelTools::SaveSampleList (*s_train, sstring, File::BINARY);

	return true;
}

bool Machine::Eval (Machine::Config &config, ISamples &samples) {

	ssi_char_t sstring[SSI_MAX_CHAR];
	
	ssi_print ("\nEVAL\n\n");

	ssi_print ("...samples\n");
	ModelTools::PrintInfo (samples, ssiout);

	ssi_print ("...evaluation\n");
    ssi_sprint (sstring, "%s/eval.txt", config.sourceName.str ());
	if (!config.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		FILE *fp = ssi_fopen(sstring, "r");
		char c;
		while ((c = getc (fp)) != EOF) {
			ssi_print ("%c", c);
		}
		fclose (fp);
		return true;
	}

    ssi_sprint (sstring, "%s/class", config.sourceName.str ());
	ssi_print ("load > '%s'\n", sstring);
	Trainer trainer;
	if (!trainer.Load (trainer, sstring)) {
		return false;
	}

	Evaluation eval;	
	eval.eval (&trainer, samples);
	ssi_size_t n_result;
	const ssi_size_t *result = eval.get_result_vec (n_result);
    ssi_sprint (sstring, "%s/decisions.txt", config.sourceName.str ());
	FILE *fp = ssi_fopen(sstring, "w");
	for (ssi_size_t i = 0; i < n_result; i++) {
		fprintf (fp, "%u %u\n", result[i*2], result[i*2+1]);
	}
	fclose (fp);

	eval.print ();
    ssi_sprint (sstring, "%s/eval.txt", config.sourceName.str ());
	fp = ssi_fopen(sstring, "w");
	eval.print (fp);
	fclose (fp);

	return true;
}

bool Machine::SaveEventList (const ssi_char_t *path, ssi_size_t n_events, ssi_event_t *events, ssi_size_t n_classes, ssi_size_t *class_ids, ssi_size_t sender_id, ssi_size_t event_id) {

	FILE *fp = ssi_fopen(path, "wb");

	if (!fp) {
		ssi_wrn ("cannot create file '%s'", path);
		return false;
	}

	const ssi_char_t *name = 0;
	ssi_size_t len = 0;

	fwrite (&n_events, sizeof (n_events), 1, fp);
	fwrite (&n_classes, sizeof (n_events), 1, fp);

	for (ssi_size_t i = 0; i < n_classes; i++) {
		name = Factory::GetString (class_ids[i]);
		len = ssi_cast (ssi_size_t, strlen (name));
		fwrite (&len, sizeof (len), 1, fp);
		fwrite (name, sizeof (ssi_char_t), len, fp);
	}

	name = Factory::GetString (sender_id);
	len = ssi_cast (ssi_size_t, strlen (name));
	fwrite (&len, sizeof (len), 1, fp);
	fwrite (name, sizeof (ssi_char_t), len, fp);

	name = Factory::GetString (event_id);
	len = ssi_cast (ssi_size_t, strlen (name));
	fwrite (&len, sizeof (len), 1, fp);
	fwrite (name, sizeof (ssi_char_t), len, fp);

	for (ssi_size_t i = 0; i < n_events; i++) {
		fwrite (&events[i], sizeof (ssi_event_t), 1, fp);
		if (events[i].tot_real > 0) {
			fwrite (events[i].ptr, sizeof (ssi_byte_t), events[i].tot, fp);
		}
	}

	fclose (fp);

	return true;
}

bool Machine::LoadEventList (const ssi_char_t *path, ssi_size_t &n_events, ssi_event_t **events) {

	FILE *fp = ssi_fopen(path, "rb");

	if (!fp) {
		ssi_wrn ("cannot open file '%s'", path);
		return false;
	}	

	ssi_size_t len = 0;
	ssi_char_t *name = 0;

	fread (&n_events, sizeof (n_events), 1, fp);
	*events = new ssi_event_t[n_events];
	
	ssi_size_t n_classes;
	fread (&n_classes, sizeof (n_events), 1, fp);
	ssi_size_t *class_ids = new ssi_size_t[n_classes];	
	for (ssi_size_t i = 0; i < n_classes; i++) {				
		fread (&len, sizeof (len), 1, fp);
		name = new ssi_char_t[len+1];
		fread (name, sizeof (ssi_char_t), len, fp);
		name[len] = '\0';
		class_ids[i] = Factory::AddString (name);
		delete[] name;
	}

	ssi_size_t sender_id;
	fread (&len, sizeof (len), 1, fp);
	name = new ssi_char_t[len+1];
	fread (name, sizeof (ssi_char_t), len, fp);
	name[len] = '\0';
	sender_id = Factory::AddString (name);
	delete[] name;

	ssi_size_t event_id;
	fread (&len, sizeof (len), 1, fp);
	name = new ssi_char_t[len+1];
	fread (name, sizeof (ssi_char_t), len, fp);
	name[len] = '\0';
	event_id = Factory::AddString (name);
	delete[] name;

	for (ssi_size_t i = 0; i < n_events; i++) {
		fread (&(*events)[i], sizeof (ssi_event_t), 1, fp);
		(*events)[i].sender_id = sender_id;
		(*events)[i].event_id = event_id;		
		if ((*events)[i].tot_real > 0) {			
			(*events)[i].ptr = new ssi_byte_t[(*events)[i].tot_real];
			fread ((*events)[i].ptr, sizeof (ssi_byte_t), (*events)[i].tot_real, fp);
			ssi_event_map_t *t = ssi_pcast (ssi_event_map_t, (*events)[i].ptr);
			for (ssi_size_t i = 0; i < n_classes; i++) {
				t[i].id = class_ids[i];				
			}
		}
	}

	fclose (fp);

	return true;
}

bool Machine::EventListToStream (Config &config, ssi_time_t sr, ssi_size_t dim, ssi_real_t missing_value) {

	ssi_char_t sstring[SSI_MAX_CHAR];	
	ssi_char_t starget[SSI_MAX_CHAR];
	
	ssi_print ("\nEVENT LIST TO STREAM\n\n");
	ssi_sprint (starget, "%s", config.sourceName.str ());
	config.targetName = starget;

    ssi_sprint (sstring, "%s/%s/%s.stream", config.rootFolder.str (), config.eventFolderName.str (), config.targetName.str ());
	if (!config.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		return true;
	}

	ssi_size_t n_events = 0;
	ssi_event_t *events = 0;	

    ssi_sprint (sstring, "%s/%s/%s.events", config.rootFolder.str (), config.eventFolderName.str (), config.sourceName.str ());
	if (!LoadEventList (sstring, n_events, &events)) {
		return false;
	}
	
	ssi_stream_t stream;
	ssi_stream_init (stream, n_events, dim, sizeof (ssi_real_t), SSI_REAL, sr);
	
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
	for (ssi_size_t i = 0; i < n_events; i++) {
		if (events[i].ptr) {
			ssi_event_map_t *e = ssi_pcast (ssi_event_map_t, events[i].ptr);
			for (ssi_size_t j = 0; j < dim; j++) {
				*ptr++ = e[j].value;
			}
		} else {
			for (ssi_size_t j = 0; j < dim; j++) {
				*ptr++ = missing_value;
			}
		}
	}
	
    ssi_sprint (sstring, "%s/%s/%s", config.rootFolder.str (), config.eventFolderName.str (), config.targetName.str ());
	ssi_print ("save > '%s' \n", sstring);
	if (!FileTools::WriteStreamFile (File::BINARY, sstring, stream)) {
		return false;
	}

	ssi_stream_destroy (stream);
	delete[] events;

	return true;
}

bool Machine::CreateEventList (Config &config, const ssi_char_t *prefix, ISamples &samples, const ssi_char_t *sender_name, const ssi_char_t *event_name, ssi_real_t boost) {

	ssi_char_t sstring[SSI_MAX_CHAR];	
	ssi_char_t starget[SSI_MAX_CHAR];
	
	ssi_print ("\nCREATE EVENTS\n\n");
	
    ssi_sprint (sstring, "%s/%s/%s/%s", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), config.eventFolderName.str());
	if(ssi_mkdir (sstring)){
		ssi_print("\ncreated directory %s\n", sstring);
	}else{
		ssi_print("\ncould not create directory %s\n", sstring);
	}

    ssi_sprint (sstring, "%s/%s/%s/%s/boost_%.3f", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str (), config.eventFolderName.str(), boost);
	if(ssi_mkdir (sstring)){
		ssi_print("\ncreated directory %s\n", sstring);
	}else{
		ssi_print("\ncould not create directory %s\n", sstring);
	}

	ssi_sprint (starget, "%s", sstring);
	config.targetName = starget;
    ssi_sprint (sstring, "%s/e.events", starget);
	if (!config.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		return true;
	}
	
	ssi_print ("...samples\n");
	ModelTools::PrintInfo (samples, ssiout);

    ssi_sprint (sstring, "%s/%s/%s/class.trainer", config.rootFolder.str (), config.transformFolderName.str (), config.sourceName.str ());
	Trainer trainer;
	Trainer::Load (trainer, sstring);

	ssi_size_t n_events = samples.getSize ();
	ssi_size_t n_classes = trainer.getClassSize ();
	ssi_size_t *class_ids = new ssi_size_t[n_classes];
	for (ssi_size_t i = 0; i < n_classes; i++) {
		class_ids[i] = Factory::AddString (trainer.getClassName (i));
	}

	ssi_size_t event_id = Factory::AddString (event_name);
	ssi_size_t sender_id = Factory::AddString (sender_name);

	ssi_event_t *events = new ssi_event_t[n_events];
	for (ssi_size_t i = 0; i < n_events; i++) {
		ssi_event_init (events[i], SSI_ETYPE_MAP, sender_id, event_id);		
	}	

	ssi_sample_t *sample = 0;
	ssi_real_t *probs = new ssi_real_t[n_classes];
	ssi_size_t count = 0;
	samples.reset ();

	while (sample = samples.next ()) {
		events[count].time = ssi_cast (ssi_size_t, sample->time * 1000.0 + 0.5);
		events[count].dur = 0;
		if (trainer.forward_probs (sample->num, sample->streams, n_classes, probs)) {

			ssi_real_t minval, maxval;
			ssi_size_t minpos, maxpos;
			if(config.discretize){				
				ssi_minmax(n_classes, 1, probs, &minval, &minpos, &maxval, &maxpos);
			}

			ssi_event_adjust (events[count], n_classes * sizeof (ssi_event_map_t));
			ssi_event_map_t *t = ssi_pcast (ssi_event_map_t, events[count].ptr);
			for (ssi_size_t i = 0; i < n_classes; i++) {
				t[i].id = class_ids[i];
				if(config.discretize){
					if(i == maxpos){
						t[i].value = 1.0f;
					}else{
						t[i].value = 0.0f;
					}
				}else{
					t[i].value = probs[i] * boost;
				}
			}
		}
		count++;
	}

    ssi_sprint (sstring, "%s/e.events", starget);
	ssi_print ("save > '%s' \n", sstring);
	if (!SaveEventList (sstring, n_events, events, n_classes, class_ids, sender_id, event_id)) {
		return false;
	}

	for (ssi_size_t i = 0; i < n_events; i++) {
		ssi_event_destroy (events[i]);
	}
	delete[] events;

	return true;
}


bool Machine::EventFusion(Config &config, const ssi_char_t *prefix, IObject *fusion, IEventListener *fwriter, ssi_size_t delta_ms, ssi_size_t offset_ms, ssi_size_t sleep_ms) {
	
	ssi_char_t sstring[SSI_MAX_CHAR];	
	ssi_char_t starget[SSI_MAX_CHAR];	

	ssi_print ("\nEVENT FUSION (1)\n\n");	
	ssi_sprint (starget, "%s", prefix);
	config.targetName = starget;

    ssi_sprint (sstring, "%s/%s/%s/%s.stream", config.rootFolder.str (), config.eventFolderName.str (), config.fusionName.str(), config.targetName.str ());
	if (!config.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		return true;
	}

	ssi_size_t n_events = 0;
	ssi_event_t *events = 0;	

    ssi_sprint (sstring, "%s/e.events", config.sourceName.str ());
	if (!LoadEventList (sstring, n_events, &events)) {
		return false;
	}

	//if (!framework_is_registered) {
	//	ssi::Factory::Register(TheFakedFramework::GetCreateName(), TheFakedFramework::Create);
	//	framework_is_registered = true;
	//}
	//TheFakedFramework *frame = ssi_pcast (TheFakedFramework, Factory::GetFramework ());		

	EventList list (100);
	ssi_size_t time_ms = 0;

	if (fwriter) {
		fusion->setEventListener(fwriter);
	}

	ssi_print ("...simulation\n");
	fusion->listen_enter ();
	fwriter->listen_enter ();
	for (ssi_size_t i = 0; i < n_events; i++) {

#if 0
		if (events[i].ptr) {
			ssi_event_tuple_t *t = ssi_pcast (ssi_event_tuple_t, events[i].ptr);
			ssi_print ("%d %u %.2f %.2f\n", i, events[i].time, t[0].value, t[1].value);
		} else {
			ssi_print ("%d %u MISSING\n", i, events[i].time);
		}
#endif

		list.clear ();
		if (events[i].ptr) {
			events[i].time = time_ms;
			list.push (events[i]);
		}

		time_ms += delta_ms;
		//frame->SetElapsedTimeMs (time_ms + offset_ms);	

		fusion->update (list, list.getSize (), time_ms + offset_ms);		

		if (sleep_ms > 0) {
            ssi_sleep (sleep_ms);
		}

	}
	fusion->listen_flush ();
	fwriter->listen_flush ();

	for (ssi_size_t i = 0; i < n_events; i++) {
		ssi_event_destroy (events[i]);
	}
	delete[] events;

	return true;
}

bool Machine::EventFusion(Config &config1, Config &config2, const ssi_char_t *prefix, IObject *fusion, IEventListener *fwriter, ssi_size_t delta_ms, ssi_size_t offset_ms, ssi_size_t sleep_ms) {
	
	ssi_char_t sstring[SSI_MAX_CHAR];	
	ssi_char_t starget[SSI_MAX_CHAR];	

	ssi_print ("\nEVENT FUSION (2)\n\n");	
	ssi_sprint (starget, "%s", prefix);
	config1.targetName = starget;
	config2.targetName = starget;

    ssi_sprint (sstring, "%s/%s/%s.stream", config1.rootFolder.str (), config1.eventFolderName.str (), config1.targetName.str ());
	if (!config1.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		return true;
	}

	ssi_print ("...events\n");

	ssi_size_t n_events1 = 0;
	ssi_event_t *events1 = 0;	

    ssi_sprint (sstring, "%s/e.events", config1.sourceName.str ());
	if (!LoadEventList (sstring, n_events1, &events1)) {
		return false;
	}

	ssi_size_t n_events2 = 0;
	ssi_event_t *events2 = 0;	

    ssi_sprint (sstring, "%s/e.events", config2.sourceName.str ());
	if (!LoadEventList (sstring, n_events2, &events2)) {
		return false;
	}

	ssi_size_t n_events = min (n_events1, n_events2);

	//if (!framework_is_registered) {
	//	ssi::Factory::Register(TheFakedFramework::GetCreateName(), TheFakedFramework::Create);
	//	framework_is_registered = true;
	//}
	//TheFakedFramework *frame = ssi_pcast (TheFakedFramework, Factory::GetFramework ());		

	EventList list (100);	
	ssi_size_t time_ms = 0;

	if (fwriter) {
		fusion->setEventListener(fwriter);
	}

	ssi_print ("...simulation\n");
	fusion->listen_enter ();
	fwriter->listen_enter ();
	for (ssi_size_t i = 0; i < n_events; i++) {

#if 0
		if (events1[i].ptr) {
			ssi_event_tuple_t *t = ssi_pcast (ssi_event_tuple_t, events1[i].ptr);
			ssi_print ("%d %u %.2f %.2f ", i, events1[i].time, t[0].value, t[1].value);
		} else {
			ssi_print ("%d %u MISSING ", i, events1[i].time);
		}

		if (events2[i].ptr) {
			ssi_event_tuple_t *t = ssi_pcast (ssi_event_tuple_t, events2[i].ptr);
			ssi_print ("%.2f %.2f\n", t[0].value, t[1].value);
		} else {
			ssi_print ("MISSING\n");
		}
#endif

		list.clear ();
		if (events1[i].ptr) {
			events1[i].time = time_ms;
			list.push (events1[i]);
		}
		if (events2[i].ptr) {

			events2[i].time = time_ms;
			list.push (events2[i]);
		}			

		time_ms += delta_ms;
		//frame->SetElapsedTimeMs (time_ms + offset_ms);	

		fusion->update (list, list.getSize (), time_ms + offset_ms);		

		if (sleep_ms > 0) {
            ssi_sleep (sleep_ms);
		}

	}
	fusion->listen_flush ();
	fwriter->listen_flush ();

	for (ssi_size_t i = 0; i < n_events1; i++) {
		ssi_event_destroy (events1[i]);
	}
	delete[] events1;

	for (ssi_size_t i = 0; i < n_events2; i++) {
		ssi_event_destroy (events2[i]);
	}
	delete[] events2;

	return true;
}

ssi_real_t Machine::get_class_prob (ssi_size_t confmat[2][2], ssi_size_t index) {

	ssi_size_t sum = 0;
	for (ssi_size_t i = 0; i < 2; ++i) {
		sum += confmat[index][i];
	}
	ssi_real_t prob = sum > 0 ? ssi_cast (ssi_real_t, confmat[index][index]) / ssi_cast (ssi_real_t, sum) : 0; 

	return prob;
}

ssi_real_t Machine::get_classwise_prob (ssi_size_t confmat[2][2]) {

	ssi_real_t prob = 0;
	for (ssi_size_t i = 0; i < 2; ++i) {
		prob += get_class_prob (confmat, i);
	}

	return prob / 2;
}

ssi_real_t Machine::get_accuracy_prob (ssi_size_t confmat[2][2], ssi_size_t n_samples) {

	ssi_size_t sum_correct = 0;
	for (ssi_size_t i = 0; i < 2; ++i) {
		sum_correct += confmat[i][i];
	}

	ssi_real_t prob = ssi_cast (ssi_real_t, sum_correct) / ssi_cast (ssi_real_t, n_samples); 

	return prob;
}

bool Machine::EventFusionEval (Config &config, const ssi_char_t *prefix, ssi_real_t thres, ISamples &samples, ssi_real_t &classwise, ssi_real_t &accuracy) {

	ssi_char_t sstring[SSI_MAX_CHAR];	
	ssi_char_t starget[SSI_MAX_CHAR];	
	ssi_print ("\nEVALUATION\n\n");	

	ssi_sprint (starget, "%s", prefix);
	config.targetName = starget;	

	ssi_print ("...evaluation\n");
    /*ssi_sprint (sstring, "%s/%s/%s.txt", config.rootFolder.str (), config.eventFolderName.str (), starget);
	if (!config.reCalculate && ssi_exists (sstring)) {	
		ssi_print ("...skip\n");
		FILE *fp = ssi_fopen (sstring, "r");
		char c;
		while ((c = getc (fp)) != EOF) {
			ssi_print ("%c", c);
		}
		fclose (fp);
		return true;
	}*/

    ssi_sprint (sstring, "%s/%s/%s/%s.stream", config.rootFolder.str (), config.eventFolderName.str (), config.fusionName.str(), config.sourceName.str ());
	ssi_stream_t result;
	FileTools::ReadStreamFile (sstring, result);

	ssi_size_t n_samples = min (samples.getSize (), result.num);
	ssi_size_t n_classes = samples.getClassSize ();
	ssi_size_t *truth = new ssi_size_t[n_samples];
	samples.reset ();
	ssi_sample_t *s = 0;
	for (ssi_size_t i = 0; i < n_samples; i++) {
		s = samples.next ();
		truth[i] = s->class_id;
	}

	ssi_size_t confmat[2][2] = {0};	
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, result.ptr);
	ssi_size_t pred = 0;
	ssi_size_t *guess = new ssi_size_t[n_samples];
	for (ssi_size_t i = 0; i < n_samples; i++) {
		if (thres > 0) {
			pred = *ptr >= thres ? 0 : 1;
		} else {
			pred = *(ptr) >= *(ptr+1) ? 0 : 1;
		}
		guess[i] = pred;
		confmat[truth[i]][pred]++;		
		ptr+=2;
	}

	ssi_size_t max_label_len = 0;
	for (ssi_size_t i = 0; i < n_classes; ++i) {
		ssi_size_t len = ssi_cast (ssi_size_t, strlen (samples.getClassName (i)));
		if (len > max_label_len) {
			max_label_len = len;
		}
	}

    ssi_sprint (sstring, "%s/%s/%s/%s_decisions.txt", config.rootFolder.str (), config.eventFolderName.str (), config.fusionName.str(), starget);
	FILE *fp = ssi_fopen(sstring, "w");
	for (ssi_size_t i = 0; i < n_samples; i++) {
		fprintf (fp, "%u %u\n", truth[i], guess[i]);
	}
	fclose (fp);

    ssi_sprint (sstring, "%s/%s/%s/%s.txt", config.rootFolder.str (), config.eventFolderName.str (), config.fusionName.str(), starget);
	File *tmp = File::CreateAndOpen (File::ASCII, File::WRITE, sstring);
	FILE *file = tmp->getFile ();
	tmp->setType (SSI_UINT);
	tmp->setFormat (" ", "6");		
	ssi_fprint (file, "#classes:      %u\n", n_classes);
	ssi_fprint (file, "#total:        %u\n", n_samples);
	ssi_fprint (file, "#classified:   %u\n", n_samples);
	ssi_fprint (file, "#unclassified: %u\n", 0);
	for (ssi_size_t i = 0; i < n_classes; ++i) {			
		ssi_fprint (file, "%*s: ", max_label_len, samples.getClassName (i));
		tmp->write (confmat[i], 0, n_classes);
		ssi_fprint (file, "   -> %8.2f%%\n", 100*get_class_prob (confmat, i));
	}
	ssi_fprint (file, "   %*s  => %8.2f%% | %.2f%%\n", max_label_len + n_classes * 7, "", 100*get_classwise_prob (confmat), 100*get_accuracy_prob (confmat, n_samples));	
	classwise = get_classwise_prob (confmat);
	accuracy = get_accuracy_prob (confmat, n_samples);
	delete tmp;

	fflush (file);

	fp = ssi_fopen(sstring, "r");
	char c;
	while ((c = getc (fp)) != EOF) {
		ssi_print ("%c", c);
	}
	fclose (fp);

	delete[] truth;
	delete[] guess;

	return true;
}

/* Event List Class */

Machine::EventList::EventList(ssi_size_t n_events)
	: _n_events(n_events),
	_events_count(0),
	_next_count(0),
	_head_pos(0),
	_next_pos(0),
	_next(0) {

	_events = new ssi_event_t[_n_events];
	for (ssi_size_t i = 0; i < _n_events; i++) {
		ssi_event_init(_events[i]);
	}

	reset();
}

Machine::EventList::~EventList() {

	clear();
	delete[] _events;
}

void Machine::EventList::reset() {

	_next_pos = _head_pos == 0 ? _n_events - 1 : _head_pos - 1;
	_next = _events + _next_pos;
	_next_count = _events_count;
}

ssi_event_t *Machine::EventList::get(ssi_size_t index) {

	if (index >= _n_events) {
		ssi_wrn("index '%u' exceeds #events '%u'", index, _n_events);
		return 0;
	}

	if (index <= _head_pos) {
		return _events + (_head_pos - index);
	}
	else {
		return _events + (_n_events - (index - _head_pos));
	}
}

void Machine::EventList::push(ssi_event_t &e) {

	ssi_event_copy(e, _events[_head_pos]);
	if (_events_count < _n_events) {
		++_events_count;
	}
	++_head_pos %= _n_events;
}

ssi_event_t *Machine::EventList::next() {

	if (_next_count-- == 0) {
		return 0;
	}

	ssi_event_t *next = _next;

	if (_next_pos == 0) {
		_next_pos = _n_events - 1;
		_next = _events + _next_pos;
	}
	else {
		--_next_pos;
		--_next;
	}

	return next;
}

ssi_size_t Machine::EventList::getSize() {

	return _events_count;
}

void Machine::EventList::clear() {
	for (ssi_size_t i = 0; i < _n_events; i++) {
		ssi_event_destroy(_events[i]);
	}
	_head_pos = 0;
	_events_count = 0;
	reset();
}


}
