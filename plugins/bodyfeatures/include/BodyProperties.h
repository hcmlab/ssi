// BodyProperties.h
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2017/07/13
// Copyright (C) 2017 University of Augsburg, Tobias Baur
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

#ifndef SSI_EVENT_BODYPROPERTIES_H
#define SSI_EVENT_BODYPROPERTIES_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "SSI_SkeletonCons.h"
#include "thread/Timer.h"

namespace ssi {
	class BodyProperties : public IFeature {
	public:

		class Options : public OptionList {
		public:

			Options() :hangin(0), hangout(0),
				leanfrontthreshold(-230.0), leanbackthreshold(100.0), shouldersupthreshold(80.0), elbowdistancethreshold(400.0),
				armsopenthreshold(500.0), handheadthreshold(300), armscrossedthres(150), handhipdistancethres(300){
				setSender("BodyProperties");
				setEvent("BodyProperty");
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");
				addOption("hangin", &hangin, 1, SSI_REAL, "hang in to trigger event (if sent to event board)");
				addOption("hangout", &hangin, 1, SSI_REAL, "hang out to finish event (if sent to event board)");
				addOption("leanfrontthres", &leanfrontthreshold, 1, SSI_REAL, "Threshold for Lean front Detection");
				addOption("leanbackthres", &leanbackthreshold, 1, SSI_REAL, "Threshold for Lean back Detection");
				addOption("shouldersupthres", &shouldersupthreshold, 1, SSI_REAL, "Threshold for Shoulders Up Detection");
				addOption("elbowdistancethres", &elbowdistancethreshold, 1, SSI_REAL, "Threshold for Elbow Distance Detection");
				addOption("armsopenthres", &armsopenthreshold, 1, SSI_REAL, "Threshold for Shoulders Up Detection");
				addOption("handheadthres", &handheadthreshold, 1, SSI_REAL, "Threshold for Elbow Distance Detection");
				addOption("armscrossedthres", &armscrossedthres, 1, SSI_REAL, "Threshold for Hand-Elbow Distance");
				addOption("handhipdistnacethres", &handhipdistancethres, 1, SSI_REAL, "Threshold for hip-wrist distance");
			};

			void setSender(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}

			void setEvent(const ssi_char_t *ename) {
				if (ename) {
					ssi_strcpy(this->ename, ename);
				}
			}

			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
			ssi_real_t hangin;
			ssi_real_t hangout;
			ssi_real_t leanfrontthreshold;
			ssi_real_t leanbackthreshold;
			ssi_real_t shouldersupthreshold;
			ssi_real_t elbowdistancethreshold;
			ssi_real_t armsopenthreshold;
			ssi_real_t handheadthreshold;
			ssi_real_t armscrossedthres;
			ssi_real_t handhipdistancethres;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "BodyProperties"; };
		static IObject *Create(const ssi_char_t *file) { return new BodyProperties(file); };
		~BodyProperties();
		BodyProperties::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Body Properties Detection. Gives normalized confidence values for defined properties"; };

		static ssi_char_t **GetPropertyNames(ssi_size_t &n) {
			static ssi_char_t *names[] = { "LeanPosture", "ArmOpenness", "HeadOrientation", "HeadTilt",
				"ArmsCrossed", "HeadNod", "HeadShake", "HeadTouch",
				"HandDistanceLeft", "HandDistanceRight", "AngleElbowLeftY", "AngleElbowRightY",
				"HandDistanceFrontLeft", "HandDistanceFrontRight", "AngleElbowLeftX","AngleElbowRightX",
				"EnergyHead", "HeadRotationX", "StandardDeviationHeadX", "StandardDeviationHeadRot"};
			n = sizeof(names) / sizeof(ssi_char_t *);
			return names;
		}

		//transformer

		void transform_enter(
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			ssi_size_t NPropertyNames;
			GetPropertyNames(NPropertyNames);
			return  NPropertyNames;
		}
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
	
			return sizeof(SSI_FLOAT);
		}
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_FLOAT) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_FLOAT;
		}

		//event sender
		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		void setLogLevel(int level) {
			ssi_log_level = level;
		}

	protected:

		BodyProperties(const ssi_char_t *file = 0);
		BodyProperties::Options _options;
		ssi_char_t *_file;

		static char *ssi_log_name;
		int ssi_log_level;

		IEventListener *_elistener;
		ssi_event_t _event;
		EventAddress _event_address;

		ssi_size_t _update_ms;
		ssi_size_t _update_counter;

		ssi_time_t _start, _last;
		ssi_real_t normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max);

		ssi_size_t NPropertyNames;
		ssi_char_t **PropertyNames;
		ssi_real_t *values;
		bool highXInsideDirection, leftarmdown, rightarmdown, alrdycnt;
		bool high1, high2, low1, low2, pause1, pause2, left1, left2, right1, right2;
		ssi_real_t framecount, lasthnf;
		ssi_size_t *nodcount;
		Timer*                      m_timer;
	};
}

#endif
