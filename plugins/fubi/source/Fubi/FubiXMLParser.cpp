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

#include "FubiXMLParser.h"

#include "FubiRecognizerFactory.h"
#include "GestureRecognizer/CombinationRecognizer.h"
#include "FubiCore.h"

// File reading for Xml parsing
#include <fstream>

#include <exception>


using namespace Fubi;
using namespace rapidxml;


CombinationRecognizer* FubiXMLParser::parseCombinationRecognizer(const std::string& xmlDefinition)
{
	CombinationRecognizer* rec = 0x0;

	// copy string to buffer
	char* buffer = new char [xmlDefinition.length()+1];
#pragma warning(push)
#pragma warning(disable:4996)
	strcpy(buffer, xmlDefinition.c_str());
#pragma warning(pop)

	// parse XML
	xml_document<>* doc = new xml_document<>();
	doc->parse<0>(buffer);
	xml_node<>* node = doc->first_node("CombinationRecognizer");
	if (node)
		rec = parseCombinationRecognizer(node, -1, false);

	// release xml doc and buffer
	doc->clear();
	delete doc;
	delete[] buffer;

	return rec;
}

bool FubiXMLParser::loadRecognizersFromXML(const std::string& fileName)
{
	FubiCore* core = FubiCore::getInstance();
	if (core == 0x0)
		return false;

	// Open the file and copy the data to a buffer
	fstream file;
	file.open (fileName.c_str(), fstream::in | fstream::binary );
	if (!file.is_open() || !file.good())
		return false;

	bool loadedAnything = false;

	// get length of file:
	file.seekg (0, fstream::end);
	int length = (int)file.tellg();
	file.seekg (0, fstream::beg);
	// allocate memory:
	char* buffer = new char [length+1];
	// read data as a block:
	file.read(buffer, length);
	// null terminate the string
	buffer[length] = '\0';
	// and close the file
	file.close();

	// Load the string to the parser
	xml_document<>* doc = new xml_document<>();   // character type defaults to char

	try
	{
		doc->parse<0>(buffer);
	}
	catch (std::exception& e)
	{
		Fubi_logWrn("Error parsing recognizer XML: %s", e.what());
		return false;
	}

	// Parse the content
	xml_node<>* node = doc->first_node("FubiRecognizers");
	if (node)
	{
		float globalMinConf = -1.0f;
		xml_attribute<>* globalMinConfA = node->first_attribute("globalMinConfidence");
		if (globalMinConfA)
			globalMinConf = (float)atof(globalMinConfA->value());

		bool globalUseFilteredData = false;
		xml_attribute<>* globalfilterA = node->first_attribute("globalUseFilteredData");
		if (globalfilterA)
			globalUseFilteredData = strcmp(globalfilterA->value(), "false") != 0 && strcmp(globalfilterA->value(), "0") != 0;

		xml_node<>* recNode;
		for(recNode = node->first_node("JointRelationRecognizer"); recNode; recNode = recNode->next_sibling("JointRelationRecognizer"))
		{
			std::string name;
			xml_attribute<>* attr = recNode->first_attribute("name");
			if (attr)
				name = attr->value();

			bool visible = true;
			attr = recNode->first_attribute("visibility");
			if (attr)
				visible = removeWhiteSpacesAndToLower(attr->value()) != "hidden";

			bool localPos = false;
			attr = recNode->first_attribute("useLocalPositions");
			if (attr)
				localPos = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			float minConf = globalMinConf;
			xml_attribute<>* minConfA = recNode->first_attribute("minConfidence");
			if (minConfA)
				minConf = (float)atof(minConfA->value());

			bool useFilteredData = globalUseFilteredData;
			attr = recNode->first_attribute("useFilteredData");
			if (attr)
				useFilteredData = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			BodyMeasurement::Measurement measure = BodyMeasurement::NUM_MEASUREMENTS;
			xml_attribute<>* measuringUnit = recNode->first_attribute("measuringUnit");
			if (measuringUnit)
				measure = Fubi::getBodyMeasureID(measuringUnit->value());

			SkeletonJoint::Joint joint = SkeletonJoint::RIGHT_HAND;
			SkeletonJoint::Joint relJoint = SkeletonJoint::NUM_JOINTS;
			xml_node<>* jointNode = recNode->first_node("Joints");
			if (jointNode)
			{
				attr = jointNode->first_attribute("main");
				if (attr)
					joint = getJointID(attr->value());
				attr = jointNode->first_attribute("relative");
				if (attr)
					relJoint = getJointID(attr->value());
			}
			RecognizerTarget::Target targetSkeleton = RecognizerTarget::BODY;
			xml_node<>* handJointNode = recNode->first_node("HandJoints");
			if (handJointNode)
			{
				attr = handJointNode->first_attribute("main");
				if (attr)
				{
					SkeletonHandJoint::Joint hJoint = getHandJointID(attr->value());
					// This is the hacky part, converting a hand joint enum to a skeleton joint enum
					// We only care about the actual digit...
					if (hJoint != SkeletonHandJoint::NUM_JOINTS)
					{
						xml_node<>* jointNode = recNode->first_node("Joint");
						if (jointNode)
						{
							attr = jointNode->first_attribute("name");
							if (attr)
								joint = getJointID(attr->value());
						}
						if (joint == SkeletonJoint::RIGHT_HAND)
							targetSkeleton = RecognizerTarget::RIGHT_HAND;
						else if (joint == SkeletonJoint::LEFT_HAND)
							targetSkeleton = RecognizerTarget::LEFT_HAND;
						else
							targetSkeleton = RecognizerTarget::BOTH_HANDS;
						joint = (SkeletonJoint::Joint) hJoint;
						attr = handJointNode->first_attribute("relative");
						if (attr)
						{
							hJoint = getHandJointID(attr->value());
							if (hJoint == SkeletonHandJoint::NUM_JOINTS)
								relJoint = SkeletonJoint::NUM_JOINTS;
							else
								relJoint = (SkeletonJoint::Joint) hJoint;
						}
					}
				}
			}

			Vec3f minValues = DefaultMinVec;
			float minDistance = 0;
			xml_node<>* minNode = recNode->first_node("MinValues");
			if (minNode)
			{
				attr = minNode->first_attribute("x");
				if (attr)
					minValues.x = (float) atof(attr->value());
				attr = minNode->first_attribute("y");
				if (attr)
					minValues.y = (float) atof(attr->value());
				attr = minNode->first_attribute("z");
				if (attr)
					minValues.z = (float) atof(attr->value());

				attr = minNode->first_attribute("dist");
				if (attr)
					minDistance = (float) atof(attr->value());
			}

			Vec3f maxValues = DefaultMaxVec;
			float maxDistance = Math::MaxFloat;
			xml_node<>* maxNode = recNode->first_node("MaxValues");
			if (maxNode)
			{
				attr = maxNode->first_attribute("x");
				if (attr)
					maxValues.x = (float) atof(attr->value());
				attr = maxNode->first_attribute("y");
				if (attr)
					maxValues.y = (float) atof(attr->value());
				attr = maxNode->first_attribute("z");
				if (attr)
					maxValues.z = (float) atof(attr->value());

				attr = maxNode->first_attribute("dist");
				if (attr)
					maxDistance = (float) atof(attr->value());
			}

			for(xml_node<>* relNode = recNode->first_node("Relation"); relNode; relNode = relNode->next_sibling("Relation"))
			{
				float min = -Math::MaxFloat;
				float max = Math::MaxFloat;
				attr = relNode->first_attribute("min");
				if (attr)
					min = (float) atof(attr->value());
				attr = relNode->first_attribute("max");
				if (attr)
					max = (float) atof(attr->value());
				attr = relNode->first_attribute("type");
				if (attr)
				{
					std::string lowerValue = removeWhiteSpacesAndToLower(attr->value());
					if (lowerValue == "infrontof")
					{
						maxValues.z = minf(maxValues.z, -min);
						minValues.z = maxf(minValues.z, -max);
					}
					else if (lowerValue == "behind")
					{
						maxValues.z = minf(maxValues.z, max);
						minValues.z = maxf(minValues.z, min);
					}
					else if (lowerValue == "leftof")
					{
						maxValues.x = minf(maxValues.x, -min);
						minValues.x = maxf(minValues.x, -max);
					}
					else if (lowerValue == "rightof")
					{
						maxValues.x = minf(maxValues.x, max);
						minValues.x = maxf(minValues.x, min);
					}
					else if (lowerValue == "above")
					{
						maxValues.y = minf(maxValues.y, max);
						minValues.y = maxf(minValues.y, min);
					}
					else if (lowerValue == "below")
					{
						maxValues.y = minf(maxValues.y, -min);
						minValues.y = maxf(minValues.y, -max);
					}
					else if (lowerValue == "apartof")
					{
						minDistance = min;
						maxDistance = max;
					}
				}
			}

			Vec3f midJointMinValues = DefaultMinVec;
			float midJointMinDistance = 0;
			Vec3f midJointMaxValues = DefaultMaxVec;
			float midJointMaxDistance = Math::MaxFloat;
			SkeletonJoint::Joint midJoint = SkeletonJoint::NUM_JOINTS;
			xml_node<>* midJointNode = recNode->first_node("MiddleJoint");
			if (midJointNode)
			{
				xml_node<>* jointNode = midJointNode->first_node("Joint");
				if (jointNode)
				{
					attr = jointNode->first_attribute("name");
					if (attr)
					{
						midJoint = getJointID(attr->value());
						if (targetSkeleton != RecognizerTarget::BODY)
							// wrong target sensor
							Fubi_logWrn("XML_Warning- Mid joint only support within one skeleton target - \"%s\"!\n", name.c_str());
					}
				}
				xml_node<>* handJointNode = midJointNode->first_node("HandJoint");
				if (handJointNode)
				{
					attr = handJointNode->first_attribute("name");
					if (attr)
					{
						SkeletonHandJoint::Joint hJoint = getHandJointID(attr->value());
						// This is the hacky part, converting a hand joint enum to a skeleton joint enum
						// We only care about the actual digit...
						if (hJoint != SkeletonHandJoint::NUM_JOINTS)
						{
							midJoint = (SkeletonJoint::Joint) hJoint;
							if (targetSkeleton == RecognizerTarget::BODY)
								// wrong target sensor
								Fubi_logWrn("XML_Warning- Mid joint only support within one skeleton target \"%s\"!\n", name.c_str());
						}
					}
				}

				xml_node<>* minNode = midJointNode->first_node("MinValues");
				if (minNode)
				{
					attr = minNode->first_attribute("x");
					if (attr)
						midJointMinValues.x = (float) atof(attr->value());
					attr = minNode->first_attribute("y");
					if (attr)
						midJointMinValues.y = (float) atof(attr->value());
					attr = minNode->first_attribute("z");
					if (attr)
						midJointMinValues.z = (float) atof(attr->value());

					attr = minNode->first_attribute("dist");
					if (attr)
						midJointMinDistance = (float) atof(attr->value());
				}

				xml_node<>* maxNode = midJointNode->first_node("MaxValues");
				if (maxNode)
				{
					attr = maxNode->first_attribute("x");
					if (attr)
						midJointMaxValues.x = (float) atof(attr->value());
					attr = maxNode->first_attribute("y");
					if (attr)
						midJointMaxValues.y = (float) atof(attr->value());
					attr = maxNode->first_attribute("z");
					if (attr)
						midJointMaxValues.z = (float) atof(attr->value());

					attr = maxNode->first_attribute("dist");
					if (attr)
						midJointMaxDistance = (float) atof(attr->value());
				}

				for(xml_node<>* relNode = midJointNode->first_node("Relation"); relNode; relNode = relNode->next_sibling("Relation"))
				{
					float min = -Math::MaxFloat;
					float max = Math::MaxFloat;
					attr = relNode->first_attribute("min");
					if (attr)
						min = (float) atof(attr->value());
					attr = relNode->first_attribute("max");
					if (attr)
						max = (float) atof(attr->value());
					attr = relNode->first_attribute("type");
					if (attr)
					{
						std::string lowerValue = removeWhiteSpacesAndToLower(attr->value());
						if (lowerValue == "infrontof")
						{
							midJointMaxValues.z = minf(midJointMaxValues.z, -min);
							midJointMinValues.z = maxf(midJointMinValues.z, -max);
						}
						else if (lowerValue == "behind")
						{
							midJointMaxValues.z = minf(midJointMaxValues.z, max);
							midJointMinValues.z = maxf(midJointMinValues.z, min);
						}
						else if (lowerValue == "leftof")
						{
							midJointMaxValues.x = minf(midJointMaxValues.x, -min);
							midJointMinValues.x = maxf(midJointMinValues.x, -max);
						}
						else if (lowerValue == "rightof")
						{
							midJointMaxValues.x = minf(midJointMaxValues.x, max);
							midJointMinValues.x = maxf(midJointMinValues.x, min);
						}
						else if (lowerValue == "above")
						{
							midJointMaxValues.y = minf(midJointMaxValues.y, max);
							midJointMinValues.y = maxf(midJointMinValues.y, min);
						}
						else if (lowerValue == "below")
						{
							midJointMaxValues.y = minf(midJointMaxValues.y, -min);
							midJointMinValues.y = maxf(midJointMinValues.y, -max);
						}
						else if (lowerValue == "apartof")
						{
							midJointMinDistance = min;
							midJointMaxDistance = max;
						}
					}
				}
			}

			IGestureRecognizer* rec = createPostureRecognizer(joint, relJoint, minValues, maxValues, minDistance, maxDistance,
				midJoint, midJointMinValues, midJointMaxValues, midJointMinDistance, midJointMaxDistance,
				localPos, minConf, measure, useFilteredData);
			rec->m_targetSkeletonType = targetSkeleton;
			if (visible)
				core->addRecognizer(rec, name);
			else
				core->addHiddenRecognizer(rec, name);
			loadedAnything = true;
		}

		for(recNode = node->first_node("JointOrientationRecognizer"); recNode; recNode = recNode->next_sibling("JointOrientationRecognizer"))
		{
			std::string name;
			xml_attribute<>* attr = recNode->first_attribute("name");
			if (attr)
				name = attr->value();

			bool visible = true;
			attr = recNode->first_attribute("visibility");
			if (attr)
				visible = removeWhiteSpacesAndToLower(attr->value()) != "hidden";

			bool localRot = true;
			attr = recNode->first_attribute("useLocalOrientations");
			if (attr)
				localRot = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			float minConf = globalMinConf;
			xml_attribute<>* minConfA = recNode->first_attribute("minConfidence");
			if (minConfA)
				minConf = (float)atof(minConfA->value());

			bool useFilteredData = globalUseFilteredData;
			attr = recNode->first_attribute("useFilteredData");
			if (attr)
				useFilteredData = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			SkeletonJoint::Joint joint = SkeletonJoint::TORSO;
			xml_node<>* jointNode = recNode->first_node("Joint");
			if (jointNode)
			{
				attr = jointNode->first_attribute("name");
				if (attr)
					joint = getJointID(attr->value());
			}
			RecognizerTarget::Target targetSkeleton = RecognizerTarget::BODY;
			xml_node<>* handJointNode = recNode->first_node("HandJoint");
			if (handJointNode)
			{
				attr = handJointNode->first_attribute("name");
				if (attr)
				{
					SkeletonHandJoint::Joint hJoint = getHandJointID(attr->value());
					// This is the hacky part, converting a hand joint enum to a skeleton joint enum
					// We only care about the actual digit...
					if (hJoint != SkeletonHandJoint::NUM_JOINTS)
					{
						if (joint == SkeletonJoint::RIGHT_HAND)
							targetSkeleton = RecognizerTarget::RIGHT_HAND;
						else if (joint == SkeletonJoint::LEFT_HAND)
							targetSkeleton = RecognizerTarget::LEFT_HAND;
						else
							targetSkeleton = RecognizerTarget::BOTH_HANDS;
						joint = (SkeletonJoint::Joint) hJoint;
					}
				}
			}

			bool useOrientation = false;
			Vec3f orient(Math::NO_INIT);
			float maxAngleDiff = 45.0f;
			xml_node<>* orientNode = recNode->first_node("Orientation");
			if (orientNode)
			{
				useOrientation = true;
				attr = orientNode->first_attribute("x");
				if (attr)
					orient.x = (float) atof(attr->value());
				attr = orientNode->first_attribute("y");
				if (attr)
					orient.y = (float) atof(attr->value());
				attr = orientNode->first_attribute("z");
				if (attr)
					orient.z = (float) atof(attr->value());
				attr = orientNode->first_attribute("maxAngleDifference");
				if (attr)
					maxAngleDiff = (float) atof(attr->value());
			}

			if (useOrientation)
			{
				IGestureRecognizer* rec = createPostureRecognizer(joint, orient, maxAngleDiff, localRot, minConf, useFilteredData);
				if (visible)
					core->addRecognizer(rec, name);
				else
					core->addHiddenRecognizer(rec, name);
				loadedAnything = true;
			}
			else
			{
				Vec3f minValues = Vec3f(-180.0f, -180.0f, -180.0f);
				xml_node<>* minNode = recNode->first_node("MinDegrees");
				if (minNode)
				{
					attr = minNode->first_attribute("x");
					if (attr)
						minValues.x = (float) atof(attr->value());
					attr = minNode->first_attribute("y");
					if (attr)
						minValues.y = (float) atof(attr->value());
					attr = minNode->first_attribute("z");
					if (attr)
						minValues.z = (float) atof(attr->value());
				}

				Vec3f maxValues = Vec3f(180.0f, 180.0f, 180.0f);
				xml_node<>* maxNode = recNode->first_node("MaxDegrees");
				if (maxNode)
				{
					attr = maxNode->first_attribute("x");
					if (attr)
						maxValues.x = (float) atof(attr->value());
					attr = maxNode->first_attribute("y");
					if (attr)
						maxValues.y = (float) atof(attr->value());
					attr = maxNode->first_attribute("z");
					if (attr)
						maxValues.z = (float) atof(attr->value());
				}

				IGestureRecognizer* rec = createPostureRecognizer(joint, minValues, maxValues, localRot, minConf, useFilteredData);
				rec->m_targetSkeletonType = targetSkeleton;
				if (visible)
					core->addRecognizer(rec, name);
				else
					core->addHiddenRecognizer(rec, name);
				loadedAnything = true;
			}
		}

		for(recNode = node->first_node("LinearMovementRecognizer"); recNode; recNode = recNode->next_sibling("LinearMovementRecognizer"))
		{
			std::string name;
			xml_attribute<>* attr = recNode->first_attribute("name");
			if (attr)
				name = attr->value();

			bool visible = true;
			attr = recNode->first_attribute("visibility");
			if (attr)
				visible = removeWhiteSpacesAndToLower(attr->value()) != "hidden";

			bool localPos = false;
			attr = recNode->first_attribute("useLocalPositions");
			if (attr)
				localPos = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			float minConf = globalMinConf;
			xml_attribute<>* minConfA = recNode->first_attribute("minConfidence");
			if (minConfA)
				minConf = (float)atof(minConfA->value());

			bool useFilteredData = globalUseFilteredData;
			attr = recNode->first_attribute("useFilteredData");
			if (attr)
				useFilteredData = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			bool useOnlyCorrectDirectionComponent = true;
			attr = recNode->first_attribute("useOnlyCorrectDirectionComponent");
			if (attr)
				useOnlyCorrectDirectionComponent = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			SkeletonJoint::Joint joint = SkeletonJoint::RIGHT_HAND;
			SkeletonJoint::Joint relJoint = SkeletonJoint::NUM_JOINTS;
			bool useRelative = false;
			xml_node<>* jointNode = recNode->first_node("Joints");
			if (jointNode)
			{
				attr = jointNode->first_attribute("main");
				if (attr)
					joint = getJointID(attr->value());
				attr = jointNode->first_attribute("relative");
				if (attr)
				{
					relJoint = getJointID(attr->value());
					useRelative = true;
				}
			}
			RecognizerTarget::Target targetSkeleton = RecognizerTarget::BODY;
			xml_node<>* handJointNode = recNode->first_node("HandJoints");
			if (handJointNode)
			{
				attr = handJointNode->first_attribute("main");
				if (attr)
				{
					SkeletonHandJoint::Joint hJoint = getHandJointID(attr->value());
					// This is the hacky part, converting a hand joint enum to a skeleton joint enum
					// We only care about the actual digit...
					if (hJoint != SkeletonHandJoint::NUM_JOINTS)
					{
						if (joint == SkeletonJoint::RIGHT_HAND)
							targetSkeleton = RecognizerTarget::RIGHT_HAND;
						else if (joint == SkeletonJoint::LEFT_HAND)
							targetSkeleton = RecognizerTarget::LEFT_HAND;
						else
							targetSkeleton = RecognizerTarget::BOTH_HANDS;
						joint = (SkeletonJoint::Joint) hJoint;
						attr = handJointNode->first_attribute("relative");
						if (attr)
						{
							hJoint = getHandJointID(attr->value());
							if (hJoint == SkeletonHandJoint::NUM_JOINTS)
								relJoint = SkeletonJoint::NUM_JOINTS;
							else
								relJoint = (SkeletonJoint::Joint) hJoint;
						}
					}
				}
			}

			Vec3f direction;
			float minVel = 0;
			float maxVel = Math::MaxFloat;
			float maxAngleDiff = 45.0f;
			float minLength = 0;
			float maxLength = Math::MaxFloat;

			xml_node<>* dirNode = recNode->first_node("Direction");
			if (dirNode)
			{
				attr = dirNode->first_attribute("x");
				if (attr)
					direction.x = (float) atof(attr->value());
				attr = dirNode->first_attribute("y");
				if (attr)
					direction.y = (float) atof(attr->value());
				attr = dirNode->first_attribute("z");
				if (attr)
					direction.z = (float) atof(attr->value());
				attr = dirNode->first_attribute("maxAngleDifference");
				if (attr)
					maxAngleDiff = (float) atof(attr->value());
			}

			dirNode = recNode->first_node("BasicDirection");
			if (dirNode)
			{
				attr = dirNode->first_attribute("type");
				if (attr)
				{
					std::string lowerValue = removeWhiteSpacesAndToLower(attr->value());
					if (lowerValue == "left")
					{
						direction = Vec3f(-1.0f, 0, 0);
					}
					else if (lowerValue == "right")
					{
						direction = Vec3f(1.0f, 0, 0);
					}
					else if (lowerValue == "up")
					{
						direction = Vec3f(0, 1.0f, 0);
					}
					else if (lowerValue == "down")
					{
						direction = Vec3f(0, -1.0f, 0);
					}
					else if (lowerValue == "forward")
					{
						direction = Vec3f(0, 0, -1.0f);
					}
					else if (lowerValue == "backward")
					{
						direction = Vec3f(0, 0, 1.0f);
					}
					else if (lowerValue == "anydirection")
					{
						direction = Vec3f(0, 0, 0);
					}
				}
				attr = dirNode->first_attribute("maxAngleDifference");
				if (attr)
					maxAngleDiff = (float) atof(attr->value());
			}

			xml_node<>* speedNode = recNode->first_node("Speed");
			if (speedNode)
			{
				attr = speedNode->first_attribute("min");
				if (attr)
					minVel = (float) atof(attr->value());
				attr = speedNode->first_attribute("max");
				if (attr)
					maxVel = (float) atof(attr->value());
			}

			bool useLength = false;
			BodyMeasurement::Measurement measure = BodyMeasurement::NUM_MEASUREMENTS;
			xml_node<>* lengthNode = recNode->first_node("Length");
			if (lengthNode)
			{
				useLength = true;
				attr = lengthNode->first_attribute("min");
				if (attr)
					minLength = (float) atof(attr->value());
				attr = lengthNode->first_attribute("max");
				if (attr)
					maxLength = (float) atof(attr->value());
				attr = lengthNode->first_attribute("measuringUnit");
				if (attr)
					measure = Fubi::getBodyMeasureID(attr->value());
			}

			IGestureRecognizer* rec = 0x0;
			if (useRelative)
			{
				if (useLength)
					rec = createMovementRecognizer(joint, relJoint, direction, minVel, maxVel, minLength, maxLength, measure, localPos, minConf, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData);
				else
					rec = createMovementRecognizer(joint, relJoint, direction, minVel, maxVel, localPos, minConf, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData);
			}
			else
			{
				if (useLength)
					rec = createMovementRecognizer(joint, direction, minVel, maxVel, minLength, maxLength, measure, localPos, minConf, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData);
				else
					rec = createMovementRecognizer(joint, direction, minVel, maxVel, localPos, minConf, maxAngleDiff, useOnlyCorrectDirectionComponent, useFilteredData);
			}
			if (rec)
			{
				rec->m_targetSkeletonType = targetSkeleton;
				if (visible)
					core->addRecognizer(rec, name);
				else
					core->addHiddenRecognizer(rec, name);
				loadedAnything = true;
			}
		}

		for(recNode = node->first_node("AngularMovementRecognizer"); recNode; recNode = recNode->next_sibling("AngularMovementRecognizer"))
		{
			std::string name;
			xml_attribute<>* attr = recNode->first_attribute("name");
			if (attr)
				name = attr->value();

			bool visible = true;
			attr = recNode->first_attribute("visibility");
			if (attr)
				visible = removeWhiteSpacesAndToLower(attr->value()) != "hidden";

			bool localRot = true;
			attr = recNode->first_attribute("useLocalOrientations");
			if (attr)
				localRot = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			float minConf = globalMinConf;
			xml_attribute<>* minConfA = recNode->first_attribute("minConfidence");
			if (minConfA)
				minConf = (float)atof(minConfA->value());

			bool useFilteredData = globalUseFilteredData;
			attr = recNode->first_attribute("useFilteredData");
			if (attr)
				useFilteredData = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

			SkeletonJoint::Joint joint = SkeletonJoint::TORSO;
			xml_node<>* jointNode = recNode->first_node("Joint");
			if (jointNode)
			{
				attr = jointNode->first_attribute("name");
				if (attr)
					joint = getJointID(attr->value());
			}
			RecognizerTarget::Target targetSkeleton = RecognizerTarget::BODY;
			xml_node<>* handJointNode = recNode->first_node("HandJoint");
			if (handJointNode)
			{
				attr = handJointNode->first_attribute("name");
				if (attr)
				{
					SkeletonHandJoint::Joint hJoint = getHandJointID(attr->value());
					// This is the hacky part, converting a hand joint enum to a skeleton joint enum
					// We only care about the actual digit...
					if (hJoint != SkeletonHandJoint::NUM_JOINTS)
					{
						if (joint == SkeletonJoint::RIGHT_HAND)
							targetSkeleton = RecognizerTarget::RIGHT_HAND;
						else if (joint == SkeletonJoint::LEFT_HAND)
							targetSkeleton = RecognizerTarget::LEFT_HAND;
						else
							targetSkeleton = RecognizerTarget::BOTH_HANDS;
						joint = (SkeletonJoint::Joint) hJoint;
					}
				}
			}

			Vec3f minVel = Fubi::DefaultMinVec;
			xml_node<>* minNode = recNode->first_node("MinAngularVelocity");
			if (minNode)
			{
				attr = minNode->first_attribute("x");
				if (attr)
					minVel.x = (float) atof(attr->value());
				attr = minNode->first_attribute("y");
				if (attr)
					minVel.y = (float) atof(attr->value());
				attr = minNode->first_attribute("z");
				if (attr)
					minVel.z = (float) atof(attr->value());
			}

			Vec3f maxVel = Fubi::DefaultMaxVec;
			xml_node<>* maxNode = recNode->first_node("MaxAngularVelocity");
			if (maxNode)
			{
				attr = maxNode->first_attribute("x");
				if (attr)
					maxVel.x = (float) atof(attr->value());
				attr = maxNode->first_attribute("y");
				if (attr)
					maxVel.y = (float) atof(attr->value());
				attr = maxNode->first_attribute("z");
				if (attr)
					maxVel.z = (float) atof(attr->value());
			}

			for(xml_node<>* basicNode = recNode->first_node("BasicAngularVelocity"); basicNode; basicNode = basicNode->next_sibling("BasicAngularVelocity"))
			{
				float min = -Math::MaxFloat;
				float max = Math::MaxFloat;
				attr = basicNode->first_attribute("min");
				if (attr)
					min = (float) atof(attr->value());
				attr = basicNode->first_attribute("max");
				if (attr)
					max = (float) atof(attr->value());
				attr = basicNode->first_attribute("type");
				if (attr)
				{
					std::string lowerValue = removeWhiteSpacesAndToLower(attr->value());
					if (lowerValue == "rollleft")
					{
						maxVel.z = minf(maxVel.z, max);
						minVel.z = maxf(minVel.z, min);
					}
					else if (lowerValue == "rollright")
					{
						maxVel.z = minf(maxVel.z, -min);
						minVel.z = maxf(minVel.z, -max);
					}
					else if (lowerValue == "pitchdown")
					{
						maxVel.x = minf(maxVel.x, -min);
						minVel.x = maxf(minVel.x, -max);
					}
					else if (lowerValue == "pitchup")
					{
						maxVel.x = minf(maxVel.x, max);
						minVel.x = maxf(minVel.x, min);
					}
					else if (lowerValue == "yawright")
					{
						maxVel.y = minf(maxVel.y, max);
						minVel.y = maxf(minVel.y, min);
					}
					else if (lowerValue == "yawleft")
					{
						maxVel.y = minf(maxVel.y, -min);
						minVel.y = maxf(minVel.y, -max);
					}
				}
			}			

			IGestureRecognizer* rec = createMovementRecognizer(joint, minVel, maxVel, localRot, minConf, useFilteredData);
			rec->m_targetSkeletonType = targetSkeleton;
			if (visible)
				core->addRecognizer(rec, name);
			else
				core->addHiddenRecognizer(rec, name);
			loadedAnything = true;
		}

		for(recNode = node->first_node("FingerCountRecognizer"); recNode; recNode = recNode->next_sibling("FingerCountRecognizer"))
		{
			std::string name;
			xml_attribute<>* attr = recNode->first_attribute("name");
			if (attr)
				name = attr->value();

			bool visible = true;
			attr = recNode->first_attribute("visibility");
			if (attr)
				visible = removeWhiteSpacesAndToLower(attr->value()) != "hidden";

			float minConf = globalMinConf;
			xml_attribute<>* minConfA = recNode->first_attribute("minConfidence");
			if (minConfA)
				minConf = (float)atof(minConfA->value());

			bool useFilteredData = globalUseFilteredData;
			attr = recNode->first_attribute("useFilteredData");
			if (attr)
				useFilteredData = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";


			SkeletonJoint::Joint joint = SkeletonJoint::NUM_JOINTS;
			xml_node<>* jointNode = recNode->first_node("Joint");
			if (jointNode)
			{
				attr = jointNode->first_attribute("name");
				if (attr)
					joint = getJointID(attr->value());
			}

			unsigned int minFingers = 0;
			unsigned int maxFingers = 5;
			bool useMedian = false;
			unsigned int medianWindowSize = 10;
			xml_node<>* countNode = recNode->first_node("FingerCount");
			if (countNode)
			{
				attr = countNode->first_attribute("min");
				if (attr)
					minFingers = (unsigned)atoi(attr->value());
				attr = countNode->first_attribute("max");
				if (attr)
					maxFingers = (unsigned)atoi(attr->value());
				attr = countNode->first_attribute("useMedianCalculation");
				if (attr)
					useMedian = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";
				attr = countNode->first_attribute("medianWindowSize");
				if (attr)
					medianWindowSize = (unsigned)atoi(attr->value());
			}

			IGestureRecognizer* rec = createPostureRecognizer(joint, minFingers, maxFingers, minConf, useMedian, medianWindowSize, useFilteredData);
			if (joint == RecognizerTarget::LEFT_HAND)
				rec->m_targetSkeletonType = RecognizerTarget::LEFT_HAND;
			else if (joint == RecognizerTarget::RIGHT_HAND)
				rec->m_targetSkeletonType = RecognizerTarget::RIGHT_HAND;
			else
				rec->m_targetSkeletonType = RecognizerTarget::BOTH_HANDS;
			if (visible)
				core->addRecognizer(rec, name);
			else
				core->addHiddenRecognizer(rec, name);
			loadedAnything = true;
		}

		for(recNode = node->first_node("TemplateRecognizer"); recNode; recNode = recNode->next_sibling("TemplateRecognizer"))
		{
			bool visible = true;
			std::string name;
			unsigned int requiredHistoryLength = 0;
			IGestureRecognizer* rec = parseTemplateRecognizer(recNode, globalMinConf, true, visible, name, requiredHistoryLength, getDirectoryPath(fileName));
			if (rec)
			{
				if (visible)
					core->addRecognizer(rec, name);
				else
					core->addHiddenRecognizer(rec, name);
				core->updateTrackingHistoryLength(requiredHistoryLength);
				loadedAnything = true;
			}
		}

		for(recNode = node->first_node("CombinationRecognizer"); recNode; recNode = recNode->next_sibling("CombinationRecognizer"))
		{
			CombinationRecognizer* rec = parseCombinationRecognizer(recNode, globalMinConf, globalUseFilteredData);
			if (rec)
			{
				core->addCombinationRecognizer(rec);
				loadedAnything = true;
			}
		}

		bool oldCombinations = false;
		for(recNode = node->first_node("PostureCombinationRecognizer"); recNode; recNode = recNode->next_sibling("PostureCombinationRecognizer"))
		{
			CombinationRecognizer* rec = parseCombinationRecognizer(recNode, globalMinConf, globalUseFilteredData);
			if (rec)
			{
				core->addCombinationRecognizer(rec);
				loadedAnything = true;
				oldCombinations = true;
			}
		}
		if (oldCombinations)
		{
			Fubi_logWrn("XML_Warning - \"PostureCombinationRecognizer\" deprecated, please use \"CombinationRecognizer\"!\n");
		}
	}

	doc->clear();
	delete doc;
	// release the buffer
	delete[] buffer;
	return loadedAnything;
}

