// Random.h
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

#pragma once

#ifndef SSI_BASE_RANDOM_H
#define SSI_BASE_RANDOM_H

#include "SSI_Cons.h"

namespace ssi 
{
	class Random
	{
	public:

		static ssi_size_t Seed();
		static void Shuffle(ssi_size_t n, ssi_size_t *arr);
		static void Shuffle(ssi_size_t n, ssi_size_t *arr, ssi_size_t seed);

	protected:

		static ssi_char_t *ssi_log_name;
	};

	class Randomi
	{

	public:

		Randomi();
		Randomi(ssi_int_t a, ssi_int_t b);
		Randomi(ssi_int_t a, ssi_int_t b, ssi_size_t seed);
		virtual ~Randomi();

		void init(ssi_int_t a, ssi_int_t b);
		void init(ssi_int_t a, ssi_int_t b, ssi_size_t seed);
		
		ssi_int_t next();

	protected:
		
		static ssi_char_t *ssi_log_name;

#if SSI_RANDOM_LEGACY_FLAG
		double _a, _b;
#endif

		void *_engine;
		void *_distr;
	};

	class Randomf
	{

	public:

		struct DISTRIBUTION
		{
			enum Value
			{
				UNIFORM, // [a..b)
				NORMAL,  // a = mean, b = stdv
				NUM
			};
		};

		Randomf();
		Randomf(ssi_real_t a, ssi_real_t b, DISTRIBUTION::Value type = DISTRIBUTION::UNIFORM);
		Randomf(ssi_real_t a, ssi_real_t b, ssi_size_t seed, DISTRIBUTION::Value type = DISTRIBUTION::UNIFORM);
		virtual ~Randomf();
		
		void init(ssi_real_t a, ssi_real_t b, DISTRIBUTION::Value type = DISTRIBUTION::UNIFORM);
		void init(ssi_real_t a, ssi_real_t b, ssi_size_t seed, DISTRIBUTION::Value type = DISTRIBUTION::UNIFORM);

		ssi_real_t next();

	protected:

		static ssi_char_t *ssi_log_name;

#if SSI_RANDOM_LEGACY_FLAG
		double _a, _b;
#endif

		void *_engine;
		void *_distr;
		DISTRIBUTION::Value _type;

	};
}

#endif