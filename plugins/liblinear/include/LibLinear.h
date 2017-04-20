// LibLinear.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 27/9/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_LIBLINEAR_LIBLINEAR_H
#define SSI_LIBLINEAR_LIBLINEAR_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace ssi {

	class LibLinear : public IModel {

	public:

		struct BALANCE {
			enum List {
				OFF,
				UNDER,
				OVER
			};
		};

		class Options : public OptionList {

		public:

			Options()
				: silent(false), balance(BALANCE::OFF), seed(0) {

				params[0] = '\0';				
				addOption("params", params, SSI_MAX_CHAR, SSI_CHAR, "liblinear parameters (see documentation)");
				addOption("srand", &seed, 1, SSI_UINT, "if >0 use fixed seed to initialize random number generator, otherwise timestamp will be used");
				addOption("balance", &balance, 1, SSI_INT, "balance #samples per class (0=off, 1=remove surplus, 2=create missing)");
				addOption("silent", &silent, 1, SSI_BOOL, "suppress library messages");
			}

			void setParams(const ssi_char_t *params) {
				ssi_strcpy(this->params, params);
			}
			
			ssi_char_t params[SSI_MAX_CHAR];
			bool silent;
			unsigned int seed;
			BALANCE::List balance;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "LibLinear"; };
		static IObject *Create(const ssi_char_t *file) { return new LibLinear(file); };
		~LibLinear();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "A Library for Large Linear Classification."; };

		bool train(ISamples &samples,
			ssi_size_t stream_index);
		bool isTrained() { return _model != 0; };
		bool forward(ssi_stream_t &stream,
			ssi_size_t n_probs,
			ssi_real_t *probs);
		void release();
		bool save(const ssi_char_t *filepath);
		bool load(const ssi_char_t *filepath);

		ssi_size_t getClassSize() { return _n_classes; };
		ssi_size_t getStreamDim() { return _n_features; };
		ssi_size_t getStreamByte() { return sizeof(ssi_real_t); };
		ssi_type_t getStreamType() { return SSI_REAL; };

		void setLogLevel(int level)
		{
			ssi_log_level = level;
		}

	protected:

		LibLinear(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];
		int ssi_log_level;

		ssi_size_t _n_classes;
		ssi_size_t _n_features;
		ssi_size_t _n_samples;		

		bool parseParams(void *params, const ssi_char_t *string);
		bool flag_find_C;
		int nr_fold;
		int flag_C_specified;
		int flag_solver_specified;
		double bias;

		void *_model;

		void exit_with_help();
		static void silent(const char *s) {}
	};



}

#endif
