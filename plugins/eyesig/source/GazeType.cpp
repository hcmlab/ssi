// GazeType.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/23
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

#include "GazeType.h"
#include "base/Factory.h"

#include "ssiocv.h"

#include <math.h>
#include <deque>

namespace ssi {


std::deque<CvSeq*> samples; 


GazeType::GazeType (const ssi_char_t *file) 
	: _file (0),
	_elistener (0),
	tempglueidface(0),
	tempglueidobject(0),
	facetracked(false),
	businessgazetracked(false),
	socialgazetracked(false),
	intimategazetracked(false),
	lookatface(false)
	{
	
	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
		
	}

	ssi_event_init (_ev_businessgaze, SSI_ETYPE_EMPTY);	
	ssi_event_init (_ev_socialgaze, SSI_ETYPE_EMPTY);	
	ssi_event_init (_ev_intimategaze, SSI_ETYPE_EMPTY);	
	ssi_event_init (_ev_face, SSI_ETYPE_EMPTY);		
}

GazeType::~GazeType () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	ssi_event_destroy (_ev_businessgaze);	
	ssi_event_destroy (_ev_socialgaze);	
	ssi_event_destroy (_ev_intimategaze);	
	ssi_event_destroy (_ev_face);	
}

  
bool GazeType::setEventListener (IEventListener *listener) {

	_elistener = listener;

	_ev_businessgaze.sender_id = Factory::AddString (_options.sname);
	_ev_intimategaze.sender_id = Factory::AddString (_options.sname);
	_ev_socialgaze.sender_id = Factory::AddString (_options.sname);
	_ev_face.sender_id = Factory::AddString (_options.sname);
	
	_ev_businessgaze.event_id = Factory::AddString ("BusinessGaze");
	_ev_intimategaze.event_id = Factory::AddString ("IntimateGaze");
	_ev_socialgaze.event_id = Factory::AddString ("SocialGaze");
	_ev_face.event_id = Factory::AddString ("GazeAtFace");

	std::stringstream enames;
	enames << _options.ename << ",";
	enames << "BusinessGaze,IntimateGaze,SocialGaze,GazeAtFace";

	_event_address.setSender (_options.sname);
	_event_address.setEvents (enames.str().c_str());	
	return true;
}

void GazeType::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	tempdurface = 0;
	tempglueidface = 0;
}


