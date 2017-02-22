// DspFiltersTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/20
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

#include "DspFilters\Dsp.h"
#include "DspFiltersTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *DspFiltersTools::ssi_log_name = "dspftools_";
ssi_char_t DspFiltersTools::InfoString[] = "";

template <class DesignType, class StateType>
void DspFiltersTools::CreateFilterDesign (Dsp::Filter** pFilter, FilterProperty prop)
{
	if (prop.smooth > 0) {
		*pFilter = new Dsp::SmoothedFilterDesign <DesignType, 1, StateType> (prop.smooth);
	} else {
		*pFilter = new Dsp::FilterDesign <DesignType, 1, StateType>;
	}
}

template <class DesignType>
void DspFiltersTools::CreateFilterState (Dsp::Filter** pFilter, FilterProperty prop)
{		
	switch (ssi_cast (int, prop.state))
	{
		case 1: CreateFilterDesign <DesignType, Dsp::DirectFormI> (pFilter, prop); break;
		case 2: CreateFilterDesign <DesignType, Dsp::DirectFormII> (pFilter, prop); break;
		case 3: CreateFilterDesign <DesignType, Dsp::TransposedDirectFormI> (pFilter, prop); break;
		case 4: CreateFilterDesign <DesignType, Dsp::TransposedDirectFormII> (pFilter, prop); break;
		default:
			ssi_wrn ("unkown filter state '%d', set to 'DirectFormI'", prop.state);
			CreateFilterDesign <DesignType, Dsp::DirectFormI> (pFilter, prop);
	};
}

void DspFiltersTools::CreateFilter (Dsp::Filter** pFilter, FilterProperty prop)
{
	Dsp::Filter* &f = *pFilter;

	const int familyId = ssi_cast (int, prop.family);
	const int typeId = ssi_cast (int, prop.type);

	//
	// RBJ
	//
	if (familyId == 1)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::RBJ::Design::LowPass> (&f, prop); break;
			case 2: CreateFilterState <Dsp::RBJ::Design::HighPass> (&f, prop); break;
			case 3: CreateFilterState <Dsp::RBJ::Design::BandPass1> (&f, prop); break;
			case 4: CreateFilterState <Dsp::RBJ::Design::BandPass2> (&f, prop); break;
			case 5: CreateFilterState <Dsp::RBJ::Design::BandStop> (&f, prop); break;
			case 6: CreateFilterState <Dsp::RBJ::Design::LowShelf> (&f, prop); break;
			case 7: CreateFilterState <Dsp::RBJ::Design::HighShelf> (&f, prop); break;
			case 8: CreateFilterState <Dsp::RBJ::Design::BandShelf> (&f, prop); break;
			case 9: CreateFilterState <Dsp::RBJ::Design::AllPass> (&f, prop); break;
		};
	}
	//
	// Butterworth
	//
	else if (familyId == 2)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::Butterworth::Design::LowPass   <50> > (&f, prop); break;
			case 2: CreateFilterState <Dsp::Butterworth::Design::HighPass  <50> > (&f, prop); break;
			case 4: CreateFilterState <Dsp::Butterworth::Design::BandPass  <50> > (&f, prop); break;
			case 5: CreateFilterState <Dsp::Butterworth::Design::BandStop  <50> > (&f, prop); break;
			case 6: CreateFilterState <Dsp::Butterworth::Design::LowShelf  <50> > (&f, prop); break;
			case 7: CreateFilterState <Dsp::Butterworth::Design::HighShelf <50> > (&f, prop); break;
			case 8: CreateFilterState <Dsp::Butterworth::Design::BandShelf <50> > (&f, prop); break;
		};
	}
	//
	// Chebyshev I
	//
	else if (familyId == 3)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::ChebyshevI::Design::LowPass   <50> > (&f, prop); break;
			case 2: CreateFilterState <Dsp::ChebyshevI::Design::HighPass  <50> > (&f, prop); break;
			case 4: CreateFilterState <Dsp::ChebyshevI::Design::BandPass  <50> > (&f, prop); break;
			case 5: CreateFilterState <Dsp::ChebyshevI::Design::BandStop  <50> > (&f, prop); break;
			case 6: CreateFilterState <Dsp::ChebyshevI::Design::LowShelf  <50> > (&f, prop); break;
			case 7: CreateFilterState <Dsp::ChebyshevI::Design::HighShelf <50> > (&f, prop); break;
			case 8: CreateFilterState <Dsp::ChebyshevI::Design::BandShelf <50> > (&f, prop); break;
		};
	}
	//
	// Chebyshev II
	//
	else if (familyId == 4)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::ChebyshevII::Design::LowPass   <50> > (&f, prop); break;
			case 2: CreateFilterState <Dsp::ChebyshevII::Design::HighPass  <50> > (&f, prop); break;
			case 4: CreateFilterState <Dsp::ChebyshevII::Design::BandPass  <50> > (&f, prop); break;
			case 5: CreateFilterState <Dsp::ChebyshevII::Design::BandStop  <50> > (&f, prop); break;
			case 6: CreateFilterState <Dsp::ChebyshevII::Design::LowShelf  <50> > (&f, prop); break;
			case 7: CreateFilterState <Dsp::ChebyshevII::Design::HighShelf <50> > (&f, prop); break;
			case 8: CreateFilterState <Dsp::ChebyshevII::Design::BandShelf <50> > (&f, prop); break;
		};
	}
	//
	// Elliptic
	//
	else if (familyId == 5)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::Elliptic::Design::LowPass  <50> > (&f, prop); break;
			case 2: CreateFilterState <Dsp::Elliptic::Design::HighPass <50> > (&f, prop); break;
			case 4: CreateFilterState <Dsp::Elliptic::Design::BandPass <50> > (&f, prop); break;
			case 5: CreateFilterState <Dsp::Elliptic::Design::BandStop <50> > (&f, prop); break;
		};
	}
	//
	// Bessel
	//
	else if (familyId == 6)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::Bessel::Design::LowPass  <25> > (&f, prop); break;
			case 2: CreateFilterState <Dsp::Bessel::Design::HighPass <25> > (&f, prop); break;
			case 4: CreateFilterState <Dsp::Bessel::Design::BandPass <25> > (&f, prop); break;
			case 5: CreateFilterState <Dsp::Bessel::Design::BandStop <25> > (&f, prop); break;
			case 6: CreateFilterState <Dsp::Bessel::Design::LowShelf <25> > (&f, prop); break;
		};
	}
	//
	// Legendre
	//
	else if (familyId == 7)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::Legendre::Design::LowPass  <25> > (&f, prop); break;
			case 2: CreateFilterState <Dsp::Legendre::Design::HighPass <25> > (&f, prop); break;
			case 4: CreateFilterState <Dsp::Legendre::Design::BandPass <25> > (&f, prop); break;
			case 5: CreateFilterState <Dsp::Legendre::Design::BandStop <25> > (&f, prop); break;
		};
	}
	//
	// Custom
	//
	else if (familyId == 8)
	{
		switch (typeId)
		{
			case 1: CreateFilterState <Dsp::Custom::Design::TwoPole> (&f, prop); break;
			case 2: CreateFilterState <Dsp::Custom::Design::OnePole> (&f, prop); break;
		};
	} 

	if (f)
	{
		f->setParams (f->getDefaultParams ());
	} 
	else 
	{
		ssi_err ("sorry, your configuration is not supported '%s'", GetInfo (prop));
	}
}

