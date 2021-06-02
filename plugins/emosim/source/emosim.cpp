// emosim.cpp
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

#include "../include/emosim.h"
#include "base/ITheFramework.h"
#include "event/include/TheEventBoard.h"
#include "base/Factory.h"
#include "SSI_Tools.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"

namespace ssi {

	char EmoSim::ssi_log_name[] = "emosim____";

	EmoSim::EmoSim(const ssi_char_t* file)
		: _file(0),
		_listener(0),
		_window(0),
		_canvas(0),
		_plot(0),
		_valence_target_save(0.0f),
		_arousal_target_save(0.0f),
		_valence_user(0.0f),
		_arousal_user(0.0f),
		_habituation_timer(0),
		_cooldown_timer(0),
		_target_approach_speed(1.0f),
		_habituation_speed(1.0f),
		_last_auto_feeback(0),
		_auto_feedback_valence_value(0.0f),
		_auto_feedback_valence_counter (0)
	{

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_MAP);
		_update_counter = 0;

	}

	EmoSim::~EmoSim() {

		ssi_event_destroy(_event);

	}

	bool EmoSim::setEventListener(IEventListener* listener) {

		_listener = listener;

		if (_options.address[0] != '\0') {

			_event_address.setAddress(_options.address);
			_event.sender_id = Factory::AddString(_event_address.getSender(0));
			_event.event_id = Factory::AddString(_event_address.getEvent(0));

		}
		else {

			ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

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
		}

		return true;

	}

	bool EmoSim::notify(INotify::COMMAND::List command, const ssi_char_t* message) {

		switch (command) {

		case INotify::COMMAND::OPTIONS_CHANGE:
		{
			_options.lock();
			char* value = 0;
			if (_options.getOptionValueAsString(message, &value)) {
				ssi_msg(SSI_LOG_LEVEL_BASIC, "new value of option '%s' is %s", message, value);
			}
			_options.unlock();
			delete[] value;

			return true;
		}

		case INotify::COMMAND::RESET:
		{
			ssi_msg(SSI_LOG_LEVEL_BASIC, "reset");

			return true;
		}

		}

		return false;
	};

	void EmoSim::listen_enter() {

		ssi_char_t string[SSI_MAX_CHAR];

		if (_listener) {
			ssi_event_adjust(_event, 4 * sizeof(ssi_event_map_t));
		}

		_update_ms = _options.update_ms;
		if (_update_ms < ssi_pcast(TheEventBoard, Factory::GetEventBoard())->getOptions()->update) {
			ssi_print("update rate of TheEventBoard too small: %d", ssi_pcast(TheEventBoard, Factory::GetEventBoard())->getOptions()->update);
		}

		ssi_sprint(string, _options.user_valence_id);
		_user_valence_id = Factory::AddString(string);
		ssi_sprint(string, _options.user_arousal_id);
		_user_arousal_id = Factory::AddString(string);

		ssi_sprint(string, _options.feedback_pos_id);
		_feedback_pos_id = Factory::AddString(string);
		ssi_sprint(string, _options.feedback_neg_id);
		_feedback_neg_id = Factory::AddString(string);

		_window = new Window();
		_canvas = new Canvas();
		_plot = new EmoPainter();
		
		_canvas->addClient(_plot);
		ssi_sprint(string, _options.wcaption);
		_window->setTitle(string);
		_window->setClient(_canvas);
		_window->setPosition(ssi_rect(_options.move[0], _options.move[1], _options.move[2], _options.move[3]));
		_window->create();
		_window->show();

		_valence_current = 0.0f;
		_valence_target = 0.0f;
		_arousal_current = 0.0f;
		_arousal_target = 0.0f;
		
		_valence_base = 0.0f;
		_arousal_base = 0.0f;

		_habituation_pos = 0.0f;
		_habituation_neg = 0.0f;

		CalculatePersonalitySettings();

		_valence_current = _valence_ocean;
		_arousal_current = _arousal_ocean;
	}

	void EmoSim::CalculatePersonalitySettings() {

		/*		[0] openness to experience		(cautious vs. curious)		[-1.0f .. 1.0f]
				[1] conscientiousness			(careless vs. organized)	[-1.0f .. 1.0f]
				[2] extraversion				(reserved vs. energetic)	[-1.0f .. 1.0f]
				[3] agreeableness				(challenging vs. friendly)	[-1.0f .. 1.0f]
				[4] neuroticism					(confident vs. nervous)		[-1.0f .. 1.0f]		*/

		_valence_ocean = (0.05f * _options.openness) + (0.05f * _options.conscientiousness) + (0.20f * _options.extraversion) + (0.50f * _options.agreeableness) + (0.20f * _options.neuroticism);
		_arousal_ocean = (0.30f * _options.openness) - (0.10f * _options.conscientiousness) + (0.30f * _options.extraversion) + (0.00f * _options.agreeableness) + (0.30f * _options.neuroticism);

		_empathy_factor_valence = _options.agreeableness * 0.5f;
		_empathy_factor_arousal = _options.agreeableness * 0.5f;

		if (_empathy_factor_valence < -1.0f) {
			_empathy_factor_valence = -1.0f;
		}
		if (_empathy_factor_valence > 1.0f) {
			_empathy_factor_valence = 1.0f;
		}
		if (_empathy_factor_arousal < -1.0f) {
			_empathy_factor_arousal = -1.0f;
		}
		if (_empathy_factor_arousal > 1.0f) {
			_empathy_factor_arousal = 1.0f;
		}

		_target_approach_speed = (((_options.openness + 1.0f) * 0.5f) + ((_options.extraversion + 1.0f) * 0.5f)) * 0.5f;
		if (_target_approach_speed < 0.25f) {
			_target_approach_speed = 0.25f;
		}

		_habituation_speed = (1.0f - ((_options.conscientiousness + 1.0f) * 0.5f)) * 0.5f;
		if (_habituation_speed < 0.25f) {
			_habituation_speed = 0.25f;
		}
	}

	void EmoSim::ResolveFeedback(bool positive, ssi_real_t strength) {

		if (positive) {

			_habituation_pos += 0.15f * strength;
			_habituation_neg -= 0.1f  * strength;

			if (_habituation_pos > 1.0f) {
				_habituation_pos = 1.0f;
			}
			if (_habituation_neg > 1.0f) {
				_habituation_neg = 1.0f;
			}
			if (_habituation_pos < -1.0f) {
				_habituation_pos = -1.0f;
			}
			if (_habituation_neg < -1.0f) {
				_habituation_neg = -1.0f;
			}
			if (_habituation_pos <= 0.5f) {
				_valence_base += 0.2f * strength;
				_arousal_base += 0.2f * strength;
			}
			else {
				_valence_base += 0.3f * strength;
				_arousal_base -= 0.1f * strength;
			}
		}
		else
		{
			_habituation_pos -= 0.1f * strength;
			_habituation_neg += 0.15f * strength;

			if (_habituation_pos > 1.0f) {
				_habituation_pos = 1.0f;
			}
			if (_habituation_neg > 1.0f) {
				_habituation_neg = 1.0f;
			}
			if (_habituation_pos < -1.0f) {
				_habituation_pos = -1.0f;
			}
			if (_habituation_neg < -1.0f) {
				_habituation_neg = -1.0f;
			}
			if (_habituation_neg <= 0.5f) {
				_valence_base -= 0.2f * strength;
				_arousal_base += 0.1f * strength;
			}
			else {
				_valence_base -= 0.3f * strength;
				_arousal_base -= 0.2f * strength;
			}
		}
	}

	bool EmoSim::update(IEvents& events, ssi_size_t n_new_events, ssi_size_t time_ms) {

		CalculatePersonalitySettings();

		ssi_char_t string[SSI_MAX_CHAR];

		if (time_ms - _habituation_timer > 100) {

			_habituation_timer = time_ms;
			
			if (_habituation_pos >= (0.01f * _habituation_speed))
			{
				_habituation_pos -= (0.01f * _habituation_speed);
			}
			else if (_habituation_pos <= (0.01f * _habituation_speed))
			{
				_habituation_pos += (0.01f * _habituation_speed);
			}
			if (_habituation_neg >= (0.01f * _habituation_speed))
			{
				_habituation_neg -= (0.01f * _habituation_speed);
			}
			else if (_habituation_neg <= (0.01f * _habituation_speed))
			{
				_habituation_neg += (0.01f * _habituation_speed);
			}
		}

		_valence_target = 0.0f;
		_arousal_target = 0.0f;

		ssi_real_t user_valence = 0.0f;
		ssi_real_t user_arousal = 0.0f;
		ssi_size_t n_user_valence_values = 0;
		ssi_size_t n_user_arousal_values = 0;

		ssi_event_t* e = 0;
		
		for (ssi_size_t nevent = 0; nevent < n_new_events; nevent++) {

			e = events.next();

			if (e != 0) {

				if (e->type == SSI_ETYPE_MAP) {
					if (e->event_id == _user_valence_id) {

						ssi_event_map_t* tuples = ssi_pcast(ssi_event_map_t, e->ptr);
						ssi_size_t e_dim = (e->tot / (sizeof(ssi_event_map_t)));
						if (e_dim != 1) {
							ssi_print("\nEmoSim received Valence event with dim != 1");
						}
						user_valence += tuples[0].value;
						n_user_valence_values++;

					}
					else if (e->event_id == _user_arousal_id) {

						ssi_event_map_t* tuples = ssi_pcast(ssi_event_map_t, e->ptr);
						ssi_size_t e_dim = (e->tot / (sizeof(ssi_event_map_t)));
						if (e_dim != 1) {
							ssi_print("\nEmoSim received Arousal event with dim != 1");
						}
						user_arousal += tuples[0].value;
						n_user_arousal_values++;

					}
					else {

						ssi_print("\nEmoSim received map event not registered for Valence or Arousal\n(%d != %d || %d)", e->event_id, _user_valence_id, _user_arousal_id);

					}
				}
				else if (e->type == SSI_ETYPE_STRING) {

					if (e->event_id == _feedback_pos_id) {
						
						ResolveFeedback(true, 1.0f);
					}
					else if (e->event_id == _feedback_neg_id) {

						ResolveFeedback(false, 1.0f);
					}
					else {

						ssi_print("\nEmoSim received string event not registered for Positive or Negative Feedback\n(%d != %d || %d)", e->event_id, _user_valence_id, _user_arousal_id);

					}
				}
				else {

					ssi_print("\nEmoSim received event of other type than SSI_ETYPE_MAP or SSI_ETYPE_STRING");

				}
			}
		}

		if (time_ms - _cooldown_timer > 100) {

			_cooldown_timer = time_ms;
			
			if (_valence_base >= _valence_ocean + 0.01f * _target_approach_speed) {
				_valence_base -= (0.01f * _target_approach_speed);
			}
			else if (_valence_base <= _valence_ocean - 0.01f * _target_approach_speed) {
				_valence_base += (0.01f* _target_approach_speed);
			}
			if (_arousal_base >= _arousal_ocean + 0.01f * _target_approach_speed) {
				_arousal_base -= (0.01f * _target_approach_speed);
			}
			else if (_arousal_base <= _arousal_ocean - 0.01f * _target_approach_speed) {
				_arousal_base += (0.01f * _target_approach_speed);
			}
		}

		if (_valence_base < -1.0f) {
			_valence_base = -1.0f;
		}
		if (_valence_base > 1.0f) {
			_valence_base = 1.0f;
		}
		if (_arousal_base < -1.0f) {
			_arousal_base = -1.0f;
		}
		if (_arousal_base > 1.0f) {
			_arousal_base = 1.0f;
		}

		if (n_user_valence_values > 0) {
			_valence_user = user_valence / (float)n_user_valence_values;
		}
		if (n_user_arousal_values > 0) {
			_arousal_user = user_arousal / (float)n_user_arousal_values;
		}

		if (_options.auto_feedback_valence) {
			if (time_ms - _last_auto_feeback > _options.auto_feedback_timer) {
				_last_auto_feeback = time_ms;
				_auto_feedback_valence_value += _valence_user;
				_auto_feedback_valence_counter++;

				ssi_real_t feedback_valence = _auto_feedback_valence_value / (ssi_real_t)_auto_feedback_valence_counter;
				
				// ssi_print("\nDEBUG: feedback_valence = %.2f", feedback_valence);

				if (feedback_valence > 0.0f) {

					ResolveFeedback(true, abs(feedback_valence));
				}
				else {

					ResolveFeedback(false, abs(feedback_valence));
				}

				_auto_feedback_valence_value = 0.0f;
				_auto_feedback_valence_counter = 0;
			}
			else
			{
				_auto_feedback_valence_value += _valence_user;
				_auto_feedback_valence_counter++;
			}
		}

		ssi_real_t empathy_effect_valence = _valence_user * abs(_empathy_factor_valence);
		ssi_real_t empathy_effect_arousal = _arousal_user * abs(_empathy_factor_arousal);

		if (empathy_effect_valence < -1.0f) {
			empathy_effect_valence = -1.0f;
		}
		if (empathy_effect_valence > 1.0f) {
			empathy_effect_valence = 1.0f;
		}
		if (empathy_effect_arousal < -1.0f) {
			empathy_effect_arousal = -1.0f;
		}
		if (empathy_effect_arousal > 1.0f) {
			empathy_effect_arousal = 1.0f;
		}

		if (n_user_valence_values > 0) {
			if (_empathy_factor_valence >= 0.0f) {
				_valence_target = (_valence_base * (1.0f - abs(_empathy_factor_valence))) + empathy_effect_valence;
			}
			else {
				_valence_target = (_valence_base * (1.0f - abs(_empathy_factor_valence))) - empathy_effect_valence;
			}
		}
		else {
			_valence_target = _valence_target_save;
		}
		if (n_user_arousal_values > 0) {
			if (_empathy_factor_arousal >= 0.0f) {
				_arousal_target = (_arousal_base * (1.0f - abs(_empathy_factor_arousal))) + empathy_effect_arousal;
			}
			else {
				_arousal_target = (_arousal_base * (1.0f - abs(_empathy_factor_arousal))) - empathy_effect_arousal;
			}
		}
		else {
			_arousal_target = _arousal_target_save;
		}

		if (_valence_current >= _valence_target + 0.01f * _target_approach_speed) {
			_valence_current -= (0.01f * _target_approach_speed);
		}
		else if (_valence_current <= _valence_target - 0.01f * _target_approach_speed) {
			_valence_current += (0.01f * _target_approach_speed);
		}
		if (_arousal_current >= _arousal_target + 0.01f * _target_approach_speed) {
			_arousal_current -= (0.01f * _target_approach_speed);
		}
		else if (_arousal_current <= _arousal_target - 0.01f * _target_approach_speed) {
			_arousal_current += (0.01f * _target_approach_speed);
		}

		_valence_target_save = _valence_target;
		_arousal_target_save = _arousal_target;

		if(_plot && _window && _canvas) {

			ssi_char_t emo[SSI_MAX_CHAR];
			convertToEmotionLabel(emo, _valence_current, _arousal_current);
			ssi_char_t emo_user[SSI_MAX_CHAR];
			convertToEmotionLabel(emo_user, _valence_user, _arousal_user);
			ssi_char_t emo_base[SSI_MAX_CHAR];
			convertToEmotionLabel(emo_base, _valence_base, _arousal_base);

			_plot->setData(emo, emo_user, emo_base,
							_valence_current, _arousal_current,
							_valence_user, _arousal_user,
							_empathy_factor_valence, _empathy_factor_arousal,
							_valence_base, _arousal_base,
							_habituation_pos, _habituation_neg,
							_options.openness, _options.conscientiousness, _options.extraversion, _options.agreeableness, _options.neuroticism);

			_window->update();

		}

		if (_listener) {

			if (_update_counter * _update_ms <= time_ms) {

				_event.dur = time_ms - _event.time;
				_event.time = time_ms;
				ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
				ssi_sprint(string, "valence_agent");
				e[0].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
				e[0].value = _valence_current;
				ssi_sprint(string, "arousal_agent");
				e[1].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
				e[1].value = _arousal_current;
				ssi_sprint(string, "valence_user");
				e[2].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
				e[2].value = _valence_user;
				ssi_sprint(string, "arousal_user");
				e[3].id = Factory::AddString(string); // TODO: einmalig in listen_enter registrieren und id als variable speichern
				e[3].value = _arousal_user;
				
				_listener->update(_event);

				_update_counter++;
			}

		}

		return true;
	}

	void EmoSim::listen_flush() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		if (_listener) {
			ssi_event_reset(_event);
		}

		if (_plot) {
			_window->close();
			delete _plot;
			_plot = 0;
			delete _window;
			_window = 0;
			delete _canvas;
			_canvas = 0;
		}
	}

	void EmoSim::convertToEmotionLabel(ssi_char_t* emo_label, ssi_real_t valenceValue, ssi_real_t arousalValue)
	{
		if (valenceValue >= 0.25f)
		{
			if (valenceValue >= 0.5f)
			{
				if (arousalValue >= 0.25f)
				{
					if (arousalValue >= 0.5f)
					{
						// valenceValue veryHigh && arousalValue veryHigh
						ssi_sprint(emo_label, "happy");
					}
					else
					{
						// valenceValue veryHigh && arousalValue high
						ssi_sprint(emo_label, "pleased");
					}
				}
				else if (arousalValue <= -0.25f)
				{
					if (arousalValue <= -0.5f)
					{
						// valenceValue veryHigh && arousalValue veryLow  
						ssi_sprint(emo_label, "relaxed");
					}
					else
					{
						// valenceValue veryHigh && arousalValue low
						ssi_sprint(emo_label, "serene");
					}
				}
				else
				{
					// valenceValue veryHigh && arousalValue neutral
					ssi_sprint(emo_label, "satisfied");
				}
			}
			else
			{
				if (arousalValue >= 0.25f)
				{
					if (arousalValue >= 0.5f)
					{
						// valenceValue high && arousalValue veryHigh
						ssi_sprint(emo_label, "excited");
					}
					else
					{
						// valenceValue high && arousalValue high
						ssi_sprint(emo_label, "amused");
					}
				}
				else if (arousalValue <= -0.25f)
				{
					if (arousalValue <= -0.5f)
					{
						// valenceValue high && arousalValue veryLow
						ssi_sprint(emo_label, "at ease");
					}
					else
					{
						// valenceValue high && arousalValue low
						ssi_sprint(emo_label, "calm");
					}
				}
				else
				{
					// valenceValue high && arousalValue neutral
					ssi_sprint(emo_label, "content");
				}
			}
		}
		else if (valenceValue <= -0.25f)
		{
			if (valenceValue <= -0.5f)
			{
				if (arousalValue >= 0.25f)
				{
					if (arousalValue >= 0.5f)
					{
						// valenceValue veryLow && arousalValue veryHigh
						ssi_sprint(emo_label, "angry");
					}
					else
					{
						// valenceValue veryLow && arousalValue high
						ssi_sprint(emo_label, "frustrated");
					}
				}
				else if (arousalValue <= -0.25f)
				{
					if (arousalValue <= -0.5f)
					{
						// valenceValue veryLow && arousalValue veryLow
						ssi_sprint(emo_label, "depressed");
					}
					else
					{
						// valenceValue veryLow && arousalValue low
						ssi_sprint(emo_label, "miserable");
					}
				}
				else
				{
					// valenceValue veryLow && arousalValue neutral
					ssi_sprint(emo_label, "distressed");
				}
			}
			else
			{
				if (arousalValue >= 0.25f)
				{
					if (arousalValue >= 0.5f)
					{
						// valenceValue low && arousalValue veryHigh
						ssi_sprint(emo_label, "afraid");
					}
					else
					{
						// valenceValue low && arousalValue high
						ssi_sprint(emo_label, "annoyed");
					}
				}
				else if (arousalValue <= -0.25f)
				{
					if (arousalValue <= -0.5f)
					{
						// valenceValue low && arousalValue veryLow
						ssi_sprint(emo_label, "worried");
					}
					else
					{
						// valenceValue low && arousalValue low
						ssi_sprint(emo_label, "nervous");
					}
				}
				else
				{
					// valenceValue low && arousalValue neutral
					ssi_sprint(emo_label, "sad");
				}
			}
		}
		else
		{
			if (arousalValue >= 0.25f)
			{
				if (arousalValue >= 0.5f)
				{
					// valenceValue neutral && arousalValue veryHigh
					ssi_sprint(emo_label, "alarmed");
				}
				else
				{
					// valenceValue neutral && arousalValue high
					ssi_sprint(emo_label, "tense");
				}
			}
			else if (arousalValue <= -0.25f)
			{
				if (arousalValue <= -0.5f)
				{
					// valenceValue neutral && arousalValue veryLow
					ssi_sprint(emo_label, "hesitant");
				}
				else
				{
					// valenceValue neutral && arousalValue low
					ssi_sprint(emo_label, "anxious");
				}
			}
			else
			{
				// valenceValue neutral && arousalValue neutral
				ssi_sprint(emo_label, "neutral");
			}
		}
	}
}