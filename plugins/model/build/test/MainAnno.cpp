// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/11
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

#include "ssi.h"
#include "ssiml/include/ssiml.h"
#include "ssimodel.h"
using namespace ssi;

bool ex_csv(void *args);
bool ex_stream(void *args);
bool ex_ssi(void *args);
bool ex_edit_classes(void *args);
bool ex_convert_to_frames(void *args);
bool ex_convert_to_stream(void *args);
bool ex_create_samples(void *args);
bool ex_filter(void *args);
bool ex_pack_class(void *args);
bool ex_eval(void *args);
bool ex_elan(void *args);

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ssi_random_seed ();

	Exsemble exsemble;
	exsemble.console(0, 0, 650, 800);
	exsemble.add(&ex_ssi, 0, "SSI", "Writes/reads a continuous/discrete/free annotation to/from a ssi file");
	exsemble.add(&ex_csv, 0, "CSV", "Reads a continuous/discrete/free annotation from a CSV file");
	exsemble.add(&ex_stream, 0, "STREAM", "Converts a stream into a continuous/discrete annotation");
	exsemble.add(&ex_edit_classes, 0, "EDIT CLASSES", "Edit classes in a discrete annotation.");
	exsemble.add(&ex_convert_to_frames, 0, "CONVERT TO FRAMES", "Convert a discrete annotation to frames.");
	exsemble.add(&ex_convert_to_stream, 0, "CONVERT TO STREAM", "Convert an annotation to a stream.");
	exsemble.add(&ex_create_samples, 0, "CREATE SAMPLES", "Create a sample list from a discrete annotation.");
	exsemble.add(&ex_filter, 0, "FILTER", "Remove labels with small confidence/duration.");
	exsemble.add(&ex_pack_class, 0, "PACK CLASS", "Combine successive samples of same class.");
	exsemble.add(&ex_eval, 0, "EVALUATION", "Framewise comparison of two annotations.");
	exsemble.add(&ex_elan, 0, "READ ELAN", "Read an ELAN annotation.");
	exsemble.show();

	ssi_print ("\n\n\tpress enter to quit\n\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}


bool ex_ssi(void *args) {

	{
		StringList classes;
		classes.add("apple");
		classes.add("banana");
		classes.add("coconut");

		Annotation anno;
		anno.setDiscreteScheme("discrete", classes);
		anno.add(1.0, 2.0, "apple", 1.0f);
		anno.add(2.0, 1.0, "banana", 0.9f);
		anno.add(3.0, 4.0, "coconut", 0.5f);
		anno.add(5.0, 6.0, 0, 0.4f);
		anno.add(7.0, 8.0, 10, 1.0f);
		anno.add(8.0, 9.0, "apple", 1.0f);
		anno.add(9.0, 10.0, "chocolate", 0.0f);
		anno.setMeta("annotator", "Hans");
		anno.setMeta("role", "Expert");

		anno.save("discrete_a", File::ASCII);
		anno.save("discrete_b", File::BINARY);

		anno.load("discrete_a");
		anno.print();
		anno.load("discrete_b");
		anno.print();
	}

	{
		Annotation anno;
		anno.setContinuousScheme("continuous", 1.0, 0.0f, 1.0f);
		anno.add(0.2f, 1.0f);
		anno.add(0.5f, 0.2f);
		anno.add(1.2f, 0.5f);
		anno.add(0.0f, 0.6f);
		anno.add(0.1f, 1.0f);
		anno.setMeta("annotator", "Max");
		anno.setMeta("role", "Novice");

		anno.save("continuous_a", File::ASCII);
		anno.save("continuous_b", File::BINARY);

		anno.load("continuous_a");
		anno.print();
		anno.load("continuous_b");
		anno.print();
	}

	{
		Annotation anno;
		anno.setFreeScheme("free");
		anno.add(1.0, 2.0, "applè", 0.1f);
		anno.add(2.0, 1.0, "bananà", 0.6f);
		anno.add(3.0, 4.0, "coconut", 0.9f);
		anno.add(8.0, 9.0, "", 1.0f);
		anno.add(9.0, 10.0, "chocolatè", 0.0f);

		anno.save("free_a", File::ASCII);
		anno.save("free_b", File::BINARY);

		anno.load("free_a");
		anno.print();
		anno.load("free_b");
		anno.print();
	}

	return true;
}

bool ex_csv(void *args) {

	FileCSV file;
	const ssi_char_t *csv_path_d = "in/anno_d.csv";
	const ssi_char_t *csv_path_c = "in/anno_c.csv";

	StringList classes;
	classes.add("apple");
	classes.add("bananà");
	classes.add("coconut");

	ssi_print("\nread discrete/free csv via column names:\n\n");

	if (file.parseFile(csv_path_d, ',', true))
	{
		Annotation anno;

		anno.setDiscreteScheme("discrete", classes);
		anno.addCSV(file, "from", "to", "name", "conf");
		anno.addCSV(file, "from", "to", "name", 1.0f);
		anno.print();

		anno.setFreeScheme("free");
		anno.addCSV(file, "from", "to", "name", "conf");
		anno.addCSV(file, "from", "to", "name", 1.0f);
		anno.print();
	}
	else
	{
		ssi_wrn("file not found '%s'\n\n", csv_path_d);
	}
	file.clear();

	ssi_print("\nread discrete/free csv via column indices:\n\n");

	if (file.parseFile(csv_path_d, ',', true))
	{
		Annotation anno;
		anno.setDiscreteScheme("discrete", classes);
		anno.addCSV(file, 0u, 1u, 2u, 3u);
		anno.addCSV(file, 0u, 1u, 2u, 1.0f);
		anno.print();
		anno.setFreeScheme("free");
		anno.addCSV(file, 0u, 1u, 2u, 3u);
		anno.addCSV(file, 0u, 1u, 2u, 1.0f);
		anno.print();
	}
	else
	{
		ssi_wrn("file not found '%s'\n\n", csv_path_d);
	}
	file.clear();

	ssi_print("\nread continuous csv via column names:\n\n");

	if (file.parseFile(csv_path_c, ',', true))
	{
		Annotation anno;
		anno.setContinuousScheme("continuous", 1.0, 0.0f, 1.0f);
		anno.addCSV(file, "score", "conf");
		anno.addCSV(file, "score", 1.0f);
		anno.print();
	}
	else
	{
		ssi_wrn("file not found '%s'\n\n", csv_path_c);
	}
	file.clear();

	ssi_print("\nread continuous csv via column indices:\n\n");

	if (file.parseFile(csv_path_c, ',', true))
	{
		Annotation anno;
		anno.setContinuousScheme("continuous", 1.0, 0.0f, 1.0f);
		anno.addCSV(file, 0u, 1u);
		anno.addCSV(file, 0u, 1.0f);
		anno.print();
	}
	else
	{
		ssi_wrn("file not found '%s'\n\n", csv_path_c);
	}
	file.clear();

	return true;
}

bool ex_stream(void *args) {

	const ssi_char_t *stream_path = "in/anno";

	ssi_stream_t stream;
	FileTools::ReadStreamFile(stream_path, stream);

	ssi_print("\nconvert stream to continuous annotation:\n\n");

	{
		Annotation anno;
		anno.setContinuousScheme("arousal", stream.sr, 0.0f, 1.0f);
		anno.addStream(stream, 1u, 0u);
		anno.addStream(stream, 0u, 1.0f);
		anno.print();
	}

	ssi_print("\nconvert stream to discrete annotation:\n\n");

	{
		Annotation anno;
		anno.setDiscreteScheme("activity");
		anno.addClass(0, "ACTIVITY");
		anno.addStream(stream, 0, 0, 0, 1.0f, 0.07);
		anno.print();
	}

	ssi_stream_destroy(stream);

	return true;
}

bool ex_edit_classes(void *args)
{
	StringList classes;
	classes.add("apple");
	classes.add("banana");
	classes.add("coconut");

	Annotation anno;
	anno.setDiscreteScheme("discrete", classes);
	anno.add(1.0, 2.0, "apple", 1.0f);
	anno.add(2.0, 1.0, "banana", 0.9f);
	anno.add(3.0, 4.0, "coconut", 0.5f);
	anno.add(5.0, 6.0, -1, 0.4f);
	anno.add(7.0, 8.0, "apple", 1.0f);
	anno.add(8.0, 9.0, "banana", 1.0f);
	anno.add(9.0, 10.0, "coconut", 0.0f);
	anno.setMeta("annotator", "Hans");
	anno.setMeta("role", "Wurscht");
	anno.print();

	{
		Annotation anno_banana_and_garbage_only(anno);
		StringList remove;
		remove.push_back("apple");
		remove.push_back("coconut");
		anno_banana_and_garbage_only.removeClass(remove);
		anno_banana_and_garbage_only.print();

		Annotation anno_banana_only(anno_banana_and_garbage_only);
		anno_banana_only.removeClass(SSI_SAMPLE_GARBAGE_CLASS_NAME);
		anno_banana_only.print();
	}

	{
		Annotation anno_banana_and_garbage_only(anno);
		StringList keep;
		keep.push_back("banana");
		keep.push_back(SSI_SAMPLE_GARBAGE_CLASS_NAME);
		anno_banana_and_garbage_only.keepClass(keep);
		anno_banana_and_garbage_only.print();

		Annotation anno_banana_only(anno_banana_and_garbage_only);
		anno_banana_only.keepClass("banana");
		anno_banana_only.print();
	}

	{
		Annotation anno_ex(anno);
		StringList keep;
		keep.push_back("apple");
		keep.push_back("cheese");
		keep.push_back("wurst");		
		anno_ex.addClass(keep);

		anno_ex.add(11.0, 12.0, "cheese", 0.7f);
		anno_ex.add(12.0, 13.0, "wurst", 0.3f);

		anno_ex.print();
	}

	{
		Annotation anno_ex(anno);
		anno_ex.renameClass("apple", "abble");
		anno_ex.mapClass("banana", "coconut");
		anno_ex.print();
	}

	return true;
}

bool ex_convert_to_frames(void *args)
{
	std::map<String,ssi_size_t> classes;
	classes["apple"] = 3;
	classes["banana"] = 5;
	classes["coconut"] = 1;

	Annotation anno;
	anno.setDiscreteScheme("discrete", classes);
	anno.add(1.0, 1.5, "apple", 1.0f);
	anno.add(2.0, 2.1, "banana", 0.9f);	
	anno.add(2.1, 2.2, "apple", 0.9f);
	anno.add(2.5, 3.0, "coconut", 0.9f);
	anno.add(5.0, 6.0, -1, 1.0f);
	anno.add(7.0, 10.0, "coconut", 0.0f);
	anno.setMeta("annotator", "Hans");
	anno.setMeta("role", "Wurscht");
	anno.print();

	{
		Annotation anno_frames(anno);
		anno_frames.convertToFrames(1.0, 0);
		anno_frames.print();
	}
	
	{
		Annotation anno_frames(anno);
		anno_frames.convertToFrames(1.0, 0, 0.0, 0.8f);
		anno_frames.print();
	}

	{
		Annotation anno_frames(anno);
		anno_frames.convertToFrames(1.0, "empty", 12.0, 0.5f);
		anno_frames.print();
	}

	{
		Annotation anno_frames(anno);
		anno_frames.convertToFrames(1.0, "apple", 12.0, 0.5f);
		anno_frames.print();
	}

	return true;
}

bool ex_convert_to_stream(void *args)
{
	{
		std::map<String, ssi_size_t> classes;
		classes["apple"] = 0;
		classes["banana"] = 1;
		classes["coconut"] = 2;

		Annotation anno;
		anno.setDiscreteScheme("discrete", classes);
		anno.add(1.0, 1.5, "apple", 1.0f);
		anno.add(2.0, 2.1, "banana", 0.9f);
		anno.add(2.1, 2.2, "apple", 0.9f);
		anno.add(2.5, 3.0, "coconut", 0.9f);
		anno.add(5.0, 6.0, -1, 1.0f);
		anno.add(7.0, 10.0, "coconut", 0.0f);
		anno.setMeta("annotator", "Hans");
		anno.setMeta("role", "Wurscht");
		anno.print();

		ssi_stream_t stream;
		anno.convertToStream(stream, 5.0, 12.0);
		ssi_stream_print(stream, ssiout);
		ssi_stream_destroy(stream);
	}

	{
		{
			Annotation anno;
			anno.setContinuousScheme("continuous", 1.0, 0.0f, 1.0f);
			anno.add(0.2f, 1.0f);
			anno.add(0.5f, 0.2f);
			anno.add(1.0f, 0.5f);
			anno.add(0.0f, 0.6f);
			anno.add(0.1f, 1.0f);
			anno.setMeta("annotator", "Max");
			anno.setMeta("role", "Novice");

			anno.print();

			ssi_stream_t stream;
			anno.convertToStream(stream, 10.0, 12.0);
			ssi_stream_print(stream, ssiout);
			ssi_stream_destroy(stream);
		}
	}

	return true;
}

bool ex_create_samples(void *args)
{
	{


		std::map<String, ssi_size_t> classes;
		classes["apple"] = 3;
		classes["banana"] = 5;
		classes["coconut"] = 1;

		Annotation anno;
		anno.setDiscreteScheme("discrete", classes);
		anno.add(1.0, 1.5, "apple", 1.0f);
		anno.add(2.0, 2.1, "banana", 0.9f);
		anno.add(2.1, 2.2, "apple", 0.9f);
		anno.add(2.5, 3.0, "coconut", 0.9f);
		anno.add(5.0, 6.0, -1, 1.0f);
		anno.add(7.0, 10.0, "coconut", 0.0f);
		anno.setMeta("annotator", "Hans");
		anno.setMeta("role", "Wurscht");
		anno.print();

		ssi_stream_t stream;
		ssi_stream_init(stream, 8, 1, sizeof(ssi_real_t), SSI_REAL, 1.0);
		ssi_stream_zero(stream);

		SampleList samples;
		anno.extractSamples(stream, &samples);

		ModelTools::PrintSamples(samples);

		ssi_stream_destroy(stream);

	}

	{
		{
			Annotation anno;
			anno.setContinuousScheme("continuous", 1.0, 0.0f, 1.0f);
			anno.add(0.2f, 1.0f);
			anno.add(0.5f, 0.2f);
			anno.add(0.8f, 0.5f);
			anno.add(0.0f, 0.6f);
			anno.add(0.1f, 1.0f);
			anno.setMeta("annotator", "Max");
			anno.setMeta("role", "Novice");

			ssi_stream_t stream;
			ssi_stream_init(stream, 5, 1, sizeof(ssi_real_t), SSI_REAL, 1.0);
			ssi_stream_zero(stream);

			SampleList samples;
			anno.extractSamples(stream, &samples);

			ModelTools::PrintSamples(samples);

			ssi_stream_destroy(stream);
		}
	}

	return true;
}

bool ex_filter(void *args)
{
	std::map<String, ssi_size_t> classes;
	classes["apple"] = 3;
	classes["banana"] = 5;
	classes["coconut"] = 1;

	Annotation anno;
	anno.setDiscreteScheme("discrete", classes);
	anno.add(0.9, 1.0, "apple", 1.0f);
	anno.add(1.0, 1.5, "apple", 1.0f);
	anno.add(1.5, 2.0, "apple", 0.2f);
	anno.add(2.5, 3.0, "apple", 0.9f);
	anno.add(3.1, 3.2, "banana", 0.1f);
	anno.add(3.2, 3.5, "apple", 0.9f);
	anno.add(2.5, 3.0, "coconut", 0.9f);
	anno.add(4.0, 5.0, -1, 0.0f);
	anno.add(5.0, 6.0, -1, 1.0f);
	anno.add(7.0, 10.0, "coconut", 0.0f);
	anno.add(8.0, 9.0, "coconut", 0.0f);
	anno.setMeta("annotator", "Hans");
	anno.setMeta("role", "Wurscht");
	anno.print();

	anno.filter(0.9, Annotation::FILTER_PROPERTY::CONFIDENCE, Annotation::FILTER_OPERATOR::LESSER_EQUAL, "apple");
	anno.print();

	anno.filter(0.9, Annotation::FILTER_PROPERTY::CONFIDENCE, Annotation::FILTER_OPERATOR::LESSER_EQUAL);
	anno.print();

	anno.filter(0.5, Annotation::FILTER_PROPERTY::DURATION, Annotation::FILTER_OPERATOR::GREATER_EQUAL);
	anno.print();

	anno.filter(3.0, Annotation::FILTER_PROPERTY::FROM, Annotation::FILTER_OPERATOR::GREATER);
	anno.print();

	anno.filter(5.0, Annotation::FILTER_PROPERTY::TO, Annotation::FILTER_OPERATOR::LESSER_EQUAL);
	anno.print();

	return true;
}

bool ex_pack_class(void *args)
{
	std::map<String, ssi_size_t> classes;
	classes["apple"] = 3;
	classes["banana"] = 5;
	classes["coconut"] = 1;

	Annotation anno;
	anno.setDiscreteScheme("discrete", classes);
	anno.add(0.5, 1.0, "apple", 1.0f);
	anno.add(1.0, 1.5, "apple", 1.0f);
	anno.add(1.5, 2.0, "apple", 1.0f);
	anno.add(2.5, 3.0, "apple", 0.9f);
	anno.add(3.1, 3.2, "banana", 0.9f);
	anno.add(3.2, 3.5, "apple", 0.9f);
	anno.add(2.5, 3.0, "coconut", 0.9f);
	anno.add(4.0, 5.0, -1, 0.0f);
	anno.add(5.0, 6.0, -1, 1.0f);
	anno.add(7.0, 10.0, "coconut", 0.0f);
	anno.add(8.0, 9.0, "coconut", 0.0f);
	anno.setMeta("annotator", "Hans");
	anno.setMeta("role", "Wurscht");
	anno.print();

	{
		Annotation anno_pack(anno);
		anno_pack.packClass(0, "apple");
		anno_pack.print();
	}

	{
		Annotation anno_pack(anno);
		anno_pack.packClass(0);
		anno_pack.print();
	}

	{
		Annotation anno_pack(anno);
		anno_pack.packClass(1.0, "apple");
		anno_pack.print();
	}

	{
		Annotation anno_pack(anno);
		anno_pack.packClass(1.0);
		anno_pack.print();
	}

	return true;
}

bool ex_eval(void *args)
{
	std::map<String, ssi_size_t> classes;
	classes["apple"] = 3;
	classes["banana"] = 5;
	classes["coconut"] = 1;

	ssi_time_t duration = 4.0;

	Annotation truth;
	truth.setDiscreteScheme("discrete", classes);
	truth.add(0.5, 1.0, "apple", 1.0f);
	truth.add(1.0, 1.5, "apple", 1.0f);
	truth.add(1.5, 2.0, -1, 1.0f);
	truth.add(2.5, 3.0, "apple", 0.9f);
	truth.add(3.1, 3.2, "banana", 0.9f);
	truth.add(3.2, 3.5, "apple", 0.9f);
	truth.add(2.5, 3.0, "coconut", 0.9f);		
	truth.print();

	truth.convertToFrames(0.1, "none", duration, 0.5f);

	Evaluation eval;
	eval.eval(&truth, &truth);
	eval.print();

	Annotation prediction;
	prediction.setDiscreteScheme("discrete", classes);
	prediction.add(0.3, 1.1, "apple", 1.0f);	
	prediction.add(1.5, 2.0, "banana", 1.0f);
	prediction.add(2.5, 3.0, "apple", 0.9f);
	prediction.add(3.1, 3.2, "banana", 0.9f);
	prediction.add(3.2, 3.5, "coconut", 0.9f);
	prediction.add(2.5, 3.0, "apple", 0.9f);
	prediction.print();

	prediction.convertToFrames(0.1, "none", duration, 0.5f);
	
	eval.eval(&prediction, &truth);
	eval.print();

	return true;
}

bool ex_elan(void *args)
{
	//const ssi_char_t *filepath = "in\\test.eaf";
	ssi_char_t filepath[SSI_MAX_CHAR];

	int sess[19] = { 10, 11, 112, 113, 2, 3, 34, 35, 36, 37, 42, 43, 48, 5, 79, 8, 94, 95, 97 };

	for (int i = 0; i < 19; i++)
	{
		ssi_sprint(filepath, "X:\\nova\\semaine\\%d\\%d.eaf", sess[i], sess[i]);

		ElanDocument *elanDoc = ElanDocument::Read(filepath);

		ElanTier &laughter = (*elanDoc)["Laughter"];
		laughter.unify("laughter");
		ssi_print("TIER %s has %u entries\n", laughter.name(), (ssi_size_t)laughter.size());
		laughter.print();

		ElanTier &transcription = (*elanDoc)["Transcription"];
		ssi_print("TIER %s has %u entries\n", transcription.name(), (ssi_size_t)transcription.size());
		transcription.print();

		ElanTier speech("speech");
		transcription.pack(speech);
		speech.unify("speech");
		ssi_print("TIER %s has %u entries\n", speech.name(), (ssi_size_t)speech.size());
		speech.print();

		Annotation annotation;
		std::map<String, ssi_size_t> classes;
		classes["laugh"] = 0;
		classes["speech"] = 1;
		annotation.setDiscreteScheme("laughter", classes);

		for (ElanTier::iterator segment = laughter.begin(); segment != laughter.end(); segment++)
		{
			annotation.add(segment->from / 1000.0, segment->to / 1000.0, classes["laugh"], 1.0f);
		}
		for (ElanTier::iterator segment = speech.begin(); segment != speech.end(); segment++)
		{
			annotation.add(segment->from / 1000.0, segment->to / 1000.0, classes["speech"], 1.0f);
		}
		annotation.sort();
		annotation.print();

		ssi_sprint(filepath, "X:\\nova\\semaine\\%d\\user.laughter", sess[i]);
		annotation.save(filepath, File::ASCII);

		delete elanDoc;
	}

	return true;
}