const ssi_char_t *DspFiltersTools::GetInfo (FilterProperty prop) {

	InfoString[0] = '\0';
	size_t len = 0;

	switch (prop.family) {
		case RBJ:
			ssi_sprint (InfoString + len, "RBJ");
			break;
		case BUTTERWORTH:
			ssi_sprint (InfoString + len, "BUTTERWORTH");
			break;
		case CHEBYSHEV_I:
			ssi_sprint (InfoString + len, "CHEBYSHEV_I");
			break;
		case CHEBYSHEV_II:
			ssi_sprint (InfoString + len, "CHEBYSHEV_II");
			break;
		case ELLIPTIC:
			ssi_sprint (InfoString + len, "ELLIPTIC");
			break;
		case BESSEL:
			ssi_sprint (InfoString + len, "BESSEL");
			break;
		case LEGENDRE:
			ssi_sprint (InfoString + len, "LEGENDRE");
			break;
		case CUSTOM:
			ssi_sprint (InfoString + len, "CUSTOM");
			break;
	}

	len = strlen (InfoString);
	InfoString[len++] = ':';		

	switch (prop.type) {
		case LOWPASS:
			ssi_sprint (InfoString + len, "LOWPASS");
			break;
		case HIGHPASS:
			ssi_sprint (InfoString + len, "HIGHPASS");
			break;
		case BANDPASS1:
			ssi_sprint (InfoString + len, "BANDPASS1");
			break;
		case BANDPASS2:
			ssi_sprint (InfoString + len, "BANDPASS2");
			break;
		case BANDSTOP:
			ssi_sprint (InfoString + len, "BANDSTOP");
			break;
		case LOWSHELF:
			ssi_sprint (InfoString + len, "LOWSHELF");
			break;
		case HIGHSHELF:
			ssi_sprint (InfoString + len, "HIGHSHELF");
			break;
		case BANDSHELF:
			ssi_sprint (InfoString + len, "BANDSHELF");
			break;
		case ALLPASS:
			ssi_sprint (InfoString + len, "ALLPASS");
			break;
	}

	len = strlen (InfoString);
	InfoString[len++] = ':';	

	switch (prop.state) {
		case DIRECTFORM_I:
			ssi_sprint (InfoString + len, "DIRECTFORM_I");
			break;
		case DIRECTFORM_II:
			ssi_sprint (InfoString + len, "DIRECTFORM_II");
			break;
		case TRANSPOSEDDIRECTFORM_I:
			ssi_sprint (InfoString + len, "TRANSPOSEDDIRECTFORM_I");
			break;
		case TRANSPOSEDDIRECTFORM_II:
			ssi_sprint (InfoString + len, "TRANSPOSEDDIRECTFORM_II");
			break;
	}

	return InfoString;
}

}
