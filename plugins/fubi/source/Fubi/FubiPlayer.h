// ****************************************************************************************
//
// Fubi FubiPlayer
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

namespace rapidxml
{
	template<class Ch> class xml_node;
}

class FubiPlayer
{
public:
	FubiPlayer();
	~FubiPlayer();

	int loadRecording(const std::string& fileName);
	void startPlaying(bool loop = false);
	void setMarkers(int currentFrame, int startFrame = 0, int endFrame = -1);
	void getMarkers(int& currentFrame, int& startFrame, int& endFrame);
	bool trimFileToMarkers();
	double getPlaybackDuration();
	void update();
	void stopPlaying();
	void pausePlaying() { m_isPaused = true; }

	FubiUser* getUser()  { return (m_isUser) ? &m_user : 0x0; }
	FubiHand* getHand()  { return (!m_isUser) ? &m_hand : 0x0; }
	bool isPlaying(int* currentFrameID = 0x0, bool* isPaused = 0x0);

	void updateTrackingHistoryLength(unsigned int requiredTrackingHistoryLength);

private:
	FubiUser m_user;
	FubiHand m_hand;
	bool m_isUser;
	std::deque<Fubi::TrackingData>		m_recordedData;
	std::deque<std::pair<int, int>>		m_recordedFingerCounts;

	bool m_isPlaying, m_isPaused, m_loop;
	double m_startTime;
	int m_startFrameID, m_nextFrameID, m_lastFrameID;
	std::string m_fileName;
};