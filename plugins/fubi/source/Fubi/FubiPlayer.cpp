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

#include "FubiPlayer.h"

#include "FubiXMLParser.h"

#include <iostream>
// file access
#include <fstream>
// XML
#include "rapidxml_print.hpp"

using namespace Fubi;
using namespace std;
using namespace rapidxml;

FubiPlayer::FubiPlayer() : m_isPlaying(false), m_isUser(true)
{
	// Disable body measurement updates
	m_user.m_bodyMeasurementUpdateIntervall = Math::MaxDouble;
}

FubiPlayer::~FubiPlayer()
{
	stopPlaying();
}

int FubiPlayer::loadRecording(const std::string& fileName)
{
	stopPlaying();
	m_recordedData.clear();
	m_recordedFingerCounts.clear();
	int startFrame = -1, endFrame = -1;

	// Parse the recording file
	if (FubiXMLParser::parseSkeletonFile(m_recordedData, m_user.m_bodyMeasurements, m_recordedFingerCounts, startFrame, endFrame, m_isUser, fileName))
	{
		m_fileName = fileName;
		setMarkers(startFrame - 1, startFrame, endFrame);
	}

	return (int) m_recordedData.size();
}

void FubiPlayer::setMarkers(int currentFrame, int startFrame /*= 0*/, int endFrame /*= -1*/)
{
	size_t dataSize = m_recordedData.size();
	if (dataSize > 0)
	{
		int maxID = (int)(dataSize - 1);
		m_startFrameID = clamp(startFrame, 0, maxID);
		m_lastFrameID = (endFrame < 0) ? maxID : clamp(endFrame, m_startFrameID, maxID);
		m_nextFrameID = clamp(currentFrame+1, m_startFrameID, m_lastFrameID+1);
	}
}

void FubiPlayer::getMarkers(int& currentFrame, int& startFrame, int& endFrame)
{
	currentFrame = m_nextFrameID - 1;
	startFrame = m_startFrameID;
	endFrame = m_lastFrameID;
}

bool FubiPlayer::trimFileToMarkers()
{
	size_t dataSize = m_recordedData.size();
	if (dataSize > 0)
	{
		if (m_startFrameID == 0 && m_lastFrameID == dataSize - 1)
			return true;

		// Open the recording file
		fstream file;
		file.open(m_fileName, fstream::in | fstream::binary);
		if (file.is_open() && file.good())
		{
			// get length of file:
			file.seekg(0, fstream::end);
			int length = (int)file.tellg();
			file.seekg(0, fstream::beg);
			// allocate memory:
			char* buffer = new char[length + 1];
			// read data as a block:
			file.read(buffer, length);
			// null terminate the string
			buffer[length] = '\0';
			// and close the file
			file.close();
			// Load the string to the parser
			rapidxml::xml_document<>* doc = new rapidxml::xml_document<>();   // character type defaults to char
			try
			{
				doc->parse<0>(buffer);
			}
			catch (std::exception& e)
			{
				Fubi_logWrn("Error parsing recording XML: %s", e.what());
				return false;
			}

			// Trim the content
			rapidxml::xml_node<>* rootNode = doc->first_node("FubiRecording");
			if (rootNode)
			{
				// Remove start and end markers as they will be invalidated
				xml_attribute<>* attr = rootNode->first_attribute("startMarker");
				if (attr)
				{
					rootNode->remove_attribute(attr);
				}
				attr = rootNode->first_attribute("endMarker");
				if (attr)
				{
					rootNode->remove_attribute(attr);
				}

				int i = 0;
				for (xml_node<>* frameNode = rootNode->first_node("Frame"); frameNode; ++i)
				{
					if (i < m_startFrameID || i > m_lastFrameID)
					{
						xml_node<>* nodeToRemove = frameNode;
						frameNode = frameNode->next_sibling("Frame");
						rootNode->remove_node(nodeToRemove);
					}
					else
					{
						attr = frameNode->first_attribute("frameID");
						if (attr)
						{
							frameNode->remove_attribute(attr);
							frameNode->append_attribute(doc->allocate_attribute("frameID", doc->allocate_string(numToString(i-m_startFrameID, 0).c_str())));
						}
						frameNode = frameNode->next_sibling("Frame");
					}

				}

				// Finished trimming now write the content back to the file
				file.open(m_fileName, fstream::out | fstream::trunc);
				if (file.is_open() && file.good())
				{
					file << *doc;
					
					if (file.fail())
						Fubi_logErr("Error writing to file: %s \n", m_fileName.c_str());
					else
					{
						int numFrames = m_lastFrameID - m_startFrameID + 1;
						// Trim already loaded data as well
						if (m_recordedData.size() > 0)
						{
							m_recordedData.erase(m_recordedData.begin(), m_recordedData.begin() + m_startFrameID);
							m_recordedData.erase(m_recordedData.begin() + numFrames, m_recordedData.end());
						}
						if (m_recordedFingerCounts.size() > 0)
						{
							m_recordedFingerCounts.erase(m_recordedFingerCounts.begin(), m_recordedFingerCounts.begin() + m_startFrameID);
							m_recordedFingerCounts.erase(m_recordedFingerCounts.begin() + numFrames, m_recordedFingerCounts.end());
						}
						setMarkers(-1);
						return true;
					}
				}
				else
					Fubi_logErr("Error opening file (already openend in another application?): %s \n", m_fileName.c_str());
			}
			else
				Fubi_logErr("Error: file with invalid format: %s \n", m_fileName.c_str());
		}
		else
			Fubi_logErr("Error opening file (already openend in another application?): %s \n", m_fileName.c_str());
	}
	return false;
}