CombinationRecognizer* FubiXMLParser::parseCombinationRecognizer(xml_node<>* node, float globalMinConfidence, bool globalUseFilteredData)
{
	CombinationRecognizer* rec = 0x0;

	std::string name;
	xml_attribute<>* attr = node->first_attribute("name");
	FubiCore* core = FubiCore::getInstance();
	if (core && attr)
	{
		name = attr->value();

		// Create combination recognizer template (not assigned to a user or hand)
		rec = new CombinationRecognizer(name);

		attr = node->first_attribute("waitUntilLastStateRecognizersStop");
		if (attr)
		{
			std::string lowerValue = removeWhiteSpacesAndToLower(attr->value());
			rec->setWaitUntilLastStateRecognizersStop(lowerValue != "0" && lowerValue != "false");
		}

		xml_node<>* stateNode;
		int stateNum;
		for(stateNode = node->first_node("State"), stateNum = 1; stateNode; stateNode = stateNode->next_sibling("State"), ++stateNum)
		{
			double maxDuration = -1;
			double minDuration = 0;
			double timeForTransition = 1.0;
			double maxInterruption = -1;
			bool noInterrruptionBeforeMinDuration = false;
			bool restartOnFail = true;

			attr = stateNode->first_attribute("maxDuration");
			if (attr)
				maxDuration = atof(attr->value());
			attr = stateNode->first_attribute("minDuration");
			if (attr)
				minDuration = atof(attr->value());
			attr = stateNode->first_attribute("timeForTransition");
			if (attr)
				timeForTransition = atof(attr->value());
			attr = stateNode->first_attribute("maxInterruptionTime");
			if (attr)
				maxInterruption = atof(attr->value());
			attr = stateNode->first_attribute("noInterrruptionBeforeMinDuration");
			if (attr)
				noInterrruptionBeforeMinDuration = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";
			attr = stateNode->first_attribute("onFail");
			if (attr)
				restartOnFail = removeWhiteSpacesAndToLower(attr->value()) != "goback";

			std::vector<IGestureRecognizer*> recognizerRefs;
			xml_node<>* recRefNode;
			for(recRefNode = stateNode->first_node("Recognizer"); recRefNode; recRefNode = recRefNode->next_sibling("Recognizer"))
			{
				xml_attribute<>* NAttr = recRefNode->first_attribute("name");
				if (NAttr)
				{
					bool ignoreOnTrackingError = false;
					xml_attribute<>* attr1 = recRefNode->first_attribute("ignoreOnTrackingError");
					if (attr1)
						ignoreOnTrackingError = strcmp(attr1->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr1->value()) != "false";

					float minConf = -1.0f;
					xml_attribute<>* minConfA = recRefNode->first_attribute("minConfidence");
					if (minConfA)
						minConf = (float)atof(minConfA->value());

					bool useFilteredData = false;
					xml_attribute<>* filterA = recRefNode->first_attribute("useFilteredData");
					if (filterA)
						useFilteredData = strcmp(filterA->value(), "0") != 0 && removeWhiteSpacesAndToLower(filterA->value()) != "false";

					IGestureRecognizer* gestRec = core->cloneRecognizer(NAttr->value());
					if (gestRec)
					{
						// found it
						recognizerRefs.push_back(gestRec->clone());
						recognizerRefs.back()->m_ignoreOnTrackingError = ignoreOnTrackingError;
						if (minConf >= 0)
							recognizerRefs.back()->m_minConfidence = minConf;
						if (filterA)
							recognizerRefs.back()->m_useFilteredData = useFilteredData;
					}
					else
					{
						// Not found
						Fubi_logErr("XML_Error - Unknown reference \"%s\" in \"%s\"!\n", NAttr->value(), rec->getName().c_str());
					}
				}
			}

			std::vector<IGestureRecognizer*> notRecognizerRefs;
			xml_node<>* notRecRefNode;
			for(notRecRefNode = stateNode->first_node("NotRecognizer"); notRecRefNode; notRecRefNode = notRecRefNode->next_sibling("NotRecognizer"))
			{
				xml_attribute<>* recAttr = notRecRefNode->first_attribute("name");
				if (recAttr)
				{
					// Default for not recognizers is true, as with a tracking error there is also no recognition
					bool ignoreOnTrackingError = true;
					xml_attribute<>* attr1 = notRecRefNode->first_attribute("ignoreOnTrackingError");
					if (attr1)
						ignoreOnTrackingError = strcmp(attr1->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr1->value()) != "false";

					float minConf = -1.0f;
					xml_attribute<>* minConfA = notRecRefNode->first_attribute("minConfidence");
					if (minConfA)
						minConf = (float)atof(minConfA->value());

					bool useFilteredData = false;
					xml_attribute<>* filterA = notRecRefNode->first_attribute("useFilteredData");
					if (filterA)
						useFilteredData = strcmp(filterA->value(), "0") != 0 && removeWhiteSpacesAndToLower(filterA->value()) != "false";


					IGestureRecognizer* gestRec = core->cloneRecognizer(recAttr->value());
					if (gestRec)
					{
						// found it
						notRecognizerRefs.push_back(gestRec->clone());
						notRecognizerRefs.back()->m_ignoreOnTrackingError = ignoreOnTrackingError;
						if (minConf >= 0)
							notRecognizerRefs.back()->m_minConfidence = minConf;
						if (filterA)
							notRecognizerRefs.back()->m_useFilteredData = useFilteredData;
					}
					else
					{
						// Not found
						Fubi_logErr("XML_Error - Unknown reference \"%s\" in \"%s\"!\n", recAttr->value(), rec->getName().c_str());
					}

				}
			}

			// Now check for alternative recognizers in the same way
			std::vector<IGestureRecognizer*> alternativeRecognizerRefs;
			std::vector<IGestureRecognizer*> alternativeNotRecognizerRefs;
			xml_node<>* alternativesNode = stateNode->first_node("AlternativeRecognizers");
			if (alternativesNode)
			{
				xml_node<>* alternativeRecRefNode;
				for(alternativeRecRefNode = alternativesNode->first_node("Recognizer"); alternativeRecRefNode; alternativeRecRefNode = alternativeRecRefNode->next_sibling("Recognizer"))
				{
					xml_attribute<>* recAttr = alternativeRecRefNode->first_attribute("name");
					if (recAttr)
					{
						bool ignoreOnTrackingError = false;
						xml_attribute<>* attr1 = alternativeRecRefNode->first_attribute("ignoreOnTrackingError");
						if (attr1)
							ignoreOnTrackingError = strcmp(attr1->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr1->value()) != "false";

						float minConf = -1.0f;
						xml_attribute<>* minConfA = alternativeRecRefNode->first_attribute("minConfidence");
						if (minConfA)
							minConf = (float)atof(minConfA->value());

						bool useFilteredData = false;
						xml_attribute<>* filterA = alternativeRecRefNode->first_attribute("useFilteredData");
						if (filterA)
							useFilteredData = strcmp(filterA->value(), "0") != 0 && removeWhiteSpacesAndToLower(filterA->value()) != "false";

						IGestureRecognizer* gestRec = core->cloneRecognizer(recAttr->value());
						if (gestRec)
						{
							// found it
							alternativeRecognizerRefs.push_back(gestRec->clone());
							alternativeRecognizerRefs.back()->m_ignoreOnTrackingError = ignoreOnTrackingError;
							if (minConf >= 0)
								alternativeRecognizerRefs.back()->m_minConfidence = minConf;
							if (filterA)
								alternativeRecognizerRefs.back()->m_useFilteredData = useFilteredData;
						}
						else
						{
							// Not found
							Fubi_logErr("XML_Error - Unknown reference \"%s\" in \"%s\"!\n", recAttr->value(), rec->getName().c_str());
						}
					}
				}

				xml_node<>* alternativeNotRecRefNode;
				for(alternativeNotRecRefNode = alternativesNode->first_node("NotRecognizer"); alternativeNotRecRefNode; alternativeNotRecRefNode = alternativeNotRecRefNode->next_sibling("NotRecognizer"))
				{
					xml_attribute<>* recAttr = alternativeNotRecRefNode->first_attribute("name");
					if (recAttr)
					{
						// Default for not recognizers is true, as with a tracking error there is also no recognition
						bool ignoreOnTrackingError = true;
						xml_attribute<>* attr1 = alternativeNotRecRefNode->first_attribute("ignoreOnTrackingError");
						if (attr1)
							ignoreOnTrackingError = strcmp(attr1->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr1->value()) != "false";

						float minConf = -1.0f;
						xml_attribute<>* minConfA = alternativeNotRecRefNode->first_attribute("minConfidence");
						if (minConfA)
							minConf = (float)atof(minConfA->value());

						bool useFilteredData = false;
						xml_attribute<>* filterA = alternativeNotRecRefNode->first_attribute("useFilteredData");
						if (filterA)
							useFilteredData = strcmp(filterA->value(), "0") != 0 && removeWhiteSpacesAndToLower(filterA->value()) != "false";

						IGestureRecognizer* gestRec = core->cloneRecognizer(recAttr->value());
						if (gestRec)
						{
							// found it
							alternativeNotRecognizerRefs.push_back(gestRec->clone());
							alternativeNotRecognizerRefs.back()->m_ignoreOnTrackingError = ignoreOnTrackingError;
							if (minConf >= 0)
								alternativeNotRecognizerRefs.back()->m_minConfidence = minConf;
							if (filterA)
								alternativeNotRecognizerRefs.back()->m_useFilteredData = useFilteredData;
						}
						else
						{
							// Finally not found
							Fubi_logErr("XML_Error - Unknown reference \"%s\" in \"%s\"!\n", recAttr->value(), rec->getName().c_str());
						}

					}
				}
			}

			if (recognizerRefs.size() > 0 || notRecognizerRefs.size() > 0)
			{
				// Finally load any meta data
				std::map<std::string, std::string> metaInfo;
				xml_node<>* metaInfoNode = stateNode->first_node("METAINFO");
				if (metaInfoNode)
				{
					xml_node<>* metaProperty;
					for(metaProperty = metaInfoNode->first_node("Property"); metaProperty; metaProperty = metaProperty->next_sibling("Property"))
					{
						std::string metaName;
						xml_attribute<>* metaAttr = metaProperty->first_attribute("name");
						if (metaAttr && metaAttr->value() != 0x0)
						{
							metaName = metaAttr->value();
							metaAttr = metaProperty->first_attribute("value");
							if (metaAttr &&  metaAttr->value() != 0x0)
							{
								metaInfo[metaName] = metaAttr->value();
							}
						}
					}
				}


				// Add state to the recognizer
				rec->addState(recognizerRefs, notRecognizerRefs, minDuration, maxDuration, timeForTransition, maxInterruption,
					noInterrruptionBeforeMinDuration, alternativeRecognizerRefs, alternativeNotRecognizerRefs, restartOnFail, metaInfo);
			}
			else
			{
				// No recognizers in this state
				Fubi_logInfo("FubiXMLParser: XML_Error - No references in state %d of rec \"%s\"!\n", stateNum, rec->getName().c_str());
			}
		}

		if (rec->getNumStates() == 0)
		{
			// Invalid rec, so delete it
			delete rec;
			rec = 0x0;
		}
	}

	return rec;
}

