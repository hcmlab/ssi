// OSPitchDirection.cpp
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2013/07/01
// Copyright (C) University of Augsburg, Ionut Damian
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

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

#include "OSPitchDirection.h"
#include "OSPitchSmoother.h"
#include "OSEnergy.h"
#include "OSLpc.h"
#include <sstream>

#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {
	
ssi_char_t OSPitchDirection::ssi_log_name[] = "osvad_____";

OSPitchDirection::OSPitchDirection (const ssi_char_t *file)
	:	_file (0),
		_elistener (0),
		//_has_voiceactivity(false),	
		//_ev_tstart(0),
		//_ev_dur(0),
		//_ev_tstart_old(0),
		stbuf(NULL), ltbuf(NULL), F0non0(0.0), lastF0non0(0.0),
		ltbsFrames(2),stbsFrames(1),
		stbufPtr(0), ltbufPtr(0),
		bufInit(0),
		insyl(0),
		f0cnt(0), lastE(0.0),
		startE(0.0), maxE(0.0), minE(0.0), endE(0.0),
		sylen(0), maxPos(0), minPos(0), sylenLast(0),
		inpPeriod(0.0),
		timeCnt(0.0), sylCnt(0), lastSyl(100.0),
		startF0(0.0), lastF0(0.0), maxF0(0.0), minF0(0.0),
		maxF0Pos(0), minF0Pos(0),
		longF0Avg(0.0),
		curSpkRate(0), nBuf0(0), nBuf1(0), nSyl0(0), nSyl1(0),
		_first_call(true) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	_meta_in_ptr = _meta_in;
	
	//ASSUMES that the order of the streams is pitch (with F0 and F0env) RMSEnergy
	//stream ids
	//pitch_streamId = 0; //main stream
	e_streamId = 0; //xtra stream

	ssi_event_init (_event, SSI_ETYPE_STRING, 0,0,0,0, 256);
}

OSPitchDirection::~OSPitchDirection () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	
	ssi_event_destroy (_event);
}

bool OSPitchDirection::setEventListener (IEventListener *listener) {
	
	_elistener = listener;

	_event.sender_id = Factory::AddString (_options.sname);
	_event.event_id = Factory::AddString (_options.ename);

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void OSPitchDirection::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	//ASSUMES that the order of the streams is pitch (f0, f0env), Energy
	ssi_real_t *ptr = _meta_in;
	OSPitchSmoother::meta_s *meta_pitch = ssi_pcast(OSPitchSmoother::meta_s, ptr);	ptr += sizeof(OSPitchSmoother::meta_s);
	OSEnergy::meta_s *meta_energy = ssi_pcast(OSEnergy::meta_s, ptr);				ptr += sizeof(OSEnergy::meta_s);

	if(meta_pitch->pos_F0final < 0 || meta_pitch->pos_F0finalEnv < 0 || meta_energy->pos_rms < 0)
		ssi_wrn("Required input not available for pitch direction computation");

	f0_offset = meta_pitch->pos_F0final;
	f0env_offset = meta_pitch->pos_F0finalEnv;
	e_offset = meta_energy->pos_rms;
}

