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

#include "FubiRecorder.h"

#include "rapidxml_print.hpp"

#include <iostream>

using namespace Fubi;
using namespace std;
using namespace rapidxml;

FubiRecorder::FubiRecorder() : m_userID(0), m_handID(0), m_isRecording(false)
{}

FubiRecorder::~FubiRecorder()
{
	stopRecording();
}

bool FubiRecorder::startRecording(const std::string& fileName, const FubiUser* user, bool useRawData, bool useFilteredData)
{
	stopRecording();

	m_useFilteredData = useFilteredData;
	m_useRawData = useRawData;
	
	m_userID = m_handID = 0;

	if (!m_useFilteredData && !m_useRawData)
		Fubi_logWrn("You have to select raw and/or filtered data for recording!\n");
	else if (user)
	{
		m_userID = user->id();


		// Open the file for recording, all included data will be deleted!
		m_file.open(fileName, fstream::out | fstream::trunc);
		if (m_file.is_open() && m_file.good())
		{
			m_fileName = fileName;
			m_lastTimeStamp = 0;
			m_currentFrameID = 0;
			m_recordingStart = user->currentTrackingData()->timeStamp;
			m_isRecording = true;
			// Print header and root node by hand to enable live writing to the file
			m_file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
			m_file << "<FubiRecording userID=\"" << m_userID << "\" filteredData=" << (m_useRawData ? "\"false\"" : "\"true\"")  << ">" << endl;
			// Now add the current body measurements
			xml_document<> doc;
			xml_node<>* rootNode = doc.allocate_node(node_element, "FubiRecording");
			doc.append_node(rootNode);
			xml_node<>* measuresNode = doc.allocate_node(node_element, "BodyMeasurements");
			rootNode->append_node(measuresNode);
			for (int i = 0; i < BodyMeasurement::NUM_MEASUREMENTS; ++i)
			{
				xml_node<>* node = doc.allocate_node(node_element, "Measurement");
				xml_attribute<>* attr = doc.allocate_attribute("name", getBodyMeasureName((BodyMeasurement::Measurement)i));
				node->append_attribute(attr);
				attr = doc.allocate_attribute("value", doc.allocate_string(numToString(user->bodyMeasurements()[i].m_dist, 0).c_str()));
				node->append_attribute(attr);
				attr = doc.allocate_attribute("confidence", doc.allocate_string(numToString(user->bodyMeasurements()[i].m_confidence, 4).c_str()));
				node->append_attribute(attr);
				measuresNode->append_node(node);
			}
			// Call the internal print_node directly to start at a different indent level
			rapidxml::internal::print_node(std::ostream_iterator<char>(m_file), measuresNode, 0, 1);
			if (m_file.fail())
			{
				Fubi_logErr("Error writing header to file: %s \n", m_fileName.c_str());
				stopRecording();
			}
			else if (m_useFilteredData && m_useRawData) // Two files are recorded
			{
				// Adapt file name
				size_t fileExtPos = fileName.find_last_of(".");
				m_scondaryFileName = m_fileName.substr(0, fileExtPos) + "_filtered" + m_fileName.substr(fileExtPos);
				m_secondaryFile.open(m_scondaryFileName, fstream::out | fstream::trunc);
				if (m_secondaryFile.is_open() && m_secondaryFile.good())
				{
					// Print header and root node by hand to enable live writing to the file
					m_secondaryFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
					m_secondaryFile << "<FubiRecording userID=\"" << m_userID << "\" filteredData=\"true\">" << endl;
					// Now add the current body measurements
					xml_document<> doc;
					xml_node<>* rootNode = doc.allocate_node(node_element, "FubiRecording");
					doc.append_node(rootNode);
					xml_node<>* measuresNode = doc.allocate_node(node_element, "BodyMeasurements");
					rootNode->append_node(measuresNode);
					for (int i = 0; i < BodyMeasurement::NUM_MEASUREMENTS; ++i)
					{
						xml_node<>* node = doc.allocate_node(node_element, "Measurement");
						xml_attribute<>* attr = doc.allocate_attribute("name", getBodyMeasureName((BodyMeasurement::Measurement)i));
						node->append_attribute(attr);
						attr = doc.allocate_attribute("value", doc.allocate_string(numToString(user->bodyMeasurements()[i].m_dist, 0).c_str()));
						node->append_attribute(attr);
						attr = doc.allocate_attribute("confidence", doc.allocate_string(numToString(user->bodyMeasurements()[i].m_confidence, 4).c_str()));
						node->append_attribute(attr);
						measuresNode->append_node(node);
					}
					// Call the internal print_node directly to start at a different indent level
					rapidxml::internal::print_node(std::ostream_iterator<char>(m_secondaryFile), measuresNode, 0, 1);
					if (m_secondaryFile.fail())
					{
						Fubi_logErr("Error writing header to file: %s \n", m_scondaryFileName.c_str());
						m_secondaryFile.close();
						m_secondaryFile.clear();
						m_scondaryFileName = "";
						m_useFilteredData = false;
					}
				}
				else
					Fubi_logErr("Error opening secondary file for recording (already openend in another application?): %s \n", m_scondaryFileName.c_str());
			}
		}
		else
			Fubi_logErr("Error opening file for recording (already openend in another application?): %s \n", m_fileName.c_str());
	}
	else
		Fubi_logWrn("Unable to start recording skeleton data for invalid user!\n");
	return m_isRecording;
}

