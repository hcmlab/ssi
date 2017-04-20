// FastDTW.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 12/10/2016
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

#ifndef SSI_FASTDTW_FASTDTW_H
#define SSI_FASTDTW_FASTDTW_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "base/IConsumer.h"
#include "event/EventAddress.h"
#include "model/include/ssimodel.h"


#include "FastDTW.h"
#include "DTW.h"
#include "EuclideanDistance.h"

namespace ssi {

	class SSI_FastDTW : public IConsumer {

	public:

		class Options : public OptionList {

		public:

			Options()
			{
				addOption("samplefilename", &opt_samplefile, SSI_MAX_CHAR, SSI_CHAR, "Sample filename.");
				addOption("eventdata", &outputOption, 1, SSI_INT, "0 = avg. distance of all classes; 1 = all distances; 2 = the class with the lowest avg. distance;");
				SSI_OPTIONLIST_ADD_ADDRESS(address);

				setAddress("distances@fastdtw");
				setSampleFilename("");
				outputOption = 0;
			}

			void setAddress(const ssi_char_t *address) {
				if (address) {
					ssi_strcpy(this->address, address);
				}
			}


			void setSampleFilename(const ssi_char_t *filenames) {
				if (filenames) {
					ssi_strcpy(this->opt_samplefile, filenames);
				}
			}

			ssi_size_t outputOption;
			ssi_char_t opt_samplefile[SSI_MAX_CHAR];
			ssi_char_t address[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName() { return "FastDTW"; };
		static IObject *Create(const ssi_char_t *file) { return new SSI_FastDTW(file); };
		~SSI_FastDTW();

		Options *getOptions() override { return &_options; };
		const ssi_char_t *getName() override { return GetCreateName(); };
		const ssi_char_t *getInfo() override { return "Using FastDTW algorithm for pattern recognition of 2 or 3 dimensional data streams."; };

		//consumer
		void consume_enter(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]) override;
		void consume(IConsumer::info consume_info,
			ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]) override;
		void consume_flush(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]) override;

		bool setEventListener(IEventListener *listener) override;
		const ssi_char_t *getEventAddress() override
		{
			return _eaddress.getAddress();
		}

	protected:

		SSI_FastDTW(const ssi_char_t *file = nullptr);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		//streams loaded from sample file
		std::vector<fastdtw::TimeSeries<float, 1>*> timeseries_1d;
		std::vector<fastdtw::TimeSeries<float, 2>*> timeseries_2d;
		std::vector<fastdtw::TimeSeries<float, 3>*> timeseries_3d;

		//current stream data to be compared with DTW
		fastdtw::TimeSeries<float, 1> ts_1d;
		fastdtw::TimeSeries<float, 2> ts_2d;
		fastdtw::TimeSeries<float, 3> ts_3d;


		std::vector<std::string> *classnames;

		int inputdim;


		IEventListener *_elistener;
		ssi_event_t _event;
		EventAddress _eaddress;
	};

}

#endif
