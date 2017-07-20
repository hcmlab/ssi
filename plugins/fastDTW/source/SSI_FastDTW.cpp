// FastDTW.cpp
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

#include "SSI_FastDTW.h"
#include "base/Factory.h"
#include "ioput/file/FileTools.h"


namespace ssi {

	char SSI_FastDTW::ssi_log_name[] = "fastdtw___";

	SSI_FastDTW::SSI_FastDTW(const ssi_char_t *file)
		: _file(0), _elistener(0), inputdim(0){

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_MAP);

		classnames = new std::vector<std::string>();
	}

	SSI_FastDTW::~SSI_FastDTW() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		//delete event
		ssi_event_destroy(_event);
	}


	bool SSI_FastDTW::setEventListener(IEventListener *listener) {

		if ( _options.address[0] == '\0')
		{
			return false;
		}

		_elistener = listener;
		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _eaddress, _event);

		return true;
	}


	void SSI_FastDTW::consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{
		SampleList sampleList;
		ModelTools::LoadSampleList(sampleList, _options.opt_samplefile);

		sampleList.printInfo();

		//get streams of classes from file sample file and store in memory

		for (int i = 0; i < sampleList.getSize(); i++) {

			ssi_sample_t* sample = sampleList.get(i);

			for (int j = 0; j < sample->num; j++) {

				uint32_t id = sample->class_id;

				Factory::AddString(sampleList.getClassName(id));
				classnames->push_back(sampleList.getClassName(id));

				ssi_stream_t **stream = sample->streams;

				inputdim = stream[j]->dim;
				if (stream[j]->dim < 1 || stream[j]->dim > 3)
				{
					ssi_err("Sample list: Unsupported count of dimensions!");
				}

				if (inputdim == 1) {

					fastdtw::TimeSeries<float, 1> *ts = new fastdtw::TimeSeries<float, 1>();
					float* f1 = reinterpret_cast<float*>(stream[j]->ptr);

					for (int k = 0; k < stream[j]->num; k++) {
						ts->addLast(ts->numOfPts(), fastdtw::TimeSeriesPoint<float, 1>(f1++));
					}

					timeseries_1d.push_back(ts);
				} else if (inputdim == 2) {

					fastdtw::TimeSeries<float, 2> *ts = new fastdtw::TimeSeries<float, 2>();
					float* f1 = reinterpret_cast<float*>(stream[j]->ptr);

					for (int k = 0; k < stream[j]->num; k++) {
						float arr[2] = { *f1++, *f1++ };
						ts->addLast(ts->numOfPts(), fastdtw::TimeSeriesPoint<float, 2>(arr));
					}

					timeseries_2d.push_back(ts);
				} else if (inputdim == 3)
				{
					fastdtw::TimeSeries<float, 3> *ts = new fastdtw::TimeSeries<float, 3>();
					float* f1 = reinterpret_cast<float*>(stream[j]->ptr);

					for (int k = 0; k < stream[j]->num; k++) {
						float arr[3] = { *f1++, *f1++, *f1++ };
						ts->addLast(ts->numOfPts(), fastdtw::TimeSeriesPoint<float, 3>(arr));
					}

					timeseries_3d.push_back(ts);
				}


			}
		}

		sampleList.clear();
	}


	void SSI_FastDTW::consume(IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{

		if (inputdim != stream_in[0].dim)
			ssi_err("Dimension of sample list differs from input stream!");

		if  (stream_in_num == 1 && stream_in[0].dim > 0 && stream_in[0].dim < 4) {

			//store distances between each stored data stream and the current data
			std::multimap<std::string, float> dist_mm;

			if (inputdim == 1) {
				//clear old data
				ts_1d.clear();

				//get gloat values out of input stream (expects that there is just one stream)
				float* f1 = reinterpret_cast<float*>(stream_in[0].ptr);

				//copy raw data into FastDTW timeseries
				for (int i = 0; i < stream_in[0].num; i++)
				{
					ts_1d.addLast(ts_1d.numOfPts(), fastdtw::TimeSeriesPoint<float, 1>(f1++));
				}

				#pragma omp parallel for
				for (int i = 0; i < classnames->size(); i++) {
					fastdtw::TimeWarpInfo<float> info = fastdtw::FAST::getWarpInfoBetween(ts_1d, *timeseries_1d.at(i), fastdtw::EuclideanDistance());

					dist_mm.insert(std::pair<std::string, float>(classnames->at(i), info.getDistance()));
				}
			}
			else if (inputdim == 2) {
				//clear old data
				ts_2d.clear();

				//get gloat values out of input stream (expects that there is just one stream)
				float* f1 = reinterpret_cast<float*>(stream_in[0].ptr);

				//copy raw data into FastDTW timeseries
				for (int i = 0; i < stream_in[0].num; i++)
				{
					float arr[2] = { *f1++, *f1++ };
					ts_2d.addLast(ts_2d.numOfPts(), fastdtw::TimeSeriesPoint<float, 2>(arr));
				}

				#pragma omp parallel for
				for (int i = 0; i < classnames->size(); i++) {
					fastdtw::TimeWarpInfo<float> info = fastdtw::FAST::getWarpInfoBetween(ts_2d, *timeseries_2d.at(i), fastdtw::EuclideanDistance());

					dist_mm.insert(std::pair<std::string, float>(classnames->at(i), info.getDistance()));
				}
			}
			else if (inputdim == 3)
			{
				//clear old data
				ts_3d.clear();

				//get gloat values out of input stream (expects that there is just one stream)
				float* f1 = reinterpret_cast<float*>(stream_in[0].ptr);

				//copy raw data into FastDTW timeseries
				for (int i = 0; i < stream_in[0].num; i++)
				{
					float arr[3] = { *f1++, *f1++, *f1++ };
					ts_3d.addLast(ts_3d.numOfPts(), fastdtw::TimeSeriesPoint<float, 3>(arr));
				}

				#pragma omp parallel for
				for (int i = 0; i < classnames->size(); i++) {
					fastdtw::TimeWarpInfo<float> info = fastdtw::FAST::getWarpInfoBetween(ts_3d, *timeseries_3d.at(i), fastdtw::EuclideanDistance());

					dist_mm.insert(std::pair<std::string, float>(classnames->at(i), info.getDistance()));
				}
			}


			if (_elistener) {

				switch (_options.outputOption)  {

					case 0: {

							//average distance
							std::map<std::string, float> m_dist;
							std::map<std::string, int> m_count;

							std::multimap<std::string, float>::iterator it;

							for (it = dist_mm.begin(); it != dist_mm.end(); ++it) {
								if (m_dist.find((*it).first) == m_dist.end())
								{
									m_dist[(*it).first] = (*it).second;
									m_count[(*it).first] = 1;
								}
								else {
									m_dist[(*it).first] += (*it).second;
									m_count[(*it).first]++;
								}
							}

							int j = 0;

							//create event with all classnames and distances
							ssi_event_adjust(_event, m_dist.size() * sizeof(ssi_event_map_t));
							ssi_event_map_t *e = ssi_pcast(ssi_event_map_t, _event.ptr);

							for (map<std::string, float>::iterator it = m_dist.begin(); it != m_dist.end(); ++it) {

								e[j].id = Factory::GetStringId(it->first.c_str());
								e[j].value = ssi_cast(ssi_real_t, it->second / m_count[it->first]);

								j++;
							}
					} break;

					case 1:
						{
							ssi_event_adjust(_event,classnames->size() * sizeof(ssi_event_map_t));
							ssi_event_map_t *e = ssi_pcast(ssi_event_map_t, _event.ptr);
							
							int j = 0;
							std::multimap<std::string, float>::iterator it;

							for (it = dist_mm.begin(); it != dist_mm.end(); ++it) {
								e[j].id = Factory::GetStringId((*it).first.c_str());
								e[j].value = ssi_cast(ssi_real_t, (*it).second);

								j++;
							}
					} break;

					case 2:  {

						//average distance
						std::map<std::string, float> m_dist;
						std::map<std::string, int> m_count;

						std::multimap<std::string, float>::iterator it;

						for (it = dist_mm.begin(); it != dist_mm.end(); ++it) {
							if (m_dist.find((*it).first) == m_dist.end())
							{
								m_dist[(*it).first] = (*it).second;
								m_count[(*it).first] = 1;
							}
							else {
								m_dist[(*it).first] += (*it).second;
								m_count[(*it).first]++;
							}
						}

						int j = 0;

						//create event with all classnames and distances
						ssi_event_adjust(_event, 1 * sizeof(ssi_event_map_t));
						ssi_event_map_t *e = ssi_pcast(ssi_event_map_t, _event.ptr);

						std::string classname = "";
						float min_dist = -1;

						for (map<std::string, float>::iterator it = m_dist.begin(); it != m_dist.end(); ++it) {

							float val = it->second / m_count[it->first];
							if (min_dist == -1 || min_dist > val) {
								classname = it->first;
								min_dist = val;
							}
						}

						e[0].id = Factory::GetStringId(classname.c_str());
						e[0].value = ssi_cast(ssi_real_t, min_dist);

					} break;
					default:
						ssi_err("Unknown option for eventdata!")
					}



				_event.time = Factory::GetFramework()->GetElapsedTimeMs();
				_event.dur = 0;
				_event.state = SSI_ESTATE_COMPLETED;

				_elistener->update(_event);
			}

		} else
		{
			ssi_err("Just one input stream with dimension 1, 2 or 3 is allowed!");
		}

	}

	void SSI_FastDTW::consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{
		if (inputdim == 1) {
			//clear stream data that was loaded from disk
			for (int i = 0; i < timeseries_1d.size(); i++) {
				timeseries_1d.at(i)->clear();
				delete timeseries_1d.at(i);
			}

			timeseries_1d.clear();
		} else if (inputdim == 2) {
			//clear stream data that was loaded from disk
			for (int i = 0; i < timeseries_2d.size(); i++) {
				timeseries_2d.at(i)->clear();
				delete timeseries_2d.at(i);
			}

			timeseries_2d.clear();
		} else if (inputdim == 3)
		{
			//clear stream data that was loaded from disk
			for (int i = 0; i < timeseries_3d.size(); i++) {
				timeseries_3d.at(i)->clear();
				delete timeseries_3d.at(i);
			}

			timeseries_3d.clear();
		}

		delete classnames;
	}

}