void FubiRecorder::recordNextSkeletonFrame(const FubiUser& user, const Fubi::FingerCountData& leftFingerCountData, const Fubi::FingerCountData& rightFingerCountData)
{
	const Fubi::TrackingData* data = m_useRawData
		? user.currentTrackingData()
		: user.currentFilteredTrackingData();

	int leftFingerCount = -1, rightFingerCount = -1;
	if (leftFingerCountData.trackingEnabled && leftFingerCountData.fingerCounts.size() > 0)
		leftFingerCount = leftFingerCountData.fingerCounts.back();
	if (rightFingerCountData.trackingEnabled && rightFingerCountData.fingerCounts.size() > 0)
		rightFingerCount = rightFingerCountData.fingerCounts.back();

	if (m_isRecording && data && (data->timeStamp - m_lastTimeStamp) > Math::Epsilon)
	{
		m_lastTimeStamp = data->timeStamp;
		recordNextSkeletonFrame(data, leftFingerCount, rightFingerCount, m_file, m_fileName);
		if (m_useRawData && m_useFilteredData)
			recordNextSkeletonFrame(user.currentFilteredTrackingData(), leftFingerCount, rightFingerCount, m_secondaryFile, m_scondaryFileName);
		++m_currentFrameID;
	}

}

bool FubiRecorder::startRecording(const std::string& fileName, const FubiHand* hand, bool useRawData, bool useFilteredData)
{
	stopRecording();

	m_useFilteredData = useFilteredData;
	m_useRawData = useRawData;
	m_userID = m_handID = 0;

	if (!m_useFilteredData && !m_useRawData)
		Fubi_logWrn("You have to select raw and/or filtered data for recording!\n");
	else if (hand)
	{
		m_handID = hand->id();

		// Open the file for recording, all included data will be deleted!
		m_file.open(fileName, fstream::out | fstream::trunc);
		if (m_file.is_open() && m_file.good())
		{
			m_fileName = fileName;
			m_lastTimeStamp = 0;
			m_currentFrameID = 0;
			m_recordingStart = hand->currentTrackingData()->timeStamp;
			m_isRecording = true;
			// Print header and root node by hand to enable live writing to the file
			m_file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
			m_file << "<FubiRecording handID=\"" << m_handID << "\" filteredData=" << (m_useRawData ? "\"false\"" : "\"true\"") << ">" << endl;
			if (m_file.fail())
			{
				Fubi_logErr("Error writing header to file: %s \n", m_fileName.c_str());
				stopRecording();
			}
			else if (m_useFilteredData && m_useRawData) // Two files are recorded
			{
				// Adapt file name
				size_t fileExtPos = fileName.find_last_of(".", 0);
				m_scondaryFileName = m_fileName.substr(0, fileExtPos) + "_filtered" + m_fileName.substr(fileExtPos);
				m_secondaryFile.open(m_scondaryFileName, fstream::out | fstream::trunc);
				if (m_secondaryFile.is_open() && m_secondaryFile.good())
				{
					// Print header and root node by hand to enable live writing to the file
					m_secondaryFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
					m_secondaryFile << "<FubiRecording handID=\"" << m_handID << "\" filteredData=\"true\">" << endl;
					if (m_secondaryFile.fail())
					{
						Fubi_logErr("Error writing header to file: %s \n", m_scondaryFileName.c_str());
						m_secondaryFile.close();
						m_secondaryFile.clear();
						m_scondaryFileName = "";
						m_useFilteredData = false;
					}
				}
				else
					Fubi_logErr("Error opening secondary file for recording (already openend in another application?): %s \n", m_scondaryFileName.c_str());
			}
		}
		else
			Fubi_logErr("Error opening file for recording (already openend in another application?): %s \n", m_fileName.c_str());
	}
	else
		Fubi_logWrn("Unable to start recording skeleton data for invalid hand!\n");
	return m_isRecording;
}