void FubiPlayer::startPlaying(bool loop /*= false*/)
{
	size_t dataSize = m_recordedData.size();
	if (dataSize > 0)
	{
		if (m_nextFrameID > m_lastFrameID)
			m_nextFrameID = m_startFrameID;
		m_startTime = currentTime() - m_recordedData[m_nextFrameID].timeStamp;
		if (m_isUser)
		{
			m_user.m_inScene = true;
			m_user.m_isTracked = true;
			m_user.m_id = PlaybackUserID;
		}
		else // is hand
		{
			m_hand.m_isTracked = true;
			m_hand.m_id = PlaybackHandID;
		}

		m_isPlaying = true;
		m_isPaused = false;
		m_loop = loop;
	}
}

void FubiPlayer::update()
{
	if (m_isPlaying)
	{
		if (m_isPaused)
		{
			if (m_nextFrameID > m_startFrameID)
			{
				if (m_isUser)
				{
					m_user.addNewTrackingData(m_recordedData[m_nextFrameID - 1].jointPositions, m_recordedData[m_nextFrameID - 1].jointOrientations,
						m_recordedFingerCounts[m_nextFrameID - 1].first, m_recordedFingerCounts[m_nextFrameID - 1].second,
						m_recordedData[m_nextFrameID - 1].timeStamp);
				}
				else
				{
					SkeletonJoint::Joint handType = (m_recordedFingerCounts[m_nextFrameID - 1].first >= 0) ? SkeletonJoint::LEFT_HAND : SkeletonJoint::RIGHT_HAND;
					int fingerCount = maxi(m_recordedFingerCounts[m_nextFrameID - 1].first, m_recordedFingerCounts[m_nextFrameID - 1].second);

					m_hand.addNewTrackingData(m_recordedData[m_nextFrameID - 1].jointPositions, m_recordedData[m_nextFrameID - 1].jointOrientations,
						fingerCount, handType, m_recordedData[m_nextFrameID - 1].timeStamp);
				}
			}
		}
		else
		{
			if (m_nextFrameID > m_lastFrameID)
			{
				stopPlaying();
				if (m_loop)
					startPlaying(true);
			}
			if (m_isPlaying)
			{
				double timePassed = currentTime() - m_startTime;
				double frameTime = m_recordedData[m_nextFrameID].timeStamp;
				while (timePassed >= frameTime)
				{
					if (m_isUser)
					{
						m_user.addNewTrackingData(m_recordedData[m_nextFrameID].jointPositions, m_recordedData[m_nextFrameID].jointOrientations,
							m_recordedFingerCounts[m_nextFrameID].first, m_recordedFingerCounts[m_nextFrameID].second,
							m_recordedData[m_nextFrameID].timeStamp);
					}
					else
					{
						SkeletonJoint::Joint handType = (m_recordedFingerCounts[m_nextFrameID].first >= 0) ? SkeletonJoint::LEFT_HAND : SkeletonJoint::RIGHT_HAND;
						int fingerCount = maxi(m_recordedFingerCounts[m_nextFrameID].first, m_recordedFingerCounts[m_nextFrameID].second);
						m_hand.addNewTrackingData(m_recordedData[m_nextFrameID].jointPositions, m_recordedData[m_nextFrameID].jointOrientations,
							fingerCount, handType, m_recordedData[m_nextFrameID].timeStamp);
					}
					++m_nextFrameID;
					if (m_nextFrameID > m_lastFrameID)
						break;
					frameTime = m_recordedData[m_nextFrameID].timeStamp ;
				}
			}
		}
	}
}

void FubiPlayer::stopPlaying()
{
	m_isPlaying = false;
	m_isPaused = false;
	m_nextFrameID = m_startFrameID;
	m_user.reset();
	m_hand.reset();
}

bool FubiPlayer::isPlaying(int* currentFrameID /*= 0x0*/, bool* isPaused /*= 0x0*/)
{
	if (currentFrameID)
		*currentFrameID = m_nextFrameID - 1;
	if (isPaused)
		*isPaused = m_isPaused;
	return m_isPlaying;
}

double FubiPlayer::getPlaybackDuration()
{
	if (m_recordedData.size() > 0)
		return m_recordedData[m_lastFrameID].timeStamp - m_recordedData[m_startFrameID].timeStamp;
	return -1.0;
}

void FubiPlayer::updateTrackingHistoryLength(unsigned int requiredTrackingHistoryLength)
{
	if (m_user.m_maxTrackingHistoryLength < requiredTrackingHistoryLength)
		m_user.m_maxTrackingHistoryLength = requiredTrackingHistoryLength;
	if (m_hand.m_maxTrackingHistoryLength < requiredTrackingHistoryLength)
		m_hand.m_maxTrackingHistoryLength = requiredTrackingHistoryLength;
}