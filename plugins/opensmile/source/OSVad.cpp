// OSVad.cpp
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2012/10/16 
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
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

#include "OSVad.h"
#include "OSPitchSmoother.h"
#include "OSEnergy.h"
#include "OSLpc.h"

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
	
ssi_char_t OSVad::ssi_log_name[] = "osvad_____";

OSVad::OSVad (const ssi_char_t *file)
	:	_file (0),
		_elistener (0),
		_has_voiceactivity(false),	
		_ev_tstart(0),
		_ev_dur(0),
		_ev_tstart_old(0),
		spec(NULL), div0(0.0),
		t0histIdx(0),
		f0v_0(0.0), ent_0(0.0), E_0(0.0),
		nInit(0),
		uF0v(0.0), uEnt(0.0), uE(0.0),
		vF0v(0.0), vEnt(0.0), vE(0.0),
		tuF0v(0.0), tuEnt(0.0), tuE(0.0),
		tvF0v(0.0), tvEnt(0.0), tvE(0.0),
		vadFuzHidx(0), vadBin(0),
		F0vHidx(0), entHidx(0), EHidx(0),
		tF0vHidx(0), tentHidx(0), tEHidx(0),
		nInitT(0), nInitN(0),
		spec_offset(0), e_offset(0), voiceProb_offset(0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_meta_ptr = _meta;
	
	//ASSUMES that the order of the streams is LSP, Pitch, Energy
	//stream ids
	spec_streamId = 0; //LSP
	voiceProb_streamId = 1; //pitch
	e_streamId = 2; //energy
		
	ssi_event_init (_event, SSI_ETYPE_STRING, 0, 0, 0, 0, 256);
}