void OSPitchDirection::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	//check to see if we have all the input we need
	if(xtra_stream_in_num != 1)
	{
		ssi_err("component requires 2 input streams: pitch and energy");
		return;
	}

	//check whether all streams are synchronized
	if(stream_in.num != xtra_stream_in[0].num)
	{
		ssi_err("input streams have different sample rates");
		return;
	}

	ssi_time_t time = info.time;
	ssi_time_t time_step = 1.0/stream_in.sr;

	if(_first_call)
	{
		stbsFrames = (long)ceil(_options.stbs / time_step);
		ltbsFrames = (long)ceil(_options.ltbs / time_step);

		// allocate memory for long/short term average buffers
		if (_options.stbs > 0) {
			stbuf = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*stbsFrames);
		}
		if (_options.ltbs > 0) {
			ltbuf = (FLOAT_DMEM*)calloc(1,sizeof(FLOAT_DMEM)*ltbsFrames);
		}

		_first_call = false;
	}

	ssi_size_t num = stream_in.num;
	for (ssi_size_t k = 0; k < num; k++) { 
			
		/*
		 * F0 (pitch) stream
		 * dim = 1
		 * OpenSmile: This is computed by pitchSmoother.cpp, field name "f0final"
		 * SSI: This is computed by ssi_transformer_OSPitchChain
		 */
		ssi_real_t *src_f0 = ssi_pcast (ssi_real_t, stream_in.ptr + k * stream_in.dim * stream_in.byte + f0_offset * stream_in.byte);
	
		/*
		 * F0env (pitch) stream
		 * dim = 1
		 * OpenSmile: This is computed by pitchSmoother.cpp, field name "f0finalenv"
		 * SSI: This is computed by ssi_transformer_OSPitchChain
		 */
		ssi_real_t *src_f0env = ssi_pcast (ssi_real_t, stream_in.ptr + k * stream_in.dim * stream_in.byte + f0env_offset * stream_in.byte);

		/*
		 * Energy stream
		 * dim = 1
		 * OpenSmile: This is computed by energy.cpp, field name "LOG"
		 * SSI: This is computed by ssi_feature_OSEnergy
		 */
		ssi_real_t *src_e = ssi_pcast (ssi_real_t, xtra_stream_in[e_streamId].ptr + k * xtra_stream_in[e_streamId].dim * xtra_stream_in[e_streamId].byte + e_offset * xtra_stream_in[e_streamId].byte);

		/*
		 * Output
		 */
		ssi_real_t *out = ssi_pcast (ssi_real_t, stream_out.ptr + k * stream_out.dim * stream_out.byte);


		// following code taken from openSMILE 1.0.1, vadV1.cpp
		// http://opensmile.sourceforge.net/

		/* actually process data... */
		ssi_msg(SSI_LOG_LEVEL_DEBUG,"processing value vector");

		// ALGO:
		// Rising/Falling pitch is determined at end of vowels (end of pitch regions)
		// using two moving window averages (long term and short term)
		// We use intensity/loudness, (F0env,) and F0 to determine vowel/syllable positions

		FLOAT_DMEM f0eNow = *src_f0env;
		FLOAT_DMEM f0now =  *src_f0;
		FLOAT_DMEM loudn = *src_e;
		
		/*
		#ifdef DEBUG
		float dbg[3];
		dbg[0] = (float)f0eNow;
		dbg[1] = (float)f0s;
		if (!insyl) dbg[1] = 0.0;
		dbg[2] = (float)loudn;
		//saveFloatVector_csv("pitchdir.dat",dbg,3,1);
		#endif
		*/
		// f0now == 0 ==> unvoiced

		if (f0now != 0.0) {
			lastF0non0 = F0non0;
			F0non0 = f0now;
		}

		/* -- speaking rate buffer cycling -- */
		if (nBuf0 < _options.speakingRateBsize) {
			nBuf0++; 
			if (nBuf0 == _options.speakingRateBsize) {
				curSpkRate = (double)nSyl0 / ( (double)nBuf0 * time_step); //OS used here the frame period of the recording instead of "time_step"
			}
		}
		if (nBuf1 < _options.speakingRateBsize*2) nBuf1++;
		else {
			curSpkRate = (double)nSyl1 / ( (double)nBuf1 * time_step); //OS used here the frame period of the recording instead of "time_step"
			nBuf1 -= nBuf0;
			nSyl1 -= nSyl0;
			nSyl0 = 0;
			nBuf0 = 0;
		}
		/* ---------*/

		// only if first syllable encountered and last syllable encountered not longer than X sec. ago
		lastSyl += inpPeriod;
		if (lastSyl < 0.8 ) { 
			timeCnt += inpPeriod;
		} else {
			timeCnt = 0.0;
		}

		long i; 
		// fill buffer:
		if (!bufInit) {
		stbuf[stbufPtr] = f0eNow;
		ltbuf[ltbufPtr] = f0eNow;
		if (++stbufPtr >= stbsFrames) stbufPtr = 0;

		if (++ltbufPtr >= ltbsFrames) {
			ltbufPtr = 0;
			bufInit = 1;
			// compute initial sums!
			ltSum=0.0; stSum=0.0;
			for (i=0; i<ltbsFrames; i++) {
				ltSum+=ltbuf[i];
			}
			for (i=0; i<stbsFrames; i++) {
				stSum+=stbuf[i];
			}
		}
		} else {
			// find pseudo-syllables (energetic voiced segments)
			if (!insyl) { // in unvoiced part
				if (f0now > 0.0) { // detect beginning
				if (f0cnt >= 1) {
					/* syl. start */

					if (nBuf0 < _options.speakingRateBsize) nSyl0++;  /* syllable rate counters */
					nSyl1++;  /* syllable rate counters */

					insyl = 1; sylen = f0cnt;
					f0cnt=0; lastSyl = 0.0;
					startF0 = (lastF0+f0now)*(FLOAT_DMEM)0.5;
					f0s = startF0;
					maxF0 = MAX(lastF0,f0now);
					minF0 = MIN(lastF0,f0now);
					maxF0Pos = 0;
					minF0Pos = 0;
					nFall=0; nRise=0; nFlat=0;
				}
				f0cnt++; 
				if (startE == 0.0) { 
					minE = maxE = startE = lastE;
				}
				} else { f0cnt = 0; startE = 0.0; maxE = 0.0; minE = 0.0; }
			} else { // in syllable part
				if (f0now <= 0.0) { // detect end
				if (f0cnt >= 1) {

					/* syl. end */
					// syllable energy verification:

					/*
					if (( 0.5*(startE+endE) < 0.975*maxE )&&( MIN(startE,endE) < 1.05*minE )) {
					printf("E-verify OK\n");
					}
					*/

					// other stuff:
					insyl = 0;

					if (sylen > 3) 
					{
						endE = lastE;
						sylenLast = sylen-f0cnt;
						f0cnt=0; 
						sylCnt++; lastSyl=0.0;
						/* //syllable rate computed with each new syllable
						if (timeCnt > 0.0) {
							printf("pseudo-syllable nucleus end (sylen=%i): syl rate = %f Hz\n",sylen,(double)sylCnt / timeCnt);
						}
						*/

						// pitch contour classification:
						FLOAT_DMEM endF0 = f0s; //0.5*(F0non0+lastF0non0);
						//lastF0 : end
						//startF0 : start
						// maxF0 / minF0 : start/end
						FLOAT_DMEM conf = (FLOAT_DMEM)sylen;
						if (conf > 10.0) conf = 10.0;
						conf *= 30.0;
						int score=0; int rf=0; int result = -1;

						// determine type of pitch rise/fall

						if (endF0 > (FLOAT_DMEM)pow(startF0,(FLOAT_DMEM)1.01)) {
							if (startF0 != 0.0) 
							score = (int)((endF0-startF0)/startF0*conf);
							if (score >= 1) {
							//printf("Syl: Rise %i\n",score);
							rf=1;
							result = 0;
							}
						}
						else if (endF0 < (FLOAT_DMEM)pow(startF0,(FLOAT_DMEM)(1.0/1.01))) {
							if (startF0 != 0.0) 
							score = (int)((startF0-endF0)/startF0*conf);
							if (score >= 1) {
								//printf("Syl: Fall %i\n",score);
								rf=1;
								result = 1;
							}
							//printf("Syl: Fall %i\n",(int)((startF0-endF0)/startF0*conf));
						}

						if ((!rf)&&(maxF0 > (FLOAT_DMEM)pow(endF0,(FLOAT_DMEM)1.01))&&(maxF0 > (FLOAT_DMEM)pow(startF0,(FLOAT_DMEM)1.01))) {
							if (startF0 != 0.0) 
								//printf("Syl: Rise->Fall %i , %i\n",(int)((maxF0-startF0)/startF0*conf),(int)((maxF0-endF0)/startF0*conf));
								if (result >= 0) {
									if (score < 15) result = 2;
								} else { result = 2; }
						}
						if ((!rf)&&(minF0 < (FLOAT_DMEM)pow(endF0,(FLOAT_DMEM)(1.0/1.01)))&&(minF0 < (FLOAT_DMEM)pow(startF0,(FLOAT_DMEM)(1.0/1.01)))) {
							if (startF0 != 0.0) 
								//printf("Syl: Fall->Rise %i , %i\n",(int)((startF0-minF0)/startF0*conf),(int)((endF0-minF0)/startF0*conf));
								if (result >= 0) {
									if (score < 15) result = 3;
								} else { result = 3; }
						}

						// check if result is in line with majority vote over syllable
						if ((result == 0)||(result==1)) {

							if ((nFall>nRise)&&(nFall>nFlat)) {
								//printf("MajVote: Falling\n");
								if (result == 0) result = -1;
							} else
							if ((nRise>nFall)&&(nRise>nFlat)) {
								//printf("MajVote: Rising\n");
								if (result == 1) result = -1;
							} else { 
								//printf("MajVote: Flat\n"); 
								result = -1; 
							}
						}

						// send result:
						if (result >= 0)  {
							if (result == 0) {
								ssi_msg(SSI_LOG_LEVEL_DEBUG, "  __^^__ pitch UP\n");
							} else if (result == 1) {
								ssi_msg(SSI_LOG_LEVEL_DEBUG, "  __vv__ pitch DOWN\n");
							}

							handleEvent(_elistener, &_event, result, time);
						}

					}
				}
				f0cnt++;
				} else { f0cnt = 0; }

				if (insyl)  // check if status has changed..
				{
					//monitor energy levels: loudness must be low at beginning and end, and higher in the middle
					if (loudn > maxE) { maxE = loudn; maxPos = sylen; }
					if (loudn < minE) { minE = loudn; minPos = sylen; }
					f0s = (FLOAT_DMEM)(0.5)*f0s + (FLOAT_DMEM)(0.5) * F0non0; /* smoothed F0 */
					if (f0s > maxF0) { maxF0 = f0s; maxF0Pos = sylen; }
					if (f0s < minF0) { minF0 = f0s; minF0Pos = sylen; }
					sylen++; lastSyl=0.0;
					if (longF0Avg == 0.0) longF0Avg = F0non0;
					longF0Avg = (FLOAT_DMEM)(0.02)*F0non0 + (FLOAT_DMEM)(0.98)*longF0Avg;

					double lmean = ltSum/(double)ltbsFrames;
					double smean = stSum/(double)stbsFrames;
					if (smean > pow(lmean,1.02)) {
						nRise++;
						//dir = 1.0;
					} else 
						if (smean < pow(lmean,1.0/1.02)) {
							nFall++;
							//dir = -1.0;
						} else {
							nFlat++;
							//dir = 0.0;
						}
				}
			}
			lastF0 = f0now;
			lastE = loudn;
		
		}

		FLOAT_DMEM dir = 0.0;
		double smean = 0.0, lmean = 0.0;

		if (insyl) //(f0now > 0.0) {
		{ 			
			//// cycle buffer and evaluate data
			// mean of ltbuf:
			ltSum -= (double)ltbuf[ltbufPtr];
			ltbuf[ltbufPtr] = f0s;
			ltSum += (double)f0s;
			if (++ltbufPtr >= ltbsFrames) ltbufPtr = 0;

			// mean of stbuf:
			stSum -= (double)stbuf[stbufPtr];
			stbuf[stbufPtr] = f0s;
			stSum += (double)f0s;
			if (++stbufPtr >= stbsFrames) stbufPtr = 0;

			// detect rising/falling pitch continuously
			lmean = ltSum/(double)ltbsFrames;
			smean = stSum/(double)stbsFrames;
			if (smean > pow(lmean,1.01)) {
				//printf("PITCH : rising\n"); 
				dir = 1.0;
			} else 
				if (smean < pow(lmean,1.0/1.01)) {
				//printf("PITCH : falling\n"); 
				dir = -1.0;
				} else {
				//printf("PITCH : flat\n"); 
				dir = 0.0;
				}
		}
		
		//OUTPUT
		if (_options.F0direction) {
			*out++ = dir;
		}
		if (_options.directionScore) { // smean-lmean
			*out++ = (FLOAT_DMEM)(smean-lmean);
		}
		if (_options.speakingRate) {
			*out++ = (FLOAT_DMEM)(curSpkRate); // TODO!!
		}
		if (_options.F0avg) {
			*out++ = (FLOAT_DMEM)( ltSum/(double)ltbsFrames );
		}
		if (_options.F0smooth) {
			*out++ = (float)f0s;
			//if (!insyl) { *out = (FLOAT_DMEM)(0.0); } //Johnny: disabled as it would cause the component to sometimes output 5 values, sometimes 6, possibly even overwriting other sutff in the memory. Not sure if intended
		}
		
		/*
		 * Handle events
		 */
		//if (_elistener && dir != 0.0f)
		//{
		//	_event.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
		//	_event.dur = 0;					
		//
		//	_event.state = SSI_ESTATE_COMPLETED;

		//	if(dir == -1.0f)
		//		strcpy(_event.ptr, "Fall" );
		//	else if(dir == 1.0f)
		//		strcpy(_event.ptr, "Rise" );

		//	_elistener->update (_event);
		//}

		//if we have multiple samples, we need to advance the timestamp
		time += time_step;
	}
}

