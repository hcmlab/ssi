// BodyProperties.cpp
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2013/02/25
// Copyright (C) 2007-12 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
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

#include "../include/BodyProperties.h"
#include "base/Factory.h"
#include <Math.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	char *BodyProperties::ssi_log_name = "bodyproper";

	BodyProperties::BodyProperties(const ssi_char_t *file)
		: ssi_log_level(SSI_LOG_LEVEL_DEFAULT),
		_elistener(0),
		_update_ms(0),
		_file(0),
		framecount(0),
		nodcount(0),
		lasthnf(0),
		m_timer(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		PropertyNames = GetPropertyNames(NPropertyNames);
		values = new ssi_real_t[NPropertyNames];

		ssi_event_init(_event, SSI_ETYPE_MAP);
	}

	BodyProperties::~BodyProperties() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		delete[] values;
		ssi_event_destroy(_event);
	}

	bool BodyProperties::setEventListener(IEventListener *listener) {
		_elistener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
		_event.prob = 1.0;

		ssi_event_adjust(_event, NPropertyNames * sizeof(ssi_event_map_t));
		ssi_event_map_t *e = ssi_pcast(ssi_event_map_t, _event.ptr);
		for (ssi_size_t i = 0; i < NPropertyNames; i++) {
			e[i].id = Factory::AddString(PropertyNames[i]);;
			e[i].value = 0.0f;
		}

		return true;
	}

	void BodyProperties::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		_update_counter = 0;
	}

	void BodyProperties::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_time_t time = info.time;
		ssi_real_t* ptr = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;
		ssi_real_t *out = ssi_pcast(ssi_real_t, stream_out.ptr);
		SSI_SKELETON *ss = 0;

		highXInsideDirection = high1 = high2 = low1 = low2 = pause1 = pause2 = left1 = left2 = right1 = right2 = false;
		double armscrossed = 0;
		double headtouch = 0;

		if (sample_dimension == SSI_SKELETON_JOINT::NUM * SSI_SKELETON_JOINT_VALUE::NUM) {
			ss = ssi_pcast(SSI_SKELETON, stream_in.ptr);
		}
		else {
			ssi_err("skeleton not supported");
			return;
		}

		for (ssi_size_t i = 0; i < NPropertyNames; i++) {
			values[i] = 0.0;
		}

		alrdycnt = false;
		double highprob = 0;
		double lowprob = 0;
		double minhighlow = 0.0;
		double maxhighlow = 7.0;

		double leftprob = 0;
		double rightprob = 0;
		double minleftright = 0.0;
		double maxleftright = 7.0;
		double armstolerance = 10;

		double leftwristx = 0;
		double leftwristy = 0;

		double rightwristx = 0;
		double rightwristy = 0;


		armscrossed = 0;
		headtouch = 0;

		ssi_real_t *peaks;
		ssi_real_t *bottoms;

		for (ssi_size_t i = 0; i < sample_number; i++)
		{
			values[0] = values[0] + (ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::WAIST][SSI_SKELETON_JOINT_VALUE::POS_Z]); //Lean
			values[1] = values[1] + abs(ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X]);//Handsdistance
			values[2] = values[2] + abs(ss[i][SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::ROT_Y]);//Headrotation
			values[3] = values[3] + abs(ss[i][SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::ROT_Z]);//Headtilt
			values[5] = values[5] + ss[i][SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::ROT_X];//Nod
			values[6] = values[6] + ss[i][SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::ROT_Y];//Shake
			
			values[4] = values[4] + abs(ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X]) + abs(ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Y]); //we cheat a little here, using the unused array space for values[4] for the other hand.
			


			double righthandheadtouch = std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Z]), 2));
			double lefthandheadtouch = std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Z]), 2));


			if (righthandheadtouch < _options.handheadthreshold || lefthandheadtouch < _options.handheadthreshold) headtouch = 1.0;
	

			double righthandleftelbow = std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_X]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2));
			double lefthandrightelbow = std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_X]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2));


			if (righthandleftelbow < _options.armscrossedthres && lefthandrightelbow < _options.armscrossedthres) armscrossed = 1.0;
			else if (righthandleftelbow < _options.armscrossedthres) armscrossed = 0.5;
			else if (lefthandrightelbow < _options.armscrossedthres) armscrossed = 0.5;

			values[8] = values[8] + std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2)); //Distance left wrist left hip
			values[9] = values[9] + std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2)); //Distance right wrist right hip

			values[14] = values[14] + ss[i][SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::ROT_X];
			values[15] = values[15] + ss[i][SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::ROT_X];

			values[10] = values[10] + ss[i][SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::ROT_Y];
			values[11] = values[11] + abs(ss[i][SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::ROT_Y]);

			values[12] = values[12] + std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2)); //Wrist in front of hip left
			values[13] = values[13] + std::sqrt(std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z]), 2) + std::pow((ss[i][SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y]), 2)); //Wrist in front of hip right

			values[17] = values[17] + ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X];

			values[18] = values[18] + ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_X];

			values[19] = values[19] + ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X];

		}

		peaks =  new ssi_real_t[sample_number];
		bottoms = new ssi_real_t[sample_number];

		for (int i = 0; i < sample_number; i++) {
			peaks[i] = FLT_MIN;
			bottoms[i] = FLT_MIN;
		}

		float headthres = 2;
		float bot, prev = ss[0][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[0][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_X];


		float maxrothead = 10;

		for (int i = 1; i < sample_number - 1; i++) {
			if (ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X] > ss[i - 1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X] &&
				ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X] > ss[i + 1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X])
			{
				peaks[i] = ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X];
				bottoms[i] = FLT_MIN;
			}
			else if (ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X] < ss[i - 1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X] &&
				ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X]< ss[i + 1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X])
			{
				peaks[i] = FLT_MIN;
				bottoms[i] = ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X];
			}
		}

		values[5] = 0;

		for (ssi_size_t i = 1; i < sample_number - 1; i++) {
			if (peaks[i] != FLT_MIN) {
				for (ssi_size_t j = i; j < sample_number; j++) {
					if (bottoms[j] != FLT_MIN) {
						for (ssi_size_t k = j; k < sample_number; k++) {
							if (peaks[k] != FLT_MIN && peaks[i] - bottoms[j] > headthres && peaks[i] - bottoms[j] < maxrothead) {
								values[5] = peaks[i] - bottoms[j];
								k = sample_number;
								j = sample_number;
								i = sample_number;
							}
						}
					}
				}
			}
		}


		double thresLeftRight = 10;
		double refLeftRight = values[6] / sample_number;
		double diffLeftRight;
		double left = DBL_MIN;
		double right = DBL_MAX;

		for (ssi_size_t i = 0; i < sample_number; i++) {
			if (ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_CONF] >= 0.5) {
				diffLeftRight = refLeftRight - ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_Y];
			
				if (diffLeftRight > refLeftRight && diffLeftRight > left)
					left = diffLeftRight;

				if (diffLeftRight < refLeftRight && diffLeftRight < right)
					right = diffLeftRight;
				
			}

		}

		if (left > thresLeftRight && abs(right) > thresLeftRight) {
			values[6] = 1;
		}
		else {
			values[6] = 0;
		}

		float tmp_x = 0;
		float tmp_y = 0;
		float tmp_z = 0;

		float sumdata = 0;
		int sumcount = 0;

		for (ssi_size_t i = 1; i < sample_number; i++) {

			if (ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_CONF] > 0.5f && ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_CONF] > 0.5f)
			{

				tmp_x = abs(abs(ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_X]) - abs(ss[i-1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_X]));
				tmp_y = abs(abs(ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_Y]) - abs(ss[i - 1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_Y]));
				tmp_z = abs(abs(ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_Z]) - abs(ss[i - 1][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_Z]));
				sumdata += (tmp_x * tmp_x + tmp_y * tmp_y + tmp_z * tmp_z) / 3;
				sumcount += 1;
			}
			
		}
		
		float tmp_sd = 0;
		float tmp_mean = values[18] / sample_number;

		float tmp_sd_rot = 0;
		float tmp_mean_rot = values[19] / sample_number;

		for (ssi_size_t i = 0; i < sample_number; i++) {

			tmp_sd = tmp_sd + std::pow((ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_X] - tmp_mean), 2);
			tmp_sd_rot = tmp_sd_rot + std::pow((ss[i][SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::ROT_X] - tmp_mean_rot), 2);

		}

		framecount = framecount + sample_number;

		ssi_real_t back = normalizevalue(values[0] / sample_number, 0.0, _options.leanbackthreshold);    //LeanBack
		ssi_real_t front = normalizevalue(values[0] / sample_number, 0.0, _options.leanfrontthreshold); //LeanFront

		//LeanPosture
		if (front > 0) values[0] = 0.5 + front / 2;
		else  values[0] = 0.5 - back / 2;

		values[1] = normalizevalue(values[1] / sample_number, 150.0, _options.armsopenthreshold); //ArmOpenness
		values[2] = normalizevalue(45 - (values[2] / sample_number), 5.0, 45.0); //HeadOrientation
		values[3] = normalizevalue(values[3] / sample_number, 0.0, 28.0); //HeadTilt
		values[7] = headtouch;
		values[4] = armscrossed;
		values[8] = normalizevalue(values[8] / sample_number, 0, _options.handhipdistancethres); //Distance left wrist left hip
		values[9] = normalizevalue(values[9] / sample_number, 0, _options.handhipdistancethres); //Distance right wrist right hip
		values[10] = normalizevalue((values[10] / sample_number), 0, 180); //left elbow y rot
		values[11] = normalizevalue((values[11] / sample_number), 0, 180); //right elbow y rot
		values[12] = normalizevalue(values[12] / sample_number, 0, _options.handhipdistancethres); //Wrist in front of hip left
		values[13] = normalizevalue(values[13] / sample_number, 0, _options.handhipdistancethres); //Wrist in front of hip right
		values[14] = normalizevalue((values[14] / sample_number), -30, 45); //left elbow x rot
		values[15] = normalizevalue((values[15] / sample_number), -30, 45); //right elbow x rot
		values[16] = normalizevalue((sumcount > 0 ? sqrt(sumdata / sumcount) : 0), 0, 50); // energy head
		values[17] = values[17] / sample_number; //head x rot
		values[18] = sqrt(tmp_sd / (sample_number - 1)); //standard deviation head X
		values[19] = sqrt(tmp_sd_rot / (sample_number - 1)); //standard deviation head rot x


		for (int i = 0; i < NPropertyNames; i++) {
			*(out++) = values[i];
		}

		_event.time = ssi_cast(ssi_size_t, 1000 * info.time + 0.5);
		_event.dur = 0;

		if (_elistener)
		{
			ssi_event_map_t *e = ssi_pcast(ssi_event_map_t, _event.ptr);
			for (ssi_size_t i = 0; i < NPropertyNames; i++) {
				e[i].value = ssi_cast(ssi_real_t, values[i]);
			}

			_elistener->update(_event);
		}
	}

	ssi_real_t BodyProperties::normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max) {
		ssi_real_t result = (ssi_real_t)((value - min)*(1 / (max - min)));
		if (result < 0.0) return 0.0;
		else if (result > 1.0) return 1.0;
		else return result;
	}

}