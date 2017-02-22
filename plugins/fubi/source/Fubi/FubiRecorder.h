// ****************************************************************************************
//
// Fubi FubiRecorder
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

#include "FubiUser.h"
#include "FubiHand.h"

// file access
#include <fstream>
// XML
#include "rapidxml.hpp"

class FubiRecorder
{
public:
	FubiRecorder();
	~FubiRecorder();
#if (defined _MSC_VER && _MSC_VER >= 1700)
	// Don't allow a copy constructor as fstreams cannot be copied
	FubiRecorder(const FubiRecorder&) = delete;
#endif

	bool startRecording(const std::string& fileName, const FubiUser* user, bool useRawData, bool useFilteredData);
	void recordNextSkeletonFrame(const FubiUser& user, const Fubi::FingerCountData& leftFingerCountData, const Fubi::FingerCountData& rightFingerCountData);

	// Same for hands
	bool startRecording(const std::string& fileName, const FubiHand* hand, bool useRawData, bool useFilteredData);
	void recordNextSkeletonFrame(const FubiHand& hand);

	void stopRecording();

	unsigned int getUserID()  { return m_userID; }
	unsigned int getHandID()  { return m_handID; }
	bool isRecording(int* currentFrameID = 0x0)
	{
		if (currentFrameID)
			*currentFrameID = m_currentFrameID;
		return m_isRecording; 
	}

private:
#if (!_MSC_VER || _MSC_VER < 1700)
	// Don't allow a copy constructor as fstreams cannot be copied
	FubiRecorder(const FubiRecorder&);
#endif
	void recordNextSkeletonFrame(const Fubi::TrackingData* data, int leftFingerCount, int rightFingerCount, std::fstream& file, const std::string& fileName);

	unsigned int m_userID, m_handID;
	bool m_isRecording;

	bool m_useRawData, m_useFilteredData;

	double m_recordingStart, m_lastTimeStamp;

	std::fstream m_file, m_secondaryFile;
	std::string m_fileName, m_scondaryFileName;
	int m_currentFrameID;
};