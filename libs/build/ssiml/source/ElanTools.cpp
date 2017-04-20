// ElanTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#include "ElanTools.h"
#include <vector>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

ssi_char_t *ElanTools::ssi_log_name = "elantools_";

void ElanTools::LoadSampleList(SampleList &samples,
	ssi_stream_t &stream,
	ElanTier &elanTier,
	const ssi_char_t *user_name,
	bool useTierNameAsLabel) {

	ssi_stream_t *s = &stream;
	ElanTools::LoadSampleList(samples, 1, &s, elanTier, user_name, useTierNameAsLabel);
}

void ElanTools::LoadSampleList(SampleList &samples,
	ssi_size_t num,
	ssi_stream_t *streams[],
	ElanTier &elanTier,
	const ssi_char_t *user_name,
	bool useTierNameAsLabel) {

	// add user name
	ssi_size_t user_id = samples.addUserName(user_name);

	// add labels
	ssi_size_t class_id;
	if (useTierNameAsLabel) {
		class_id = samples.addClassName(elanTier.name());
	}

	// add samples
	ElanTier::iterator anno;
	for (anno = elanTier.begin(); anno != elanTier.end(); anno++) {

		ssi_sample_t *sample = new ssi_sample_t;
		ssi_stream_t **chops = new ssi_stream_t *[num];

		bool success = false;
		for (ssi_size_t j = 0; j < num; j++) {

			// calculate start and stop index
			ssi_size_t start_index = ssi_cast(ssi_size_t, (anno->from / 1000.0) * streams[j]->sr + 0.5);
			ssi_size_t stop_index = ssi_cast(ssi_size_t, (anno->to / 1000.0) * streams[j]->sr + 0.5);

			if (!(start_index <= stop_index && stop_index < streams[j]->num)) {
				ssi_wrn("invalid interval [%lf..%lf]s", anno->from / 1000.0, anno->to / 1000.0);
				continue;
			}

			// extract sample			
			chops[j] = new ssi_stream_t;
			ssi_stream_copy(*streams[j], *chops[j], start_index, stop_index);

			success = true;
		}

		if (success) {

			// create and add new sample
			if (useTierNameAsLabel) {
				sample->class_id = class_id;
			}
			else {
				sample->class_id = samples.addClassName(anno->value.str());
			}
			sample->num = num;
			sample->score = 0.0f;
			sample->streams = chops;
			sample->time = anno->from / 1000.0;
			sample->user_id = user_id;
			samples.addSample(sample);

		}
		else {
			delete sample;
			delete[] chops;
		}

	}
}

//helper function for boolOps
bool ElanTools::ElanTier_within(ElanTier* elanTier, int time)
{
	bool returnValue = false;

	for (size_t i = 0; i < elanTier->size(); i++)
	{
		if ((time >= (int)elanTier->at(i).from) && (time <= (int)elanTier->at(i).to))
		{
			returnValue = true;
			return returnValue;
		}
	}

	return returnValue;
}


