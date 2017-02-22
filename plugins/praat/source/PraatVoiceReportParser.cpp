// PraatVoiceReportParser.cpp
// author: Andreas Seiderer
// created: 2013/09/16
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
// version 3 of the License, or any later version.
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

#include "PraatVoiceReportParser.h"
#include "base/Factory.h"

#include <sstream>
#include <iostream>
#include <fstream>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

char *PraatVoiceReportParser::NAMES[SSI_PRAATVOICEREPORT_N_VALUES] = {
	"duration",
	"pitch_median", "pitch_mean", "pitch_std_dev", "pitch_min", "pitch_max",
	"pulses_number", "pulses_period_number", "pulses_mean_period", "pulses_std_dev_period",
	"voicing_fraction_unvoiced", "voicing_voice_breaks", "voicing_degree_voice_breaks",
	"jitter_local", "jitter_local_abs", "jitter_rap", "jitter_ppq5", "jitter_ddp",
	"shimmer_local", "shimmer_local_dB", "shimmer_apq3", "shimmer_apq5", "shimmer_apq11", "shimmer_dda",
	"harmonicity_autocorr", "harmonicity_noise_to_harmonics_ratio", "harmonicity_harmonics_to_noise_ratio"
};

PraatVoiceReportParser::PraatVoiceReportParser(ssi_real_t undefined_value)
	: _undefined_value(undefined_value) {

	for (ssi_size_t i = 0; i < SSI_PRAATVOICEREPORT_N_VALUES; i++) {
		_values[i] = 0;
	}
}

PraatVoiceReportParser::~PraatVoiceReportParser () {
}

//parser for "voice_report.praat"
bool PraatVoiceReportParser::parseValues (std::string input) {

	std::istringstream f(input);
	std::string line;

	int pos;
	ssi_real_t value;

	int valCount = 0;

	std::getline(f, line);
	if (line.find("Error") != std::string::npos)
	{
		ssi_wrn("Error in Praat: %s", line.c_str());
		return false;
	}

	do {
		//cout << line << endl;

		if (line.size() > 2 && line.at(line.size()-2) != ':') {

			pos=line.find(":")+2;
			int endpos = pos;
			for (unsigned int i = pos; i < line.size(); i++)	{
				if (isdigit(line.at(i)) || line.at(i) == '.') {
					endpos = i;
				}
				else {
					//handle scientific notation
					if (ssi_cast (int, line.size()) > endpos+1 && line.at(++endpos) == 'E') {
						endpos+=2;										//overjump E and sign
						while (isdigit(line.at(endpos))) 
							endpos++;
					}

					std::stringstream str; 
					str << line.substr(pos,endpos-pos);		//get number as string
					str >> value;							//convert string to double

					_values[valCount++] = value;

					break;
				}
			}
		}			
	} while (std::getline(f, line));

	SSI_ASSERT (valCount == 27);	//check if the parsed value count is correct  (unaltered voice report)

	return true;
}

}
