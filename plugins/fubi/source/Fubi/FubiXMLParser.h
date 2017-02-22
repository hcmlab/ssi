// ****************************************************************************************
//
// Fubi FubiXMLParser
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// ****************************************************************************************

#pragma once

// XML parsing
#include "rapidxml.hpp"

// STL containers
#include <string>
#include <vector>

#include "FubiMath.h"
#include "FubiUser.h"
#include "FubiHand.h"

class IGestureRecognizer;
class CombinationRecognizer;

class FubiXMLParser
{
public:
	// Load recognizers out of an xml configuration file
	static bool loadRecognizersFromXML(const std::string& fileName);

	// Load a combination recognizer from an xml string
	static CombinationRecognizer* parseCombinationRecognizer(const std::string& xmlDefinition);

	// Parse a file with skeleton tracking data that was previously recorded with the FubiRecorder
	static bool parseSkeletonFile(std::deque<Fubi::TrackingData>& skeletonData,
		Fubi::BodyMeasurementDistance* bodyMeasures, std::deque<std::pair<int, int>>& fingerCounts, int& startMarker, int& endMarker, bool& isUser,
		const std::string& fileName, int start = 0, int end = -1);

	// Parse a Template recognizer out of an xml definition string
	static IGestureRecognizer* parseTemplateRecognizer(const std::string& xmlDefinition, unsigned int& requiredHistoryLength, std::string& name);

private:
	// Load a combination recognizer from the given xml node
	static CombinationRecognizer* parseCombinationRecognizer(rapidxml::xml_node<>* node, float globalMinConfidence, bool globalUseFilteredData);

	// Parse Template recognizer from given xml node
	static IGestureRecognizer* parseTemplateRecognizer(rapidxml::xml_node<>* node, float globalMinConfidence, bool defaultUseFilteredData,
		bool& isVisible, std::string& name, unsigned int& requiredHistoryLength, const std::string& baseDir = "");

	static void parseJointData(rapidxml::xml_node<char>* jointNode, Fubi::Vec3f& trans, float& confidence);
};