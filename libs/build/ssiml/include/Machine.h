// Machine.h
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

#pragma once

#ifndef SSI_MODEL_MACHINE_H
#define SSI_MODEL_MACHINE_H

#include "SSI_Cons.h"
#include "base/String.h"
#include "base/ITransformer.h"
#include "base/IModel.h"
#include "model/SampleList.h"
#include "base/IFusion.h"
#include "model/include/FloatingSearch.h"

namespace ssi {

class Machine {                                                                

public:

	struct Config {
		bool reCalculate;
		String rootFolder;
		String rawFolderName;
		String transformFolderName;
		String eventFolderName;
		bool doSnippets;
		String snippetFolderName;
		String signalName;	
		bool isAudio;
		String annoName;
		String listName;
		String sourceName;
		String targetName;
		String triggerName;
		String fusionName;
		bool discretize;
		bool featSel;
		ssi_size_t nBest;
	};

	struct ResampleType {
		enum List {
			NONE = 0,
			UNDER,
			OVER
		};
	};

	static const ssi_char_t *ResampleTypeNames[3];

public:

	static bool Init (Config &config);
	static bool Transform (Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, ITransformer *theTransformer);
	static bool SamplesFromSingleTier (Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, const ssi_char_t *tierName, bool includeInverted, bool pack, bool useTierNamesAsLabels); //pack && useTierNamesAsLabels == true in earlier version
	static bool SamplesFromMultipleTiers(Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, ssi_size_t n_tier, const ssi_char_t *tierNames[], bool pack, bool useTierNamesAsLabels); //pack && useTierNamesAsLabels == true in earlier version
	static bool SamplesSelectWithinTier (Config &config, const ssi_char_t *prefix, const ssi_char_t *tierName);
	static bool SamplesFromStream (Config &config, const ssi_char_t *prefix, const ssi_char_t *className, const ssi_char_t *userName);
	static bool Trigger (Config &config, const ssi_char_t *prefix, const ssi_char_t *trigger, ssi_size_t stream_index, ssi_real_t thres);
	static bool Collect (Config &config, SampleList &samples);
	static bool PrepareBayesNet (Config &config, const ssi_char_t *prefix, ssi_size_t frameSize, ssi_size_t deltaSize, const ssi_char_t *tierName);
	static bool Train (Config &config, ISamples &samples, const ssi_char_t *prefix, IModel *model, ResampleType::List type);
	static bool TrainFusion (Config &config1, Config &config2, ISamples &samples, const ssi_char_t *prefix, IFusion *fusion, ssi_size_t n_models, IModel **models, ResampleType::List type);
	static bool Eval(Config &config, ISamples &samples);
	static bool CreateEventList (Config &config, const ssi_char_t *prefix, ISamples &samples, const ssi_char_t *sender_name, const ssi_char_t *event_name, ssi_real_t boost);
	static bool EventListToStream (Config &config, ssi_time_t sr, ssi_size_t dim, ssi_real_t missing_value);
	static bool SaveEventList (const ssi_char_t *path, ssi_size_t n_events, ssi_event_t *events, ssi_size_t n_classes, ssi_size_t *class_ids, ssi_size_t sender_id, ssi_size_t event_id);
	static bool LoadEventList (const ssi_char_t *path, ssi_size_t &n_events, ssi_event_t **events);
	static bool EventFusion(Config &config, const ssi_char_t *prefix, IObject *fusion, IEventListener *fwriter, ssi_size_t delta_ms, ssi_size_t offset_ms, ssi_size_t sleep_ms);
	static bool EventFusion(Config &config1, Config &config2, const ssi_char_t *prefix, IObject *fusion, IEventListener *fwriter, ssi_size_t delta_ms, ssi_size_t offset_ms, ssi_size_t sleep_ms);
	static bool EventFusionEval (Config &config, const ssi_char_t *prefix, ssi_real_t thres, ISamples &samples, ssi_real_t &classwise, ssi_real_t &accuracy);
	
protected:

	static ssi_real_t get_class_prob (ssi_size_t confmat[2][2], ssi_size_t index);
	static ssi_real_t get_classwise_prob (ssi_size_t confmat[2][2]);
	static ssi_real_t get_accuracy_prob (ssi_size_t confmat[2][2], ssi_size_t n_samples);
	static bool framework_is_registered;

public:

	class EventList : public IEvents {

	public:

		EventList(ssi_size_t n_events);
		virtual ~EventList();

		void reset();
		ssi_event_t *get(ssi_size_t index);
		ssi_event_t *next(); // returns next element and moves iterator one back
		ssi_size_t getSize();

		virtual void push(ssi_event_t &e);
		virtual void clear();

	protected:

		ssi_event_t *_events;
		ssi_size_t _n_events;
		ssi_size_t _events_count;
		ssi_size_t _next_count;
		ssi_size_t _head_pos;
		ssi_event_t *_next;
		ssi_size_t _next_pos;
	};

};

}

#endif