void FubiRecorder::recordNextSkeletonFrame(const FubiHand& hand)
{
	const Fubi::FingerTrackingData* data = (const Fubi::FingerTrackingData*)(m_useFilteredData
		? hand.currentFilteredTrackingData()
		: hand.currentTrackingData());

	int leftFingerCount = (hand.getHandType() == SkeletonJoint::LEFT_HAND) ? data->fingerCount :- 1;
	int rightFingerCount = (hand.getHandType() == SkeletonJoint::RIGHT_HAND) ? data->fingerCount : -1;

	if (m_isRecording && data && (data->timeStamp - m_lastTimeStamp) > Math::Epsilon)
	{
		m_lastTimeStamp = data->timeStamp;
		recordNextSkeletonFrame(data, leftFingerCount, rightFingerCount, m_file, m_fileName);
		if (m_useRawData && m_useFilteredData)
			recordNextSkeletonFrame(hand.currentFilteredTrackingData(), leftFingerCount, rightFingerCount, m_secondaryFile, m_scondaryFileName);
		++m_currentFrameID;
	}
}

void FubiRecorder::recordNextSkeletonFrame(const TrackingData* data, int leftFingerCount, int rightFingerCount, std::fstream& file, const std::string& fileName)
{
	xml_document<> doc;
	// Create base node
	xml_node<>* frameNode = doc.allocate_node(node_element, "Frame");
	doc.append_node(frameNode);
	// With time attribute
	xml_attribute<>* attr = doc.allocate_attribute("time", doc.allocate_string(numToString(m_lastTimeStamp - m_recordingStart, 6).c_str()));
	frameNode->append_attribute(attr);
	// And frame id
	attr = doc.allocate_attribute("frameID", doc.allocate_string(numToString(m_currentFrameID, 0).c_str()));
	frameNode->append_attribute(attr);

	// Create all sub nodes for the frame
	xml_node<>* jointPositions = doc.allocate_node(node_element, "JointPositions");
	frameNode->append_node(jointPositions);
	xml_node<>* jointOrientations = doc.allocate_node(node_element, "JointOrientations");
	frameNode->append_node(jointOrientations);

	// Now fill them with the joint data
	for (int i = 0; i < data->numJoints; ++i)
	{
		xml_node<>* joint = doc.allocate_node(node_element, "Joint");
		joint->append_attribute(doc.allocate_attribute("name", data->getJointName(i)));
		const Vec3f* pos = &(data->jointPositions[i].m_position);
		joint->append_attribute(doc.allocate_attribute("x", doc.allocate_string(numToString(pos->x, 0).c_str())));
		joint->append_attribute(doc.allocate_attribute("y", doc.allocate_string(numToString(pos->y, 0).c_str())));
		joint->append_attribute(doc.allocate_attribute("z", doc.allocate_string(numToString(pos->z, 0).c_str())));
		joint->append_attribute(doc.allocate_attribute("confidence", doc.allocate_string(numToString(data->jointPositions[i].m_confidence, 0).c_str())));
		jointPositions->append_node(joint);

		joint = doc.allocate_node(node_element, "Joint");
		joint->append_attribute(doc.allocate_attribute("name", data->getJointName(i)));
		const Vec3f& rot = data->jointOrientations[i].m_orientation.getRot();
		joint->append_attribute(doc.allocate_attribute("x", doc.allocate_string(numToString(rot.x, 2).c_str())));
		joint->append_attribute(doc.allocate_attribute("y", doc.allocate_string(numToString(rot.y, 2).c_str())));
		joint->append_attribute(doc.allocate_attribute("z", doc.allocate_string(numToString(rot.z, 2).c_str())));
		joint->append_attribute(doc.allocate_attribute("confidence", doc.allocate_string(numToString(data->jointOrientations[i].m_confidence, 0).c_str())));
		jointOrientations->append_node(joint);
	}

	if (leftFingerCount > -1 || rightFingerCount > -1)
	{
		xml_node<>* fingercounts = doc.allocate_node(node_element, "FingerCounts");
		frameNode->append_node(fingercounts);
		if (leftFingerCount > -1)
			fingercounts->append_attribute(doc.allocate_attribute("left", doc.allocate_string(numToString(leftFingerCount, 0).c_str())));
		if (rightFingerCount > -1)
			fingercounts->append_attribute(doc.allocate_attribute("right", doc.allocate_string(numToString(rightFingerCount, 0).c_str())));
	}

	// Write the frame node to the file
	// Call the internal print_node directly to start at a different indent level
	internal::print_node(std::ostream_iterator<char>(file), frameNode, 0, 1);
	if (file.fail())
	{
		Fubi_logErr("Error writing skeleton data to file: %s \n", fileName.c_str());
		stopRecording();
	}
}

void FubiRecorder::stopRecording()
{
	if (m_file.is_open())
	{
		// End the root node by hand
		m_file << "</FubiRecording>";
		m_file.close();
		m_file.clear();
	}
	if (m_secondaryFile.is_open())
	{
		// End the root node by hand
		m_secondaryFile << "</FubiRecording>";
		m_secondaryFile.close();
		m_secondaryFile.clear();
	}
	m_isRecording = false;
}