float sign(cv::Point2f p1, cv::Point2f p2, cv::Point2f p3)
{
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle(cv::Point2f pt, cv::Point2f v1, cv::Point2f v2, cv::Point2f v3)
{
  bool b1, b2, b3;

  b1 = sign(pt, v1, v2) < 0.0f;
  b2 = sign(pt, v2, v3) < 0.0f;
  b3 = sign(pt, v3, v1) < 0.0f;

  return ((b1 == b2) && (b2 == b3));
}


void GazeType::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_time_t time = info.time;
	ssi_time_t start = info.time;
	ssi_size_t start_ms = ssi_cast (ssi_size_t, 1000 * info.time + 0.5);
	ssi_time_t step = 1.0 / stream_in.sr;

	ssi_size_t durFace = 0;
	ssi_size_t durBusiness = 0;
	ssi_size_t durSocial = 0;
	ssi_size_t durIntimate = 0;

	for(ssi_size_t k=0; k< stream_in.num; ++k)
	{				
		ssi_size_t *ptr =  ssi_pcast (ssi_size_t, stream_in.ptr + k * stream_in.dim * stream_in.byte);
		cv::Point2f p(*ptr, *(ptr+1));

		ShoreTools::shore_s *ss = ssi_pcast (ShoreTools::shore_s, xtra_stream_in[0].ptr + k * xtra_stream_in[0].dim * xtra_stream_in[0].byte);

		if(_options.detectFaces)
		{
			bool inyspace;
			//Data from SMI Glasses is flipped, compared to a webcam
			inyspace =  p.y < ss->imageHeight - ss->top && p.y > ss->imageHeight - ss->bottom;
			lookatface = p.x > ss->left && p.x < ss->right && inyspace;
		}

		facetrackinglost = ss->left == -1;

		if (_elistener) 
		{	
			if(_options.detectGazeKind)
			{
				cv::Point2f leye;
				cv::Point2f reye;
				cv::Point2f leyeext;
				cv::Point2f reyeext; 
				cv::Point2f head; 
				cv::Point2f mouth; 
				cv::Point2f chest;

				/*leye = cv::Point(ss->leftEye_X, (ss->imageHeight - ss->leftEye_Y));
				reye = cv::Point(ss->rightEye_X, (ss->imageHeight -  ss->rightEye_Y));

				leyeext = cv::Point(ss->leftEye_X +ss->width/20, (ss->imageHeight - ss->leftEye_Y));
				reyeext = cv::Point(ss->rightEye_X - ss->width/20, (ss->imageHeight -  ss->rightEye_Y));

				head = cv::Point((ss->rightEye_X+ss->leftEye_X) /2, (ss->imageHeight -  ss->rightEye_Y+ ss->imageHeight/10));
				mouth = cv::Point((ss->leftMouthCorner_X+ss->rightMouthCorner_X) /2, (ss->imageHeight -  ss->leftMouthCorner_Y- ss->imageHeight/20));

				chest = cv::Point((ss->leftMouthCorner_X+ss->rightMouthCorner_X) /2, (ss->imageHeight -  ss->leftMouthCorner_Y- ss->imageHeight/3));*/		

				if(_options.dataIsFlipped)
				{
					leye = cv::Point(ss->leftEye_X, (ss->imageHeight - ss->leftEye_Y));
					reye = cv::Point(ss->rightEye_X, (ss->imageHeight -  ss->rightEye_Y));

					leyeext = cv::Point(ss->leftEye_X +ss->width/20, (ss->imageHeight - ss->leftEye_Y));
					reyeext = cv::Point(ss->rightEye_X - ss->width/20, (ss->imageHeight -  ss->rightEye_Y));

					head = cv::Point((ss->rightEye_X+ss->leftEye_X) /2, (ss->imageHeight -  ss->rightEye_Y+ ss->imageHeight/10));
					mouth = cv::Point((ss->leftMouthCorner_X+ss->rightMouthCorner_X) /2, (ss->imageHeight -  ss->leftMouthCorner_Y- ss->imageHeight/20));

					chest = cv::Point((ss->leftMouthCorner_X+ss->rightMouthCorner_X) /2, (ss->imageHeight -  ss->leftMouthCorner_Y- ss->imageHeight/3));
				}
				else
				{
					leye = cv::Point(ss->leftEye_X, ss->leftEye_Y);
					reye = cv::Point(ss->rightEye_X,ss->rightEye_Y);

					leyeext = cv::Point(ss->leftEye_X +ss->width/20, ss->leftEye_Y);
					reyeext = cv::Point(ss->rightEye_X - ss->width/20,  ss->rightEye_Y);

					head = cv::Point((ss->rightEye_X+ss->leftEye_X) /2, ss->rightEye_Y - ss->imageHeight/10);

					mouth = cv::Point((ss->leftMouthCorner_X+ss->rightMouthCorner_X) /2, (ss->leftMouthCorner_Y+ ss->imageHeight/20));
					chest = cv::Point((ss->leftMouthCorner_X+ss->rightMouthCorner_X) /2, (ss->leftMouthCorner_Y+ ss->imageHeight/3));
				}

				if(!businessgazetracked)
				{
					if(PointInTriangle(p,head,reye,leye) == true)			
					{		
						_ev_businessgaze.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						//_event.event_id = Factory::AddString ("GazeKind");
						_ev_businessgaze.state =  SSI_ESTATE_CONTINUED;
						_ev_businessgaze.glue_id = tempglueidface;
						_ev_businessgaze.dur = 0;
						_elistener->update (_ev_businessgaze);
						tempdurbusiness = _ev_businessgaze.time;
						businessgazetracked = true;
					}	
				}

				else if(businessgazetracked && !facetrackinglost)
				{
					if(PointInTriangle(p,head,reye,leye) == false)			
					{
						_ev_businessgaze.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						_ev_businessgaze.dur = _ev_businessgaze.time - tempdurbusiness;		
						_ev_businessgaze.state =  SSI_ESTATE_COMPLETED;
						_ev_businessgaze.glue_id = tempglueidface++;
						_elistener->update (_ev_businessgaze);
						businessgazetracked = false;	
						durBusiness += (tempdurbusiness < start_ms) ? _ev_businessgaze.time - start_ms : _ev_businessgaze.dur;
					}
				}

				if(!socialgazetracked)
				{
					if(PointInTriangle(p,mouth,reye,leye) == true)			
					{		
						_ev_socialgaze.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						//_event.event_id = Factory::AddString ("GazeKind");
						_ev_socialgaze.state =  SSI_ESTATE_CONTINUED;
						_ev_socialgaze.glue_id = tempglueidface;
						_ev_socialgaze.dur = 0;
						_elistener->update (_ev_socialgaze);
						tempdursocial = _ev_socialgaze.time;
						socialgazetracked = true;								
					}	
				}

				else if(socialgazetracked && !facetrackinglost)
				{
					if(PointInTriangle(p,mouth,reye,leye) == false)			
					{ 
						_ev_socialgaze.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						_ev_socialgaze.dur = _ev_socialgaze.time - tempdursocial   ;		
						_ev_socialgaze.state =  SSI_ESTATE_COMPLETED;
						_ev_socialgaze.glue_id = tempglueidface++;
						_elistener->update (_ev_socialgaze);
						socialgazetracked = false;	
						durSocial += (tempdursocial < start_ms) ? _ev_socialgaze.time - start_ms : _ev_socialgaze.dur;
					}
				}


				if(!intimategazetracked)
				{
					if(PointInTriangle(p,chest,reyeext,leyeext) == true && PointInTriangle(p,mouth,reye,leye) == false)			
					{		
						_ev_intimategaze.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						//_ev_intimategaze.event_id = Factory::AddString ("GazeKind");
						_ev_intimategaze.state =  SSI_ESTATE_CONTINUED;
						_ev_intimategaze.glue_id = tempglueidface;
						_ev_intimategaze.dur = 0;
						_elistener->update (_ev_intimategaze);
						tempdurintimate = _ev_intimategaze.time;
						intimategazetracked = true;								
					}	
				}

				else if(intimategazetracked && !facetrackinglost)
				{
					if(PointInTriangle(p,chest,reyeext,leyeext) == false ||PointInTriangle(p,mouth,reye,leye) == true )			
					{
						_ev_intimategaze.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						_ev_intimategaze.dur = _ev_intimategaze.time - tempdurintimate;		
						_ev_intimategaze.state =  SSI_ESTATE_COMPLETED;
						_ev_intimategaze.glue_id = tempglueidface++;
						_elistener->update (_ev_intimategaze);
						intimategazetracked = false;	
						durIntimate += (tempdurintimate < start_ms) ? _ev_intimategaze.time - start_ms : _ev_intimategaze.dur;							
					}
				}
			}

			if(_options.detectFaces)
			{
				if(!facetracked)
				{
					if(lookatface)			
					{		
						_ev_face.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						//_event.event_id = Factory::AddString ("LookAtface");
						_ev_face.state =  SSI_ESTATE_CONTINUED;
						_ev_face.glue_id = tempglueidface;
						_ev_face.dur = 0;
						_elistener->update (_ev_face);
						tempdurface = _ev_face.time;
						facetracked = true;								
					}	
				}

				else if(facetracked && !facetrackinglost)
				{
					if(!lookatface)			
					{
						_ev_face.time = ssi_cast (ssi_size_t, 1000 * time + 0.5);
						_ev_face.dur = _ev_face.time - tempdurface;		
						_ev_face.state =  SSI_ESTATE_COMPLETED;
						_ev_face.glue_id = tempglueidface++;
						_elistener->update (_ev_face);
						facetracked = false;
						durFace += (tempdurface < start_ms) ? _ev_face.time - start_ms : _ev_face.dur;	
					}
				}
			}		
		}
		time += step;
	}

	//determine duration of gazes
	ssi_time_t tFace, tBusiness, tSocial, tIntimate;
	ssi_time_t frameSize = time - start;

	if(facetracked && tempdurface < start_ms && durFace == 0) tFace = frameSize;
	else tFace = durFace / 1000.0;
	if(businessgazetracked && tempdurbusiness < start_ms && durBusiness == 0) tBusiness = frameSize;
	else tBusiness = durBusiness / 1000.0;
	if(socialgazetracked && tempdursocial < start_ms && durSocial == 0) tSocial = frameSize;
	else tSocial = durSocial / 1000.0;
	if(intimategazetracked && tempdurintimate < start_ms && durIntimate == 0) tIntimate = frameSize;
	else tIntimate = durIntimate / 1000.0;

	if(_options.percent)
	{
		tFace /= frameSize;
		tBusiness /= frameSize;
		tSocial /= frameSize;
		tIntimate /= frameSize;
	}

	/*
	 * Output
	 */
	//printf("face %d \t kind1 %d \t kind2 %d \t kind3 %d\n", tFaces, tGazeKinds[0], tGazeKinds[1], tGazeKinds[2]);
	ssi_real_t* out = ssi_pcast(ssi_real_t, stream_out.ptr);
	if(_options.detectFaces)
		*(out++) = ssi_cast(ssi_real_t, tFace);
	if(_options.detectGazeKind)	{ 
		*(out++) = ssi_cast(ssi_real_t, tBusiness);
		*(out++) = ssi_cast(ssi_real_t, tSocial);
		*(out++) = ssi_cast(ssi_real_t, tIntimate);
	}
}


void GazeType::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

}

}




	