IGestureRecognizer* FubiXMLParser::parseTemplateRecognizer(const std::string& xmlDefinition, unsigned int& requiredHistoryLength, std::string& name)
{
	IGestureRecognizer* rec = 0x0;

	// copy string to buffer
	char* buffer = new char[xmlDefinition.length() + 1];
#pragma warning(push)
#pragma warning(disable:4996)
	strcpy(buffer, xmlDefinition.c_str());
#pragma warning(pop)

	// parse XML
	xml_document<>* doc = new xml_document<>();
	doc->parse<0>(buffer);
	xml_node<>* node = doc->first_node("TemplateRecognizer");
	if (node)
	{
		bool isVisible;
		rec = parseTemplateRecognizer(node, -1, true, isVisible, name, requiredHistoryLength);
	}

	// release xml doc and buffer
	doc->clear();
	delete doc;
	delete[] buffer;

	return rec;
}

IGestureRecognizer* FubiXMLParser::parseTemplateRecognizer(xml_node<>* node, float globalMinConfidence, bool defaultUseFilteredData,
	bool& isVisible, std::string& name, unsigned int& requiredHistoryLength, const std::string& baseDir /*=""*/)
{
	xml_attribute<>* attr = node->first_attribute("name");
	if (attr)
		name = attr->value();

	attr = node->first_attribute("visibility");
	if (attr)
		isVisible = removeWhiteSpacesAndToLower(attr->value()) != "hidden";

	bool localTrans = false;
	attr = node->first_attribute("useLocalTransformations");
	if (attr)
		localTrans = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

	bool useOrientations = false;
	attr = node->first_attribute("useOrientations");
	if (attr)
		useOrientations = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

	float minConf = globalMinConfidence;
	attr = node->first_attribute("minConfidence");
	if (attr)
		minConf = (float)atof(attr->value());

	bool useFilteredData = defaultUseFilteredData;
	attr = node->first_attribute("useFilteredData");
	if (attr)
		useFilteredData = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

	BodyMeasurement::Measurement measure = BodyMeasurement::NUM_MEASUREMENTS;
	attr = node->first_attribute("measuringUnit");
	if (attr)
		measure = Fubi::getBodyMeasureID(attr->value());

	float maxDistance = 10.0f;
	attr = node->first_attribute("maxDistance");
	if (attr)
		maxDistance = (float)atof(attr->value());

	DistanceMeasure::Measure distanceMeasure = DistanceMeasure::Euclidean;
	attr = node->first_attribute("distanceMeasure");
	if (attr)
	{
		std::string lowerVal = removeWhiteSpacesAndToLower(attr->value());
		if (lowerVal == "manhattan")
			distanceMeasure = DistanceMeasure::Manhattan;
		else if (lowerVal == "malhanobis")
			distanceMeasure = DistanceMeasure::Malhanobis;
		else if (lowerVal == "turninganglediff")
			distanceMeasure = DistanceMeasure::TurningAngleDiff;
	}

	float maxRotation = 45.0f;
	attr = node->first_attribute("maxRotation");
	if (attr)
		maxRotation = (float)atof(attr->value());

	bool aspectInvariant = false;
	attr = node->first_attribute("aspectInvariant");
	if (attr)
		aspectInvariant = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

	bool searchBestInputLength = false;
	attr = node->first_attribute("searchBestInputLength");
	if (attr)
		searchBestInputLength = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

	vector<SkeletonJoint::Joint> joints, relJoints;
	for (xml_node<>* jointNode = node->first_node("Joints"); jointNode; jointNode = jointNode->next_sibling("Joints"))
	{
		attr = jointNode->first_attribute("main");
		if (attr)
		{
			joints.push_back(getJointID(attr->value()));
			attr = jointNode->first_attribute("relative");
			if (attr)
				relJoints.push_back(getJointID(attr->value()));
			else
				relJoints.push_back(SkeletonJoint::NUM_JOINTS);
		}
	}
	if (joints.size() == 0)
		joints.push_back(SkeletonJoint::RIGHT_WRIST);
	if (relJoints.size() == 0)
		relJoints.push_back(SkeletonJoint::NUM_JOINTS);

	RecognizerTarget::Target targetSkeleton = RecognizerTarget::BODY;
	SkeletonJoint::Joint baseJoint = (joints.size() == 1) ? joints.front() : SkeletonJoint::NUM_JOINTS;
	vector<SkeletonHandJoint::Joint> handJoints, relHandJoints;
	for (xml_node<>* handJointNode = node->first_node("HandJoints"); handJointNode; handJointNode = handJointNode->next_sibling("HandJoints"))
	{
		attr = handJointNode->first_attribute("main");
		if (attr)
		{
			SkeletonHandJoint::Joint hJoint = getHandJointID(attr->value());
			if (hJoint != SkeletonHandJoint::NUM_JOINTS)
			{
				if (baseJoint == SkeletonJoint::RIGHT_HAND)
					targetSkeleton = RecognizerTarget::RIGHT_HAND;
				else if (baseJoint == SkeletonJoint::LEFT_HAND)
					targetSkeleton = RecognizerTarget::LEFT_HAND;
				else
					targetSkeleton = RecognizerTarget::BOTH_HANDS;
				handJoints.push_back(hJoint);
				attr = handJointNode->first_attribute("relative");
				if (attr)
				{
					hJoint = getHandJointID(attr->value());
					if (hJoint == SkeletonHandJoint::NUM_JOINTS)
						relHandJoints.push_back(SkeletonHandJoint::NUM_JOINTS);
					else
						relHandJoints.push_back( hJoint);
				}
				else
					relHandJoints.push_back(SkeletonHandJoint::NUM_JOINTS);
			}
		}
	}

	std::vector<std::deque<Fubi::TrackingData>> trainingData;
	Fubi::BodyMeasurementDistance bodyMeasures[BodyMeasurement::NUM_MEASUREMENTS];
	for (xml_node<>* dataNode = node->first_node("TrainingData"); dataNode; dataNode = dataNode->next_sibling("TrainingData"))
	{
		int start = 0, end = -1;
		attr = dataNode->first_attribute("start");
		if (attr)
			start = atoi(attr->value());
		attr = dataNode->first_attribute("end");
		if (attr)
			end = atoi(attr->value());
		attr = dataNode->first_attribute("file");
		if (attr && attr->value())
		{
			std::string fileName = attr->value();
			if (!baseDir.empty() && fileName.find(":") == string::npos && fileName.front() != '/')
			{
				fileName = baseDir + "/" + fileName;
			}
			int startMarker, endMarker;
			std::deque<Fubi::TrackingData> skeletonData;
			std::deque<std::pair<int, int>> fingerCounts;
			bool isUser;
			if (parseSkeletonFile(skeletonData, bodyMeasures, fingerCounts, startMarker, endMarker, isUser, fileName, start, end))
			{
				if (skeletonData.size() > 0)
					trainingData.push_back(skeletonData);
				requiredHistoryLength = maxi(requiredHistoryLength, (int)skeletonData.size());
			}
		}
	}

	unsigned int ignoreAxes = 0;
	xml_node<>* ignoreAxesNode = node->first_node("IgnoreAxes");
	if (ignoreAxesNode)
	{
		attr = ignoreAxesNode->first_attribute("x");
		if (attr && strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false")
			ignoreAxes |= CoordinateAxis::X;
		attr = ignoreAxesNode->first_attribute("y");
		if (attr && strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false")
			ignoreAxes |= CoordinateAxis::Y;
		attr = ignoreAxesNode->first_attribute("z");
		if (attr && strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false")
			ignoreAxes |= CoordinateAxis::Z;
	}

	bool useDTW = true;
	attr = node->first_attribute("useDTW");
	if (attr)
		useDTW = strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false";

	float maxWarpingFac = 0.5f;
	attr = node->first_attribute("maxWarpingFactor");
	if (attr)
		maxWarpingFac = (float)atof(attr->value());

	Fubi::ResamplingTechnique::Technique resamplingTechnique = Fubi::ResamplingTechnique::None;
	attr = node->first_attribute("resamplingTechnique");
	if (attr)
	{
		std::string lowerVal = removeWhiteSpacesAndToLower(attr->value());
		if (lowerVal == "hermitespline" || lowerVal == "hermittespline")
		{
			resamplingTechnique = ResamplingTechnique::HermiteSpline;
		}
		else if (lowerVal == "polyline")
		{
			resamplingTechnique = ResamplingTechnique::PolyLine;
		}
		else /*if (lowerVal == "equidistant")*/
		{
			resamplingTechnique = ResamplingTechnique::EquiDistant;
		}
	}
	else
	{
		attr = node->first_attribute("applyResampling");
		if (attr && strcmp(attr->value(), "0") != 0 && removeWhiteSpacesAndToLower(attr->value()) != "false")
			resamplingTechnique = ResamplingTechnique::HermiteSpline;
	}

	int resampleSize = -1;
	attr = node->first_attribute("resampleSize");
	if (attr)
		resampleSize = atoi(attr->value());

	StochasticModel::Model stochasticModel = StochasticModel::GMR;
	attr = node->first_attribute("stochasticModel");
	if (attr)
	{
		std::string lowerVal = removeWhiteSpacesAndToLower(attr->value());
		if (lowerVal == "none")
			stochasticModel = StochasticModel::NONE;
		/*else if (lowerVal == "hmm")
			stochasticModel = StochasticModel::HMM;*/
	}

	unsigned int numGMRStates = 5;
	attr = node->first_attribute("numGMRStates");
	if (attr)
		numGMRStates = (unsigned)atoi(attr->value());

	IGestureRecognizer* rec = 0x0;
	if (handJoints.size() > 0 && trainingData.size() > 0)
		rec = createGestureRecognizer(handJoints, relHandJoints, trainingData, maxDistance,
		distanceMeasure, maxRotation, aspectInvariant, ignoreAxes, useOrientations, useDTW, maxWarpingFac, resamplingTechnique,
		resampleSize, searchBestInputLength, stochasticModel, numGMRStates, minConf, localTrans, useFilteredData);
	else if (joints.size() > 0 && trainingData.size() > 0)
		rec = createGestureRecognizer(joints, relJoints, trainingData, maxDistance,
		distanceMeasure, maxRotation, aspectInvariant, ignoreAxes, useOrientations,
		useDTW, maxWarpingFac, resamplingTechnique, resampleSize, searchBestInputLength, stochasticModel, numGMRStates,
		bodyMeasures, measure, minConf, localTrans, useFilteredData);
	if (rec)
		rec->m_targetSkeletonType = targetSkeleton;
	return rec;
}

bool FubiXMLParser::parseSkeletonFile(std::deque<Fubi::TrackingData>& skeletonData,	Fubi::BodyMeasurementDistance* bodyMeasures, 
	std::deque<std::pair<int, int>>& fingerCounts, int& startMarker, int& endMarker, bool& isUser, const std::string& fileName, int start /*= 0*/, int end /*= -1*/)
{
	// Open the file
	fstream file;
	file.open(fileName, fstream::in | fstream::binary);
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
		xml_document<>* doc = new xml_document<>();   // character type defaults to char
		try
		{
			doc->parse<0>(buffer);
		}
		catch (std::exception& e)
		{
			Fubi_logWrn("Error parsing recording XML: %s", e.what());
			return false;
		}

		// Parse the content
		xml_node<>* rootNode = doc->first_node("FubiRecording");
		if (rootNode)
		{
			xml_attribute<>* attr = rootNode->first_attribute("handID");
			isUser = (attr) ? false : true;

			// Get start and end frame ID if set
			attr = rootNode->first_attribute("startMarker");
			if (attr)
				startMarker = atoi(attr->value());
			attr = rootNode->first_attribute("endMarker");
			if (attr)
				endMarker= atoi(attr->value());

			xml_node<>* measuresNode = rootNode->first_node("BodyMeasurements");
			if (measuresNode)
			{
				if (!isUser)
					Fubi_logWrn("Invalid BodyMeasurements node within recorded Hand data! Data probably corrupt.");
				else
				{
					for (xml_node<>* node = measuresNode->first_node("Measurement"); node; node = node->next_sibling("Measurement"))
					{
						attr = node->first_attribute("name");
						if (attr)
						{
							BodyMeasurement::Measurement measureID = getBodyMeasureID(attr->value());
							if (measureID != BodyMeasurement::NUM_MEASUREMENTS)
							{
								attr = node->first_attribute("value");
								if (attr)
									bodyMeasures[measureID].m_dist = (float)atof(attr->value());
								attr = node->first_attribute("confidence");
								if (attr)
									bodyMeasures[measureID].m_confidence = (float)atof(attr->value());
							}
						}
					}
				}
			}

			int frameID = 0;
			for (xml_node<>* frameNode = rootNode->first_node("Frame"); frameNode && (end < 0 || frameID <= end); frameNode = frameNode->next_sibling("Frame"), ++frameID)
			{
				if (frameID < start)
					continue;
				TrackingData dataFrame = isUser ? (TrackingData)UserTrackingData() : (TrackingData)FingerTrackingData();
				attr = frameNode->first_attribute("time");
				if (attr)
					dataFrame.timeStamp = atof(attr->value());

				xml_node<>* jointPositions = frameNode->first_node("JointPositions");
				if (jointPositions)
				{
					for (xml_node<>* jointNode = jointPositions->first_node("Joint"); jointNode; jointNode = jointNode->next_sibling("Joint"))
					{
						attr = jointNode->first_attribute("name");
						if (attr)
						{
							int jointID = isUser ? getJointID(attr->value()) : getHandJointID(attr->value());
							if (jointID != SkeletonJoint::NUM_JOINTS && (isUser || jointID != SkeletonHandJoint::NUM_JOINTS))
							{
								SkeletonJointPosition& pos = dataFrame.jointPositions[jointID];
								parseJointData(jointNode, pos.m_position, pos.m_confidence);
							}
						}
					}
				}


				xml_node<>* jointOrientations = frameNode->first_node("JointOrientations");
				if (jointOrientations)
				{
					for (xml_node<>* jointNode = jointOrientations->first_node("Joint"); jointNode; jointNode = jointNode->next_sibling("Joint"))
					{
						attr = jointNode->first_attribute("name");
						if (attr)
						{
							int jointID = isUser ? getJointID(attr->value()) : getHandJointID(attr->value());
							if (jointID != SkeletonJoint::NUM_JOINTS && (isUser || jointID != SkeletonHandJoint::NUM_JOINTS))
							{
								SkeletonJointOrientation& orient = dataFrame.jointOrientations[jointID];
								Vec3f rot;
								parseJointData(jointNode, rot, orient.m_confidence);
								degToRad(rot);
								orient.m_orientation = Matrix3f::RotMat(rot);
							}
						}
					}
				}

				int leftFingerCount = -1, rightFingerCount = -1;
				xml_node<>* fingerCount = frameNode->first_node("FingerCounts");
				if (fingerCount)
				{
					attr = fingerCount->first_attribute("left");
					if (attr)
						leftFingerCount = atoi(attr->value());
					attr = fingerCount->first_attribute("right");
					if (attr)
						rightFingerCount = atoi(attr->value());
				}
				skeletonData.push_back(dataFrame);
				fingerCounts.push_back(std::pair<int, int>(leftFingerCount, rightFingerCount));
			}
		}
	}
	else
	{
		Fubi_logErr("Error opening skeleton file (already openend in another application?): %s \n", fileName.c_str());
		return false;
	}
	return true;
}

void FubiXMLParser::parseJointData(xml_node<char>* jointNode, Vec3f& trans, float& confidence)
{
	xml_attribute<>* attr = jointNode->first_attribute("x");
	if (attr)
		trans.x = (float)atof(attr->value());
	attr = jointNode->first_attribute("y");
	if (attr)
		trans.y = (float)atof(attr->value());
	attr = jointNode->first_attribute("z");
	if (attr)
		trans.z = (float)atof(attr->value());
	attr = jointNode->first_attribute("confidence");
	if (attr)
		confidence = (float)atof(attr->value());
}