void OSPitchDirection::transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

};

//bool OSPitchDirection::handleEvent(bool condition, ssi_time_t min_dur, ssi_time_t time, const char* ev_value, ssi_estate_t ev_state)
//{
//	if(condition)
//	{
//		//compute the duration of the new event
//		if(_ev_tstart == 0)
//		{
//			_ev_tstart = time;
//			_ev_dur = 0;
//		}
//		else
//		{
//			_ev_dur = time - _ev_tstart;
//		}
//
//		if (_elistener && _ev_dur > min_dur) //only trigger once the condition has been fulfilled long enough
//		{
//			_event.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
//			_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _ev_tstart_old) + 0.5);					
//		
//			_event.state = ev_state;
//			strcpy(_event.ptr, ev_value );
//			_elistener->update (_event);
//
//			//set the time for the start of the current active event (this event)
//			_ev_tstart_old = time;
//
//			//reset variables
//			_ev_tstart = 0;
//			_ev_dur = 0;
//
//			return true;
//		}
//	}
//	else
//	{
//		_ev_tstart = 0;
//		_ev_dur = 0;
//	}
//
//	return false;
//}

bool OSPitchDirection::handleEvent(IEventListener *listener, ssi_event_t* ev, ssi_size_t class_id, ssi_time_t time)
{
	if (listener)
	{
		switch(class_id)
		{
		case 0:
			strcpy(ev->ptr, "Rise");
			break;			
		case 1:
			strcpy(ev->ptr, "Fall");
			break;
		case 2:
			strcpy(ev->ptr, "Rise-Fall");
			break;
		case 3:
			strcpy(ev->ptr, "Fall-Rise");
			break;
		}

		ev->time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
		ev->dur = 0;				
		ev->state = SSI_ESTATE_COMPLETED;			
		listener->update (*ev);

		return true;
	}	
	return false;
}



}



