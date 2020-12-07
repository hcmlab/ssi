// emosim.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2020/10/26
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

#ifndef EMOSIM_H
#define EMOSIM_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/ITheEventBoard.h"
#include "emopainter.h"

namespace ssi {

	class Window;
	class Canvas;

	class EmoSim : public IObject, public INotify {

	public:

		class Options : public OptionList {

		public:

			Options() : update_ms(100), openness(0.0f), conscientiousness(0.0f), extraversion(0.0f), agreeableness(0.0f), neuroticism(0.0f) {

				setAddress("");
				setSenderName("sender");
				setEventName("event");

				setTitle("EmoSim");
				setPosition(0, 0, 100, 100);

				setUserValenceID("");
				setUserArousalID("");

				setFeedbackPosID("");
				setFeedbackNegID("");

				addOption("address", address, SSI_MAX_CHAR, SSI_CHAR, "event address (if sent to event board) (event@sender)");
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
				addOption("update_ms", &update_ms, 1, SSI_INT, "time interval the updated emotion vector is sent via event");
				addOption("wcaption", wcaption, SSI_MAX_CHAR, SSI_CHAR, "window caption");
				addOption("move", move, 4, SSI_INT, "window position (left, top, width, height)");
				addOption("user_valence_id", user_valence_id, SSI_MAX_CHAR, SSI_CHAR, "address of user valence source");
				addOption("user_arousal_id", user_arousal_id, SSI_MAX_CHAR, SSI_CHAR, "address of user arousal source");				
				addOption("feedback_pos_id", feedback_pos_id, SSI_MAX_CHAR, SSI_CHAR, "address of positive feedback source");
				addOption("feedback_neg_id", feedback_neg_id, SSI_MAX_CHAR, SSI_CHAR, "address of negative feedback source");
				addOption("openness", &openness, 1, SSI_REAL, "Openness", false);
				addOption("conscientiousness", &conscientiousness, 1, SSI_REAL, "Conscientiousness", false);
				addOption("extraversion", &extraversion, 1, SSI_REAL, "Extraversion", false);
				addOption("agreeableness", &agreeableness, 1, SSI_REAL, "Agreeableness", false);
				addOption("neuroticism", &neuroticism, 1, SSI_REAL, "Neuroticism", false);

			};

			void setAddress(const ssi_char_t* address) {
				if (address) {
					ssi_strcpy(this->address, address);
				}
			}
			void setSenderName(const ssi_char_t* sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}
			void setEventName(const ssi_char_t* ename) {
				if (ename) {
					ssi_strcpy(this->ename, ename);
				}
			}
			void setTitle(const ssi_char_t* caption) {
				if (caption) {
					ssi_strcpy(this->wcaption, caption);
				}
			}
			void setPosition(int left, int top, int width, int height) {
				move[0] = left; move[1] = top; move[2] = width; move[3] = height;
			}
			void setUserValenceID(const ssi_char_t* valence_id) {
				if (valence_id) {
					ssi_strcpy(this->user_valence_id, valence_id);
				}
			}
			void setUserArousalID(const ssi_char_t* arousal_id) {
				if (arousal_id) {
					ssi_strcpy(this->user_arousal_id, arousal_id);
				}
			}
			void setFeedbackPosID(const ssi_char_t* feedback_id) {
				if (feedback_id) {
					ssi_strcpy(this->feedback_pos_id, feedback_id);
				}
			}
			void setFeedbackNegID(const ssi_char_t* feedback_id) {
				if (feedback_id) {
					ssi_strcpy(this->feedback_neg_id, feedback_id);
				}
			}
			
			//void setOcean(ssi_real_t openness, ssi_real_t conscientiousness, ssi_real_t extraversion, ssi_real_t agreeableness, ssi_real_t neuroticism) {
			//	ocean[0] = openness;
			//	ocean[1] = conscientiousness;
			//	ocean[2] = extraversion;
			//	ocean[3] = agreeableness;
			//	ocean[4] = neuroticism;

			//	/*		[0] openness to experience		(cautious vs. curious)		[-1.0f .. 1.0f]
			//			[1] conscientiousness			(careless vs. organized)	[-1.0f .. 1.0f]
			//			[2] extraversion				(reserved vs. energetic)	[-1.0f .. 1.0f]
			//			[3] agreeableness				(challenging vs. friendly)	[-1.0f .. 1.0f]
			//			[4] neuroticism					(confident vs. nervous)		[-1.0f .. 1.0f]		*/

			//	/*		https://en.wikipedia.org/wiki/Big_Five_personality_traits						*/
			//}

			ssi_char_t address[SSI_MAX_CHAR];
			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
			ssi_size_t update_ms;
			ssi_char_t wcaption[SSI_MAX_CHAR];
			int move[4];
			ssi_char_t user_valence_id[SSI_MAX_CHAR];
			ssi_char_t user_arousal_id[SSI_MAX_CHAR];
			ssi_char_t feedback_pos_id[SSI_MAX_CHAR];
			ssi_char_t feedback_neg_id[SSI_MAX_CHAR];
			ssi_real_t openness;
			ssi_real_t conscientiousness;
			ssi_real_t extraversion;
			ssi_real_t agreeableness;
			ssi_real_t neuroticism;
		};

	public:

		static const ssi_char_t* GetCreateName() { return "EmoSim"; };
		static IObject* Create(const ssi_char_t* file) { return new EmoSim(file); };
		~EmoSim();

		Options* getOptions() { return &_options; };
		const ssi_char_t* getName() { return GetCreateName(); };
		const ssi_char_t* getInfo() { return "..."; };

		void listen_enter();
		bool update(IEvents& events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

		bool setEventListener(IEventListener* listener);
		const ssi_char_t* getEventAddress() {
			return _event_address.getAddress();
		}

		bool notify(INotify::COMMAND::List command, const ssi_char_t* message);

		void convertToEmotionLabel(ssi_char_t* emo_label, ssi_real_t valenceValue, ssi_real_t arousalValue);

		void CalculatePersonalitySettings();

	protected:

		EmoSim(const ssi_char_t* file = 0);
		Options _options;
		static char ssi_log_name[];
		ssi_char_t* _file;
		int ssi_log_level;

		EventAddress _event_address;

		IEventListener* _listener;
		ssi_event_t _event;
		ssi_size_t _update_counter;
		ssi_size_t _update_ms;

		EmoPainter* _plot;
		Window* _window;
		Canvas* _canvas;

		ssi_size_t _user_arousal_id;
		ssi_size_t _user_valence_id;
		ssi_size_t _feedback_pos_id;
		ssi_size_t _feedback_neg_id;

		ssi_real_t _valence_ocean;
		ssi_real_t _arousal_ocean;
		ssi_real_t _valence_base;
		ssi_real_t _arousal_base;
		ssi_real_t _valence_current;
		ssi_real_t _valence_target;
		ssi_real_t _valence_target_save;
		ssi_real_t _arousal_current;
		ssi_real_t _arousal_target;
		ssi_real_t _arousal_target_save;
		ssi_real_t _valence_user;
		ssi_real_t _arousal_user;

		ssi_real_t _empathy_factor_valence;
		ssi_real_t _empathy_factor_arousal;

		ssi_real_t _habituation_speed;
		ssi_real_t _habituation_pos;
		ssi_real_t _habituation_neg;
		ssi_size_t _cooldown_timer;
		ssi_size_t _habituation_timer;

		ssi_real_t _target_approach_speed;

	};

}

#endif