OSVad::~OSVad () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool OSVad::setEventListener (IEventListener *listener) {

	_elistener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void OSVad::consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {
	
	t0hist[0] = 0.0;
	t0hist[1] = 0.0;
	t0hist[2] = 0.0;
	t0hist[3] = 0.0;
  
	ar1 = (VAD_FLOAT)( 1.0 - exp(-10.0/20.0) ); // THIS DEPENDS ON THE INPUT FRAME RATE.. ASSUMING 10ms HERE!
	ar0 = (VAD_FLOAT)( 1.0 - exp(-10.0/200.0) ); // THIS DEPENDS ON THE INPUT FRAME RATE.. ASSUMING 10ms HERE!

	arU = (VAD_FLOAT)0.005; //(VAD_FLOAT)( 1.0 - exp(-10.0/100.0) ); // THIS DEPENDS ON THE INPUT FRAME RATE.. ASSUMING 10ms HERE!
	arV = (VAD_FLOAT)0.005; //(VAD_FLOAT)( 1.0 - exp(-10.0/500.0) ); // THIS DEPENDS ON THE INPUT FRAME RATE.. ASSUMING 10ms HERE!
  
	//ar1 = (VAD_FLOAT)( 1.0 - exp(-stream_in[0].sr/20.0) );
	//ar0 = (VAD_FLOAT)( 1.0 - exp(-stream_in[0].sr/200.0) );
	//arU = (VAD_FLOAT)( 1.0 - exp(-stream_in[0].sr/2000.0) );
	//arV = (VAD_FLOAT)( 1.0 - exp(-stream_in[0].sr/2000.0) );

	for (ssi_size_t i=0; i<SSI_OPENSMILE_VAD_FUZBUF; i++) {
		vadFuzH[i] = 0.0;
	}

	for(ssi_size_t i=0; i<SSI_OPENSMILE_VAD_FTBUF; i++) {
		F0vH[i] = 0;
		entH[i] = 0;
		EH[i] = 0;
	}

	minE = _options.threshold;
	
	//ASSUMES that the order of the streams is LSP, Pitch, Energy
	ssi_real_t *ptr = _meta;
	OSLpc::meta_s *meta_lpc = ssi_pcast(OSLpc::meta_s, ptr);						ptr += sizeof(OSLpc::meta_s);
	OSPitchSmoother::meta_s *meta_pitch = ssi_pcast(OSPitchSmoother::meta_s, ptr);	ptr += sizeof(OSPitchSmoother::meta_s);
	OSEnergy::meta_s *meta_energy = ssi_pcast(OSEnergy::meta_s, ptr);				ptr += sizeof(OSEnergy::meta_s);

	spec_offset = meta_lpc->pos_lsp;
	voiceProb_offset = meta_pitch->pos_voicingC1;
	e_offset = meta_energy->pos_log;
	
	specN = meta_lpc->numCoeff;
	spec = (VAD_FLOAT*)calloc(1,sizeof(VAD_FLOAT)*(specN));
	for(int i=0; i<specN; i++) {
		spec[i] = ( (VAD_FLOAT)i * ((VAD_FLOAT)3.0/(VAD_FLOAT)specN) + (VAD_FLOAT)0.2 ); 
	}
}

VAD_FLOAT OSVad::pitchVariance(VAD_FLOAT curF0raw)
{
	// convert to period in seconds:
	VAD_FLOAT Tp=curF0raw;
	//if (curF0raw != 0.0) Tp = 1.0/curF0raw;

	// add new value to history:
	t0hist[t0histIdx++] = Tp;
	if (t0histIdx >= 8) t0histIdx = 0;

	// mean over 8 frames:
	VAD_FLOAT m = (VAD_FLOAT)0.125 * (t0hist[0]+t0hist[1]+t0hist[2]+t0hist[3]+t0hist[4]+t0hist[5]+t0hist[6]+t0hist[7]);
	//VAD_FLOAT m = 0.125 * (t0hist[0]+t0hist[1]+t0hist[2]+t0hist[3]);

	int i;
	VAD_FLOAT v = (VAD_FLOAT)0.0;
	for (i=0; i<8; i++) {
		v += (t0hist[i] - m) * (t0hist[i] - m);
	}

	return (VAD_FLOAT)sqrt((double)v/8.0);
}

void OSVad::consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

	bool vadResult = false;

	//check whether all streams are synchronized
	if((stream_in[0].num != stream_in[1].num) || (stream_in[0].num != stream_in[2].num) || (stream_in[1].num != stream_in[2].num) )
	{
		ssi_err("input streams have different sample rates");
		return;
	}

	ssi_time_t time = consume_info.time;
	ssi_time_t time_step = 1.0/stream_in[0].sr;

	ssi_size_t num = stream_in[0].num;
	for (ssi_size_t k = 0; k < num; k++) { 

		/*
		 * Spec (LSP) stream
		 * dim = specN
		 * OpenSmile: This is computed by lsp.cpp, field name "lspFreq"
		 * SSI: This is computed by ssi_feature_OSLpc, requires option "lsp"
		 */
		long lsfN = specN; // not sure about this but the OS code is just so bad I don't think I'll ever be sure about anything in this project
		ssi_real_t *src_spec = ssi_pcast (ssi_real_t, stream_in[spec_streamId].ptr + k * stream_in[spec_streamId].dim * stream_in[spec_streamId].byte + spec_offset * stream_in[spec_streamId].byte);
	
		/*
		 * Voice prob (pitch) stream
		 * dim = 1
		 * OpenSmile: This is computed by pitchACF.cpp, field name "voiceProb"
		 * SSI: This is computed by ssi_transformer_OSPitchChain
		 */
		ssi_real_t *src_voiceProb = ssi_pcast (ssi_real_t, stream_in[voiceProb_streamId].ptr + k * stream_in[voiceProb_streamId].dim * stream_in[voiceProb_streamId].byte + voiceProb_offset * stream_in[voiceProb_streamId].byte);
	
		/*
		 * Energy stream
		 * dim = 1
		 * OpenSmile: This is computed by energy.cpp, field name "LOG"
		 * SSI: This is computed by ssi_feature_OSEnergy
		 */
		ssi_real_t *src_e = ssi_pcast (ssi_real_t, stream_in[e_streamId].ptr + k * stream_in[e_streamId].dim * stream_in[e_streamId].byte + e_offset * stream_in[e_streamId].byte);

		/*
		 * Output
		 */
		ssi_real_t dst[3];


		// following code taken from openSMILE 1.0.1, vadV1.cpp
		// http://opensmile.sourceforge.net/

		/*
		// get LSF difference vector:
		if (lsfIdx>=0) {
		int j=0, i;
		for(i=lsfIdx; i<lsfIdx+lsfN-1; i++) {
			//lsf[j] = src[i+1]-src[i];
			lsf[j] = src[i];
			printf("lsf[%i] %f \n",j,lsf[j]);
			j++;
		}
		//lsf[j] = 3.15 - src[i];
		lsf[j] = src[i];
		printf("lsf[%i] %f \n",j,lsf[j]);
		} else { printf("no lsf!\n"); }
		*/

		//printf("sepcidx %i specN %i\n",specIdx,specN);
		int j=0; int i;
		
		/*
		 * Get Energy
		 */
		VAD_FLOAT E = 0.0;
		if (e_offset >= 0) {
			E = *(src_e);
		}
		// check if disableDynamicVAD is set
		if (_options.disableDynamicVAD) {
			if (E > _options.threshold) {
				dst[0] = 1.0;
				dst[1] = 1.0;
				dst[2] = 1.0;
			} else {
				dst[0] = 0.0;
				dst[1] = 0.0;
				dst[2] = 0.0;
			}		
		}
		else {			

			/*
			 * Get Spec (LSP)
			 */
			// compute LSP diversion
			VAD_FLOAT div = 0.0;
			for(i=0; i<specN; i++) {
				//VAD_FLOAT x = ( (VAD_FLOAT)i * (3.0/(VAD_FLOAT)specN) + 0.14 ) - spec[i]; 
				VAD_FLOAT x = spec[i] - src_spec[i]; 
				div += x*x;
			}
			//printf ("LSF div: %f\n",div);
			//div /= specN;
			//div = sqrt(div);
			//printf ("LSF div: %f\n",div);

			// compute spectral entropy:
			VAD_FLOAT ent = smileStat_entropy(src_spec, lsfN);

			// compute pitch variance
			VAD_FLOAT f0v=0.0;
			// if (F0rawIdx >= 0) {
			//printf("f0raw %f [%i]\n",src[F0rawIdx],F0rawIdx);
			//   f0v = pitchVariance(src[F0rawIdx]);
			// }
			f0v=div;

			/*
			 * Get voicing probability (pitch)
			 */
			VAD_FLOAT f0prob=0.0;
			if (voiceProb_offset >= 0) {
				//printf("f0raw %f [%i]\n"s,src[F0rawIdx],F0rawIdx);
				f0prob = *src_voiceProb;
			}

			//ssi_msg(SSI_LOG_LEVEL_BASIC, "LSP %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f", src_spec[0], src_spec[1], src_spec[2], src_spec[3], src_spec[4], src_spec[5], src_spec[6], src_spec[7]);
			//ssi_msg(SSI_LOG_LEVEL_BASIC, "E %.2f, VoiceProb %.2f, LSP_div %.2f, LSP_ent %.2f", E, f0prob, div, ent);

			// now smooth the 3 contours
			if (ent > ent_0) {
				ent = ar0 * (ent - ent_0) + ent_0;
			} else {
				ent = ar1 * (ent - ent_0) + ent_0;
			}

			if (f0v > f0v_0) {
				f0v = ar0 * (f0v - f0v_0) + f0v_0;  //f0v = ar * f0v + (1.0-ar) f0v_0;
			} else {
				f0v = ar1 * (f0v - f0v_0) + f0v_0;  //f0v = ar * f0v + (1.0-ar) f0v_0;
			}

			if (E < E_0) {
				E = ar0 * (E - E_0) + E_0;
			} else {
				E = ar1 * (E - E_0) + E_0;
			}

			ent_0 = ent;
			f0v_0 = f0v;
			E_0 = E;

			//int vadBin = 0;
			VAD_FLOAT vadFuz=0.0;
			VAD_FLOAT vadSmo=0.0;

			if ((nInit < SSI_OPENSMILE_VAD_NINIT)) { // prepare initial thresholds

				if (nInit > 10) {

					uF0v += f0v;
					uEnt += ent;
					uE   += E;
	
					F0vH[F0vHidx++] = f0v;
					entH[entHidx++] = ent;
					EH[EHidx++] = E;
				}
				nInit ++; 
				vadBin=0;

			} else {
				if (nInit == SSI_OPENSMILE_VAD_NINIT) { // compute inital thresholds
					VAD_FLOAT nn = ((VAD_FLOAT)nInit) - (VAD_FLOAT)10.0;

					// means
					uF0v /= nn;
					uEnt /= nn;
					uE   /= nn;

					// standard deviations:
					int i;
					for (i=0; i<nInit-10; i++) {
						vF0v += (F0vH[i] - uF0v) * (F0vH[i] - uF0v);
						vEnt += (entH[i] - uEnt) * (entH[i] - uEnt);
						vE   += (EH[i] - uE)     * (EH[i] - uE);
					}
					vF0v /= nn;
					vF0v = sqrt(vF0v);

					vEnt /= nn;
					vEnt = sqrt(vEnt);

					vE /= nn;
					vE = sqrt(vE);
					nInitN = nInit;
				}
	
				// compute thresholds
				VAD_FLOAT th1ent = uEnt;
				VAD_FLOAT th1e = uE;
				//VAD_FLOAT th1f0v = 1.0/72.0 * vF0v;
				VAD_FLOAT th1f0v = uF0v;

				VAD_FLOAT th2ent = uEnt - vEnt;
				VAD_FLOAT th2e = uE + vE;
				//VAD_FLOAT th2f0v = 1.0/36.0 * vF0v;
				VAD_FLOAT th2f0v = uF0v + vF0v;

				VAD_FLOAT th3ent = uEnt - (VAD_FLOAT)2.0*vEnt;
				VAD_FLOAT th3e = uE + (VAD_FLOAT)1.0*vE;
				//VAD_FLOAT th3f0v = 1.0/27.0 * vF0v;
				VAD_FLOAT th3f0v = uF0v + (VAD_FLOAT)2.0* vF0v;

				VAD_FLOAT th4ent = uEnt - (VAD_FLOAT)3.0*vEnt;
				VAD_FLOAT th4e = uE + (VAD_FLOAT)2.0*vE;
				//VAD_FLOAT th4f0v = 1.0/18.0 * vF0v;
				VAD_FLOAT th4f0v = uF0v + (VAD_FLOAT)3.0* vF0v;
	
				VAD_FLOAT th5ent = uEnt - (VAD_FLOAT)5.0*vEnt;
				VAD_FLOAT th5e = uE + (VAD_FLOAT)4.0*vE;
				//VAD_FLOAT th5f0v = 1.0/9.0 * vF0v;
				VAD_FLOAT th5f0v = uF0v + (VAD_FLOAT)5.0* vF0v;

	
				VAD_FLOAT tth0ent = tuEnt + (VAD_FLOAT)0.5*tvEnt;
				VAD_FLOAT tth0e = tuE - (VAD_FLOAT)0.1*tvE;
				//VAD_FLOAT th1f0v = 1.0/72.0 * vF0v;
				VAD_FLOAT tth0f0v = tuF0v - (VAD_FLOAT)0.5*tvF0v;

				VAD_FLOAT tth1ent = tuEnt - (VAD_FLOAT)0.5*tvEnt;
				VAD_FLOAT tth1e = tuE + (VAD_FLOAT)0.1*tvE;
				//VAD_FLOAT th1f0v = 1.0/72.0 * vF0v;
				VAD_FLOAT tth1f0v = tuF0v + (VAD_FLOAT)0.5*tvF0v;

				VAD_FLOAT tth2ent = tuEnt + (VAD_FLOAT)1.0*tvEnt;
				VAD_FLOAT tth2e = tuE - (VAD_FLOAT)0.5*tvE;
				//VAD_FLOAT th1f0v = 1.0/72.0 * vF0v;
				VAD_FLOAT tth2f0v = tuF0v - (VAD_FLOAT)2.0*tvF0v;

				VAD_FLOAT tth3ent = tuEnt + (VAD_FLOAT)3.0*tvEnt;
				VAD_FLOAT tth3e = tuE - (VAD_FLOAT)2.0*tvE;
				//VAD_FLOAT th1f0v = 1.0/72.0 * vF0v;
				VAD_FLOAT tth3f0v = tuF0v - (VAD_FLOAT)3.0*tvF0v;

  
				// perform VAD
				VAD_FLOAT vadEnt, vadE, vadF0v;

				if (ent < th5ent) { // inverse
					vadEnt = (VAD_FLOAT)1.0;
				} else if (ent < th4ent) {
					vadEnt = (VAD_FLOAT)0.8;
				} else if (ent < th3ent) {
					vadEnt = (VAD_FLOAT)0.6;
				} else if (ent < th2ent) {
					vadEnt = (VAD_FLOAT)0.4;
				} else if (ent < th1ent) {
					vadEnt = (VAD_FLOAT)0.2;
				} else { 
					vadEnt = (VAD_FLOAT)0.0;
				}

				if ((tuEnt > (VAD_FLOAT)0.0)&&(tth2ent<th4ent)) {
					if (ent > tth3ent) {
					vadEnt -= (VAD_FLOAT)0.3;
					} else if (ent > tth2ent) {
					vadEnt -= (VAD_FLOAT)0.2;
					} else if (ent < tth1ent) {
					vadEnt = (VAD_FLOAT)1.0;
					}  
				} else if ((tth2ent>th4ent)) {
					//vadEnt -= 0.2; // penalty, if turns are very equal to noise
				}


				if (vadEnt < (VAD_FLOAT)0.0) vadEnt = 0;
 
				if (E < th1e) {
					vadE = (VAD_FLOAT)0;
				} else if (E < th2e) {
					vadE = (VAD_FLOAT)0.2;
				} else if (E < th3e) {
					vadE = (VAD_FLOAT)0.4;
				} else if (E < th4e) {
					vadE = (VAD_FLOAT)0.6;
				} else if (E < th5e) {
					vadE = (VAD_FLOAT)0.8;
				} else { 
					vadE = (VAD_FLOAT)1.0;
				}

				if ((tuE > (VAD_FLOAT)0.0)&&(tth2e<th4e)) {
					if (E < tth3e) {
					vadE -= (VAD_FLOAT)0.2;
					} else if (E < tth2e) {
					vadE -= (VAD_FLOAT)0.2;
					} else if (E > tth1e) {
					vadE = (VAD_FLOAT)1.0;
					}  
				} else if ((tth2e<th4e)&&(tuE>(VAD_FLOAT)0.0)) {
					//vadE -= 0.2; // penalty, if turns are very equal to noise
				}
				if (vadE < (VAD_FLOAT)0.0) vadE = 0;


				if (f0v < th1f0v) {  
					vadF0v = (VAD_FLOAT)0.0;
				} else if (f0v < th2f0v) {
					vadF0v = (VAD_FLOAT)0.2;
				} else if (f0v < th3f0v) {
					vadF0v = (VAD_FLOAT)0.4;
				} else if (f0v < th4f0v) {
					vadF0v = (VAD_FLOAT)0.6;
				} else if (f0v < th5f0v) {
					vadF0v = (VAD_FLOAT)0.8;
				} else { 
					vadF0v = (VAD_FLOAT)1.0;
				}

				if ((tuF0v > (VAD_FLOAT)0.0)&&(tth2f0v<th4f0v)) {
					if (f0v < tth3f0v) {
					vadF0v -= (VAD_FLOAT)0.2;
					} else if (f0v < tth2f0v) {
					vadF0v -= (VAD_FLOAT)0.2;
					} else if (f0v > tth1f0v) {
					vadF0v = (VAD_FLOAT)1.0;
					}  
				} else if ((tth2f0v<th4f0v)&&(tuF0v>(VAD_FLOAT)0.0)) {
					//vadF0v -= 0.2; // penalty, if turns are very equal to noise
				}
				if (vadF0v < (VAD_FLOAT)0.0) vadF0v = 0;

				ssi_msg(SSI_LOG_LEVEL_DEBUG, "VADent\t %.2f\t %.2f\t %.2f\t %.2f\t | %.2f\t %.2f",vadEnt, ent, uEnt, vEnt, tuEnt, tvEnt);
				ssi_msg(SSI_LOG_LEVEL_DEBUG, "VADdiv\t %.2f\t %.2f\t %.2f\t %.2f\t | %.2f\t %.2f",vadF0v, f0v, uF0v, vF0v, tuF0v, tvF0v);
				ssi_msg(SSI_LOG_LEVEL_DEBUG, "VADE\t %.2e %.2e %.2e %.2e | %.2e %.2e",vadE, E, uE, vE, tuE, tvE);

				vadFuz = (VAD_FLOAT)0.45 * vadEnt + (VAD_FLOAT)0.25 * vadE +  (VAD_FLOAT)0.30 * vadF0v;
				//vadFuz = (VAD_FLOAT)0.55 * vadEnt + (VAD_FLOAT)0.45 * vadE  * vadF0v;

				//vadFuz = vadEnt * vadE  * vadF0v;

				// maintain history:
				vadFuzH[vadFuzHidx++] = vadFuz;
				if (vadFuzHidx >= SSI_OPENSMILE_VAD_FUZBUF) vadFuzHidx = 0;

				//printf("VADfuz %f\n",vadFuz);

				// determine vad bin:
				int i;
				VAD_FLOAT sum = 0.0;
				for (i=0; i<SSI_OPENSMILE_VAD_FUZBUF; i++) {
					sum += vadFuzH[i];
				}
				sum /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FUZBUF;

				vadSmo = sum;

				// ... the big magic ;-)
				if ((sum > (VAD_FLOAT)0.50)&&(E > minE)) {
					if (vadBin==0) { turnSum = 0.0; turnN=0.0; /*printf("turnstart\n");*/ }
					vadBin = 1;
					turnSum += sum; turnN += 1.0;
				} else {
					if (vadBin == 1) {
						//printf("turnConf = %f\n",turnSum/turnN);
					}
					vadBin = 0;
				}


				if ((vadBin == 0)&&(vadFuz < 0.5)) {    
					// dynamic threshold update:

					F0vH[F0vHidx++] = f0v;
					if (F0vHidx >= SSI_OPENSMILE_VAD_FTBUF) F0vHidx = 0;
					entH[entHidx++] = ent;
					if (entHidx >= SSI_OPENSMILE_VAD_FTBUF) entHidx = 0;
					EH[EHidx++] = E;
					if (EHidx >= SSI_OPENSMILE_VAD_FTBUF) EHidx = 0;
  
					if (nInit < SSI_OPENSMILE_VAD_FTBUF) nInit++;
					else {
						int i;
						VAD_FLOAT sumEnt=0.0;
						VAD_FLOAT sumF0v=0.0;
						VAD_FLOAT sumE=0.0;
						for (i=0; i< SSI_OPENSMILE_VAD_FTBUF; i++) {
							sumEnt += entH[i];
							sumE += EH[i];
							sumF0v += F0vH[i];
						}
						sumEnt /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
						sumE /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
						sumF0v /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;

						uEnt = ((VAD_FLOAT)1.0-arU) * uEnt + (arU)*sumEnt;
						uF0v = ((VAD_FLOAT)1.0-arU) * uF0v + (arU)*sumF0v;
						uE = ((VAD_FLOAT)1.0-arU) * uE + (arU)*sumE;

						// standard deviations:
						VAD_FLOAT stEnt=(VAD_FLOAT)0.0;
						VAD_FLOAT stF0v=(VAD_FLOAT)0.0;
						VAD_FLOAT stE=(VAD_FLOAT)0.0;
						for (i=0; i< SSI_OPENSMILE_VAD_FTBUF; i++) {
							stEnt += ( entH[i] - sumEnt ) * ( entH[i] - sumEnt );
							stE += ( EH[i] - sumE ) * ( EH[i] - sumE );
							stF0v += ( F0vH[i] - sumF0v ) * ( F0vH[i] - sumF0v );
						}
						stEnt /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
						stE /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
						stF0v /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;

						vEnt = ((VAD_FLOAT)1.0-arV) * vEnt + (arV)*sqrt(stEnt);
						vF0v = ((VAD_FLOAT)1.0-arV) * vF0v + (arV)*sqrt(stF0v);
						vE = ((VAD_FLOAT)1.0-arV) * vE + (arV)*sqrt(stE);
						//nInitN=0;
					}

					int j=0;
					for(i=0; i<specN; i++) {

						spec[j] = (VAD_FLOAT)0.995*spec[j] + (VAD_FLOAT)0.005*src_spec[i]; 
						j++;
					}


				} else if ((vadFuz > 0.6)&&(vadBin==1)&&(turnN>20.0)) {

					tF0vH[tF0vHidx++] = f0v;
					if (tF0vHidx >= SSI_OPENSMILE_VAD_FTBUF) tF0vHidx = 0;
					tentH[tentHidx++] = ent;
					if (tentHidx >= SSI_OPENSMILE_VAD_FTBUF) tentHidx = 0;
					tEH[tEHidx++] = E;
					if (tEHidx >= SSI_OPENSMILE_VAD_FTBUF) tEHidx = 0;

					if (nInitT < SSI_OPENSMILE_VAD_FTBUF) nInitT++;
					else {
					VAD_FLOAT sumEnt=0.0;
					VAD_FLOAT sumF0v=0.0;
					VAD_FLOAT sumE=0.0;
					for (i=0; i< SSI_OPENSMILE_VAD_FTBUF; i++) {
						sumEnt += tentH[i];
						sumE += tEH[i];
						sumF0v += tF0vH[i];
					}
					sumEnt /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
					sumE /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
					sumF0v /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;

					tuEnt = ((VAD_FLOAT)1.0-arU) * tuEnt + (arU)*sumEnt;
					tuF0v = ((VAD_FLOAT)1.0-arU) * tuF0v + (arU)*sumF0v;
					tuE = ((VAD_FLOAT)1.0-arU) * tuE + (arU)*sumE;

					// standard deviations:
					VAD_FLOAT stEnt=0.0;
					VAD_FLOAT stF0v=0.0;
					VAD_FLOAT stE=0.0;
					for (i=0; i< SSI_OPENSMILE_VAD_FTBUF; i++) {
						stEnt += ( tentH[i] - sumEnt ) * ( tentH[i] - sumEnt );
						stE += ( tEH[i] - sumE ) * ( tEH[i] - sumE );
						stF0v += ( tF0vH[i] - sumF0v ) * ( tF0vH[i] - sumF0v );
					}
					stEnt /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
					stE /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;
					stF0v /= (VAD_FLOAT)SSI_OPENSMILE_VAD_FTBUF;

					tvEnt = ((VAD_FLOAT)1.0-arV) * tvEnt + (arV)*sqrt(stEnt);
					tvF0v = ((VAD_FLOAT)1.0-arV) * tvF0v + (arV)*sqrt(stF0v);
					tvE = ((VAD_FLOAT)1.0-arV) * tvE + (arV)*sqrt(stE);
					//nInitT=0;
					}
				}
			}

			dst[0] = (VAD_FLOAT)vadBin;
			dst[1] = (VAD_FLOAT)vadFuz;
			dst[2] = (VAD_FLOAT)vadSmo;

			vadResult = (vadBin == 1) ? true : false;
		}

		ssi_msg(SSI_LOG_LEVEL_DEBUG, "VADbin %.2f VADFuz %.2f VADSmo %.2f", dst[0], dst[1], dst[2]);	
	
		/*
		 * Handle events
		 */
		if(!_has_voiceactivity)
			_has_voiceactivity = handleEvent(vadResult, _options.minvoicedur, time, "true", SSI_ESTATE_CONTINUED);
		else if(_has_voiceactivity)
			_has_voiceactivity = !handleEvent(!vadResult, _options.minsilencedur, time, "false", SSI_ESTATE_COMPLETED);
	
		//if we have multiple samples, we need to advance the timestamp
		time += time_step;
	}
}

bool OSVad::handleEvent(bool condition, ssi_time_t min_dur, ssi_time_t time, const char* ev_value, ssi_estate_t ev_state)
{
	if(condition)
	{
		//compute the duration of the new event
		if(_ev_tstart == 0)
		{
			_ev_tstart = time;
			_ev_dur = 0;
		}
		else
		{
			_ev_dur = time - _ev_tstart;
		}

		if (_elistener && _ev_dur > min_dur) //only trigger once the condition has been fulfilled long enough
		{
			_event.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
			_event.dur = ssi_cast (ssi_size_t, 1000 * (time - _ev_tstart_old) + 0.5);					
		
			_event.state = ev_state;
			strcpy(_event.ptr, ev_value );
			_elistener->update (_event);

			//set the time for the start of the current active event (this event)
			_ev_tstart_old = time;

			//reset variables
			_ev_tstart = 0;
			_ev_dur = 0;

			return true;
		}
	}
	else
	{
		_ev_tstart = 0;
		_ev_dur = 0;
	}

	return false;
}


}