//slow boolean operations on elan annotation tracks
void ElanTools::AddTrack_boolOp(ElanDocument* elanDoc, char* tier0, char* tier1, char *newTier, AnnoBoolOp boolOp)
{
	bool _and = false;
	bool _or = false;
	bool _xor = false;
	bool _sub = false;

	if (boolOp == AND)
		_and = true;
	if (boolOp == OR)
		_or = true;
	if (boolOp == XOR)
		_xor = true;
	if (boolOp == SUB)
		_sub = true;

	(*elanDoc)[tier0].sort();
	(*elanDoc)[tier1].sort();

	//document length
	size_t len = ((*elanDoc)[tier0]).at((*elanDoc)[tier0].size() - 1).to;

	if (len < ((*elanDoc)[tier1]).at((*elanDoc)[tier1].size() - 1).to)
		len = ((*elanDoc)[tier1]).at((*elanDoc)[tier1].size() - 1).to;

	ElanTier myTier(newTier);
	ElanAnnotation elanAnno;
	bool annotationActive = false;
	bool anno0 = false;
	bool anno1 = false;

	//find relevant annotation points to speed up calculation
	std::vector<int> annoPoints;

	for (ElanDocument::iterator tierIter = elanDoc->begin(); tierIter != elanDoc->end(); tierIter++)
	{
		for (ElanTier::iterator annoIter = tierIter->begin(); annoIter != tierIter->end(); annoIter++)
		{
			annoPoints.push_back(annoIter->from);
			annoPoints.push_back(annoIter->to);
		}
	}
	std::sort(annoPoints.begin(), annoPoints.end());

	int i = 0;
	//brute force
	//for(int i=0; i <= len+1; i++)
	//faster
	for (int h = 0; h < annoPoints.size(); h++)
		for (int j = annoPoints.at(h) - 5; j < annoPoints.at(h) + 5; j++)
		{
			if (j >= 0 && j > i)
			{

				i = j;

				bool annoStateChanged = false;
				bool _anno0 = ElanTier_within(&(*elanDoc)[tier0], i);
				bool _anno1 = ElanTier_within(&(*elanDoc)[tier1], i);
				bool _anno = false;

				if (_and)
				{
					_anno = _anno0&&_anno1;
				}
				if (_or)
				{
					_anno = _anno0 || _anno1;

				}
				if (_xor)
				{
					_anno = (_anno0&&!_anno1) || (!_anno0&&_anno1);
				}
				if (_sub)
				{
					_anno = (_anno0&&!_anno1);
				}

				if ((!annotationActive &&_anno) || (annotationActive && !_anno))
					annoStateChanged = true;

				if (annoStateChanged)
				{
					if (_anno && !annotationActive)
					{
						//start new anno
						elanAnno.from = i;
					}
					else if (annotationActive && !_anno)
					{
						//finnish anno
						elanAnno.to = i;
						myTier.push_back(elanAnno);
					}
				}

				anno0 = _anno0;
				anno1 = _anno1;
				annotationActive = _anno;
			}
		}

	elanDoc->push_back(myTier);
}

//merge two elan documents into first
void ElanTools::Merge2first(ElanDocument &first, ElanDocument &second)
{

	ElanTier::iterator anno_in;
	ElanDocument::iterator  tier_in;

	for (tier_in = second.begin(); tier_in != second.end(); tier_in++) {

		ElanTier tier_new(tier_in->name());
		ElanAnnotation anno_new;

		for (anno_in = tier_in->begin(); anno_in != tier_in->end(); anno_in++) {

			anno_new.from = anno_in->from;
			anno_new.to = anno_in->to;
			anno_new.value = anno_in->value;

			tier_new.push_back(anno_new);

		}
		first.push_back(tier_new);
	}
}



// -> features per bool op
void ElanTools::Ssi2elan(const char* annopath, ElanDocument* elanDoc)
{
	old::Annotation anno;

	//ssi anno
	ModelTools::LoadAnnotation(anno, annopath);

	if (anno.size() == 0)return;

	//name of track?
	int labelSize = anno.labelSize();
	for (int label = 0; label < labelSize; label++)
	{
		anno.reset();
		ElanAnnotation a;
		old::Annotation::Entry* ae = anno.next(label);
		ElanTier myTier(anno.getLabel(label));

		anno.reset();

		for (ssi_size_t i = 0; i < anno.size() && ae; i++) {
			ae = anno.next(label);
			if (ae){

				a.from = ssi_sec2ms(ae->start);
				a.to = ssi_sec2ms(ae->stop);
				a.value = anno.getLabel(label);
				myTier.push_back(a);

			}
		}

		elanDoc->push_back(myTier);
	}
}


void ElanTools::Elan2ssi(ElanDocument* elanDoc, old::Annotation *ssiAnno)
{
	ElanDocument::iterator tier;
	old::Annotation::Entry ae;
	int labelIndex = 0;
	for (tier = elanDoc->begin(); tier != elanDoc->end(); tier++) {

		ElanTier::iterator anno;
		ssiAnno->addLabel(tier->name());
		for (anno = tier->begin(); anno != tier->end(); anno++) {

			ae.start = ((double)anno->from) / 1000.0;
			ae.stop = ((double)anno->to) / 1000.0;
			ae.label_index = labelIndex;
			ssiAnno->add(ae);
		}
		labelIndex++;
	}
}



}
