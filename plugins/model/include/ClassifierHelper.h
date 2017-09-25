// ClassifierHelper.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/07/17
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

#ifndef SSI_MODEL_CLASSIFIERHELPER_H
#define SSI_MODEL_CLASSIFIERHELPER_H

#include "SSI_Cons.h"
#include "base/StringList.h"
#include "thread/Lock.h"

namespace ssi
{

	class Trainer;

	class ClassifierHelper
	{

	public:

		class Prediction
		{
		public:

			Prediction();
			virtual ~Prediction();

			Trainer *trainer;
			ssi_size_t n_probabilites;
			ssi_real_t *probabilites;
			ssi_real_t confidence;
			ssi_size_t winner;
		};

	public:

		ClassifierHelper();
		virtual ~ClassifierHelper();

		void release();

		bool hasTrainer();		
		bool addTrainer(const ssi_char_t *name, Trainer *trainer);
		bool addTrainerFromString(const ssi_char_t *string);
		bool addTrainerFromPath(const ssi_char_t *name, const ssi_char_t *path);
		bool switchTrainer(const ssi_char_t *name);		
		bool switchTrainer(ssi_event_t *event);

		bool predict(Prediction &prediction,
			ssi_time_t time,
			ssi_time_t dur,
			ssi_size_t n_streams,
			ssi_stream_t stream_in[],
			ssi_real_t pthres);

	protected:		

        static ssi_char_t ssi_log_name[];

		struct TrainerWrapper
		{
			Trainer *trainer;
			bool release;
		};

		class TrainerMap : public std::map<String, TrainerWrapper> {};

		Mutex _mutex;
		TrainerMap _trainerMap;
		String _selectedName;

		Trainer *getTrainer();
		bool addTrainer(const ssi_char_t *name, Trainer *trainer, bool release);
		bool parseNamePath(const ssi_char_t *string, StringList &names, StringList &paths);
	};

}

#endif
