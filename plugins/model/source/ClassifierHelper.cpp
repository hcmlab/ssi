// ClassifierHelper.cpp
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

#include "ClassifierHelper.h"
#include "base/Factory.h"
#include "ssiml/include/Trainer.h"

namespace ssi
{
	ssi_char_t ClassifierHelper::ssi_log_name[] = "classifier";

	ClassifierHelper::ClassifierHelper()
	{
		release();
	}

	ClassifierHelper::~ClassifierHelper()
	{
		release();
	}

	ClassifierHelper::Prediction::Prediction()
		: trainer(0),
		winner(0),
		n_probabilites(0),
		probabilites(0),
		confidence(0)
	{
	}
	
	ClassifierHelper::Prediction::~Prediction()
	{
		delete[] probabilites;
	}

	void ClassifierHelper::release()
	{
		Lock lock(_mutex);

		for (TrainerMap::iterator it = _trainerMap.begin(); it != _trainerMap.end(); it++)
		{
			if (it->second.release)
			{
				delete it->second.trainer;
				it->second.trainer = 0;
			}
		}
		_trainerMap.clear();
	}

	bool ClassifierHelper::addTrainer(const ssi_char_t *name, Trainer *trainer, bool release)
	{	
		TrainerWrapper wrapper;
		wrapper.trainer = trainer;
		wrapper.release = release;

		if (trainer)
		{
			Lock lock(_mutex);

			ssi_msg(SSI_LOG_LEVEL_BASIC, "adding trainer '%s'", name);

			_trainerMap[String(name)] = wrapper;
			_selectedName = String(name);
		}

		return true;
	}

	bool ClassifierHelper::addTrainer(const ssi_char_t *name, Trainer *trainer)
	{
		return addTrainer(name, trainer, false);
	}	

	bool ClassifierHelper::addTrainerFromString(const ssi_char_t *string)
	{
		StringList names, paths;
		parseNamePath(string, names, paths);
		
		for (ssi_size_t i = 0; i < names.size(); i++)
		{
			if (!addTrainerFromPath(names[i].str(), paths[i].str()))
			{
				return false;
			}
		}

		return true;
	}

	bool ClassifierHelper::addTrainerFromPath(const ssi_char_t *name, const ssi_char_t *path)
	{	
		Trainer *trainer = new Trainer();
		if (!Trainer::Load(*trainer, path)) 
		{			
			return false;
		}

		addTrainer(name, trainer, true);
		
		return true;		
	}

	bool ClassifierHelper::switchTrainer(ssi_event_t *e)
	{
		switch (e->type)
		{

		case SSI_ETYPE_STRING:
		{
			const ssi_char_t *name = (const ssi_char_t *)e->ptr;
			return switchTrainer(name);
		}
		case SSI_ETYPE_MAP:
		{
			ssi_size_t n = e->tot / sizeof(ssi_event_map_t);
			if (n > 0)
			{
				ssi_event_map_t *tuple = (ssi_event_map_t *)e->ptr;
				ssi_size_t max_ind = 0;
				ssi_real_t max_val = tuple[0].value;
				for (ssi_size_t i = 0; i < n; i++)
				{
					if (max_val < tuple[i].value)
					{
						max_val = tuple[i].value;
						max_ind = i;
					}
				}

				const ssi_char_t *name = Factory::GetString(tuple[max_ind].id);
				return switchTrainer(name);
			}
		}
		}
		
		return false;
	}

	bool ClassifierHelper::switchTrainer(const ssi_char_t *name)
	{
		Lock lock(_mutex);

		TrainerMap::iterator it = _trainerMap.find(String(name));

		if (it == _trainerMap.end())
		{
			ssi_wrn("trainer not found '%s'", name);
			return false;
		}

		ssi_msg(SSI_LOG_LEVEL_BASIC, "switching trainer '%s > %s'", _selectedName.str(), name);

		_selectedName = it->first;				

		return true;

	}

	bool ClassifierHelper::predict(Prediction &prediction, ssi_time_t time, ssi_time_t dur, ssi_size_t n_streams, ssi_stream_t stream_in[], ssi_real_t pthres)
	{		
		Trainer *trainer = getTrainer();		

		if (!trainer)
		{
			return false;
		}	

		bool result = false;

		ssi_size_t n_probs = trainer->getClassSize();
		ssi_real_t *probs = new ssi_real_t[n_probs];
		
		ssi_stream_t **streams = new ssi_stream_t *[n_streams];
		for (ssi_size_t i = 0; i < n_streams; i++) {
			streams[i] = &stream_in[i];
		}
		result = trainer->forward_probs(n_streams, streams, n_probs, probs);
		delete[] streams;		

		if (result && pthres != 0) {
			bool exceeds_pthres = false;
			for (ssi_size_t i = 0; i < n_probs; i++) {
				if (probs[i] > pthres) {
					exceeds_pthres = true;
				}
			}
			result = exceeds_pthres;
		}

		if (result)
		{
			prediction.trainer = trainer;
			prediction.confidence = 1.0f;
			prediction.n_probabilites = n_probs;
			prediction.probabilites = probs;			

			ssi_size_t max_ind = 0;
			ssi_real_t max_val = probs[0];
			for (ssi_size_t i = 1; i < n_probs; i++) {
				if (probs[i] > max_val) {
					max_val = probs[i];
					max_ind = i;
				}
			}

			prediction.winner = max_ind;
		}
		else
		{
			delete[] probs;
		}

		return result;
	}

	bool ClassifierHelper::hasTrainer()
	{		
		Lock lock(_mutex);

		return _trainerMap.size() > 0;
	}

	Trainer *ClassifierHelper::getTrainer()
	{	
		if (!hasTrainer())
		{
			return 0;
		}

		Lock lock(_mutex);

		return _trainerMap[_selectedName].trainer;
	}


	bool ClassifierHelper::parseNamePath(const ssi_char_t *string, StringList &names, StringList &paths)
	{
		ssi_size_t n = ssi_split_string_count(string, ';');
		
		ssi_char_t **tokens = new ssi_char_t *[n];
		ssi_split_string(n, tokens, string, ';');

		for (ssi_size_t i = 0; i < n; i++)
		{
			ssi_char_t *name, *path;
			if (!ssi_split_keyvalue(tokens[i], &name, &path, ':'))
			{
				ssi_wrn("invalid <name:path> token '%s'", string);				
			}

			if (path == 0)
			{
				FilePath fp(name);
				names.add(fp.getName());
				paths.add(name);
			}
			else
			{
				names.add(name);
				paths.add(path);
			}			
		}

		for (ssi_size_t i = 0; i < n; i++)
		{
			delete[] tokens[i];			
		}
		delete[] tokens;		

		return true;
	}
}