// FubiGestures.h
// author: Felix Kistler <kistler@informatik.uni-augsburg.de>
// created: 2012/09/10
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

#ifndef SSI_EVENT_FUBIGESTURES_H
#define SSI_EVENT_FUBIGESTURES_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

	class FubiGestures : public IConsumer {

	public:

		class Options : public OptionList {

		public:

			Options ()
			{
				setRecognizerXml("SampleRecognizers.xml");
				setSender ("FubiGestures");
				addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
				addOption ("recognizer_xml", recognizer_xml, SSI_MAX_CHAR, SSI_CHAR, "xml recognizer definition file (see fubi documentation)");			
			};

			void setSender (const ssi_char_t *sname) {			
				if (sname) {
					ssi_strcpy (this->sname, sname);
				}
			}

			void setRecognizerXml (const ssi_char_t *name) {
				if (name) {
					ssi_strcpy (this->recognizer_xml, name);
				}
			}

			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t recognizer_xml[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName () { return "FubiGestures"; };
		static IObject *Create (const ssi_char_t *file) { return new FubiGestures (file); };
		~FubiGestures ();
		FubiGestures::Options *getOptions () { return &_options; };
		const ssi_char_t *getName () { return GetCreateName (); };
		const ssi_char_t *getInfo () { return "..."; };

		//consumer
		void consume_enter (ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume (IConsumer::info consume_info,
			ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume_flush (ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);

		//event sender
		bool setEventListener (IEventListener *listener);
		const ssi_char_t *getEventAddress () {
			return _event_address.getAddress ();
		}

		void setLogLevel (int level) {
			ssi_log_level = level;
		}
	
		void setMetaData (ssi_size_t size, const void *meta) {
			_meta_received++;		
			memcpy (_meta_in_ptr, meta, size);
			_meta_in_ptr += size; //if there are multiple meta's, we will concatenate them
		}

	protected:

		FubiGestures(const ssi_char_t *file = 0);
		void checkRecognizers(unsigned int userID, ssi_size_t time_ms);
		void reset (ssi_time_t start);
		void convertSSIRot(float* jointData, ssi_size_t jointID);

		FubiGestures::Options _options;
		ssi_char_t *_file;
		
		ssi_byte_t _meta_in[256];
		ssi_byte_t *_meta_in_ptr;
		ssi_size_t _meta_received;

		static char *ssi_log_name;
		int ssi_log_level;

		IEventListener *_elistener;
		EventAddress _event_address;

		//ssi_event_t _event;	
		ssi_event_t *_events_postures;	
		ssi_event_t *_events_combinations;	
		ssi_size_t _glue_id;

		bool *_recognizerActive;
		ssi_size_t *_recognizerStart;
		ssi_size_t *_recognizerEnd;
		ssi_size_t _n_recognizer_postures;

		bool *_combinationSucces;		
		ssi_size_t _n_recognizer_combinations;

		// Maximum number of tracked users
		static const int Fubi_MaxUsers = 15;
		static const int Fubi_NUM_JOINTS = 25;
		static const int Fubi_JointValueLength = 8;
		float _fubiSkeleton[Fubi_MaxUsers][Fubi_NUM_JOINTS][Fubi_JointValueLength];
	};

}

#endif
