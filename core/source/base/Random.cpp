// Randomi.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/06/04
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "base/Random.h"

#include <random>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi 
{
	ssi_char_t *Random::ssi_log_name = "random____";

	ssi_size_t Random::Seed()
	{
		std::random_device seed;
		return ssi_size_t(seed());
	}

	void Random::Shuffle(ssi_size_t n, ssi_size_t *arr)
	{
		
		Shuffle(n, arr, Seed());
	}

	void Random::Shuffle(ssi_size_t n, ssi_size_t *arr, ssi_size_t seed)
	{
		Randomi random(0, n - 1, seed);
		for (ssi_size_t i = 0; i < n; i++) {
			ssi_int_t r = random.next();
			ssi_size_t tmp = arr[i];
			arr[i] = arr[r];
			arr[r] = tmp;
		}
	}


	ssi_char_t *Randomi::ssi_log_name = "randomi___";

	Randomi::Randomi()
	: _engine(0),
		_distr(0)
	{

	}

	Randomi::Randomi(ssi_int_t a, ssi_int_t b)
	: Randomi()
	{
		init(a, b);
	}

	Randomi::Randomi(ssi_int_t a, ssi_int_t b, ssi_size_t seed)
		: Randomi()
	{
		init(a, b, seed);
	}

	Randomi::~Randomi()		
	{
		delete _engine;
		delete _distr;
	}	

	void Randomi::init(ssi_int_t a, ssi_int_t b)
	{
		init(a, b, Random::Seed());
	}

	void Randomi::init(ssi_int_t a, ssi_int_t b, ssi_size_t seed)
	{
#if SSI_RANDOM_LEGACY_FLAG
		_a = double(a);
		_b = double(b);
#else
		delete _engine;
		_engine = new std::mt19937(seed);
		_distr = new std::uniform_int_distribution<ssi_int_t>(a, b);
#endif
	}

	ssi_int_t Randomi::next()
	{		
#if SSI_RANDOM_LEGACY_FLAG
		return int(ssi_random(_a, _b)+0.5f);
#else
		if (!_engine)
		{
			ssi_err("engine not initialized")
		}

		std::mt19937 *engine = (std::mt19937*)_engine;
		std::uniform_int_distribution<ssi_int_t> *distr = (std::uniform_int_distribution<ssi_int_t> *) _distr;

		return distr->operator()(*engine);
#endif
	}	

	ssi_char_t *Randomf::ssi_log_name = "randomf___";

	Randomf::Randomf()
		: _engine(0),
		_distr(0),
		_type(DISTRIBUTION::UNIFORM)
	{

	}

	Randomf::Randomf(ssi_real_t a, ssi_real_t b, DISTRIBUTION::Value type)
		: Randomf() 
	{
		init(a, b, type);
	}

	Randomf::Randomf(ssi_real_t a, ssi_real_t b, ssi_size_t seed, DISTRIBUTION::Value type)
		: Randomf()
	{
		init(a, b, seed, type);
	}

	Randomf::~Randomf()
	{
		delete _engine;
		delete _distr;
	}

	void Randomf::init(ssi_real_t a, ssi_real_t b, DISTRIBUTION::Value type)
	{
		init(a, b, Random::Seed(), type);
	}

	void Randomf::init(ssi_real_t a, ssi_real_t b, ssi_size_t seed, DISTRIBUTION::Value type)
	{
		_type = type;

#if SSI_RANDOM_LEGACY_FLAG
		_a = double(a);
		_b = double(b);
#else
		delete _engine;
		_engine = new std::mt19937(seed);

		switch (type)
		{
		case DISTRIBUTION::UNIFORM:
			_distr = new std::uniform_real_distribution<ssi_real_t>(a, b);
			break;
		case DISTRIBUTION::NORMAL:
			_distr = new std::normal_distribution<ssi_real_t>(a, b);
			break;
		}
#endif
	}

	ssi_real_t Randomf::next()
	{
#if SSI_RANDOM_LEGACY_FLAG

		switch (_type)
		{
		case DISTRIBUTION::UNIFORM:
			return ssi_real_t(ssi_random(_a, _b));
		case DISTRIBUTION::NORMAL:
			return ssi_real_t(ssi_random_distr(_a, _b));
		}
#else
		if (!_engine)
		{
			ssi_err("engine not initialized")
		}

		std::mt19937 *engine = (std::mt19937*)_engine;

		switch (_type)
		{
		case DISTRIBUTION::UNIFORM:
			return ((std::uniform_real_distribution<ssi_real_t> *)_distr)->operator()(*engine);
		case DISTRIBUTION::NORMAL:
			return ((std::normal_distribution<ssi_real_t> *)_distr)->operator()(*engine);
		}
#endif
		return 0;
	}

}