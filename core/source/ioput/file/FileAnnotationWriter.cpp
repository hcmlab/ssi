// FileAnnotationWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/12
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

#include "ioput/file/FileAnnotationWriter.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	namespace old
	{

		FileAnnotationWriter::FileAnnotationWriter(const ssi_char_t *filename, const ssi_char_t *label, const ssi_char_t *tier)
			: _file(0),
			_label(0),
			_tier(0),
			_meta(0),
			_time(0),
			_duration(0),
			_filename(0) {
			if (filename) {
				FilePath fp(filename);
				if (ssi_strcmp(fp.getExtension(), ".csv")) {
					_filename = ssi_strcpy(filename);
				}
				else {
					_filename = ssi_strcat(filename, ".csv");
				}

				_file = fopen(_filename, "w");
				if (_file == 0) {
					ssi_err("could not open file %s", _filename);
				}
			}
			else {
				_file = stdout;
			}

			if (label) {
				_label = ssi_strcpy(label);
			}

			if (tier) {
				_tier = ssi_strcpy(tier);
			}

			_meta = new ssi_char_t[2056];
		}

		FileAnnotationWriter::~FileAnnotationWriter() {
			delete[] _label;
			delete[] _tier;
			_label = 0;
			if (_filename) {
				fclose(_file);
				delete[] _filename;
				_filename = 0;
			}
		}

		void FileAnnotationWriter::setLabel(const ssi_char_t *label) {
			Lock lock(_label_mutex);

			delete[] _label; _label = 0;
			if (label) {
				_label = ssi_strcpy(label);
			}
		}

		void FileAnnotationWriter::setTier(const ssi_char_t *tier) {
			Lock lock(_tier_mutex);

			delete[] _tier; _tier = 0;
			if (tier) {
				_tier = ssi_strcpy(tier);
			}
		}

		bool FileAnnotationWriter::update(ssi_event_t &e) {
			if (_time == 0) {
				_time = e.time / 1000.0;
			}
			_duration += e.dur / 1000.0;
			if (e.type == SSI_ETYPE_STRING)
			{
				if (e.ptr != NULL)
				{
					sprintf(_meta, ";%s", e.ptr);
				}


			}

			else sprintf(_meta, "");

			if (e.state == SSI_ESTATE_COMPLETED) {
				{
					Lock lock(_label_mutex);

					if (_label) {
						if (_options.eventNameaAsTier)
						{
							ssi_sprint(_string, "%lf;%lf;%s;#%s", _time, _time + _duration, _label, Factory::GetString(e.event_id));
						}

						else if (_options.senderNameAsTier)
						{
							ssi_sprint(_string, "%lf;%lf;%s;#%s", _time, _time + _duration, _label, Factory::GetString(e.sender_id));
						}

						else
						{
							ssi_sprint(_string, "%lf;%lf;%s", _time, _time + _duration, _label);
						}

						if (_tier)
						{
							ssi_sprint(_string, "%lf;%lf;%s;#%s", _time, _time + _duration, _label, _tier);
						}
					}

					else {
						if (_options.eventNameAsLabel && _options.eventNameaAsTier)
						{
							ssi_sprint(_string, "%lf;%lf;%s;#%s%s", _time, _time + _duration, Factory::GetString(e.event_id), Factory::GetString(e.event_id), _meta);
						}

						else if (_options.eventNameAsLabel && _options.senderNameAsTier)
						{
							ssi_sprint(_string, "%lf;%lf;%s;#%s%s", _time, _time + _duration, Factory::GetString(e.event_id), Factory::GetString(e.sender_id), _meta);
						}

						else if (_options.eventNameAsLabel)
						{
							ssi_sprint(_string, "%lf;%lf;%s", _time, _time + _duration, Factory::GetString(e.event_id));
						}
						else
						{
							ssi_sprint(_string, "%lf;%lf", _time, _time + _duration);
						}
					}
				}
				_time = 0;
				_duration = 0;

				ssi_fprint(_file, "%s\n", _string);
				fflush(_file);
			}
			return true;
		}

	}
}