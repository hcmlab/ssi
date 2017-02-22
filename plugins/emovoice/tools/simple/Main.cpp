
#include "ssi.h"
#include "ssiml/include/ssiml.h"
#include "audio/include/ssiaudio.h"
#include "emovoice/include/ssiev.h"
#include "model/include/ssimodel.h"

using namespace ssi;

struct params_s {
	ssi_char_t* sourcedir;
	ssi_char_t **classes_s;
	ssi_char_t **lang_s;
	bool use_praat;
	bool smote;
	bool select_dims;
	ssi_size_t n_dims;
	ssi_size_t *dims;
};

bool VAD_ANNO(params_s &params);
bool SAMPLES(params_s &params);
bool EVAL(params_s &params);

int main(int argc, char **argv) {

	char info[1024];
	ssi_sprint(info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssiaudio");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssiemovoice");
	Factory::RegisterDLL ("ssimodel");

	params_s params;

    params.sourcedir = "./data/lena_corpus_01/user0/";

    params.lang_s = new ssi_char_t*[5];
	params.lang_s[0] = "ger";
	params.lang_s[1] = "ger";
    /*
	params.lang_s[2] = "ger";
	params.lang_s[3] = "ger";
    params.lang_s[4] = "ger";*/

    params.classes_s = new ssi_char_t*[2];
    params.classes_s[0] = "Neutral";
    params.classes_s[1] = "Emotional";
    /*

    params.classes_s = new ssi_char_t*[5];
	params.classes_s[0] = "negative";
	params.classes_s[1] = "neutral";
	params.classes_s[2] = "positive";
	params.classes_s[3] = "aPositive";
	params.classes_s[4] = "aNegative";

*/
	params.smote = true;

	params.use_praat = false; //false == use emovoice

	{

		params.n_dims = 1451;
		params.dims = 0;

	}
	
	//VAD_ANNO (params);
    SAMPLES (params);
    //EVAL (params);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "FINISH - Press Enter");
	getchar();

	Factory::Clear();

	if (params.dims != 0) {
		delete params.dims;
		params.dims = 0;
	}

	return 0;

}

bool VAD_ANNO(params_s &params) {

	// check if parameters are valid
	if (!ssi_exists_dir(params.sourcedir)) {
		ssi_wrn("invalid source directory '%s'", params.sourcedir);
		return false;
	} else {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "APPLY VAD (%s)", params.sourcedir);
	}

	ssi_char_t wavpath[SSI_MAX_CHAR];
	ssi_char_t annopath[SSI_MAX_CHAR];

	AudioActivity *activity = ssi_create(AudioActivity, 0, false);
	activity->getOptions()->threshold = 0.025;

	ZeroEventSender *zsender = ssi_create(ZeroEventSender, 0, false);
	zsender->getOptions()->mindur = 3.0;
	zsender->getOptions()->maxdur = 5.0;
	zsender->getOptions()->hangin = 3;
	zsender->getOptions()->hangout = 10;
    ssi_sprint(wavpath, "%sp1_audio.wav", params.sourcedir);
for(int i=0; i< 2; i++){
		// read data
		ssi_stream_t data;
		WavTools::ReadWavFile(wavpath, data, true);

		// apply activity
		ssi_stream_t vad;
		SignalTools::Transform(data, vad, *activity, ssi_cast(ssi_size_t, 0.03*data.sr + 0.5), ssi_cast(ssi_size_t, 0.015*data.sr + 0.5));

		// find segments and store annotation
        ssi_sprint(annopath, "%sp1.anno", params.sourcedir);

		FileAnnotationWriter writer(annopath, params.classes_s[i]);
		zsender->setEventListener(&writer);
		SignalTools::Consume(vad, *zsender, ssi_cast(ssi_size_t, 0.1*vad.sr + 0.5));

		ssi_stream_destroy(vad);
		ssi_stream_destroy(data);

}

	delete activity;
	delete zsender;

	return true;

}

bool SAMPLES(params_s &params) {

	// check if parameters are valid
	if (!ssi_exists_dir(params.sourcedir)) {
		ssi_wrn("invalid source directory '%s'", params.sourcedir);
		return false;
	} else {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "CREATE SAMPLE LISTS (%s)", params.sourcedir);
	}

	ssi_char_t wavpath[SSI_MAX_CHAR];
	ssi_char_t annopath[SSI_MAX_CHAR];
	ssi_char_t samplepath[SSI_MAX_CHAR];
	ssi_char_t samplepath_full[SSI_MAX_CHAR];

	SampleList samples_full;
    ssi_sprint(samplepath_full, "%ssaudio_in.wav.samples", params.sourcedir);

		EmoVoiceFeat *feat = ssi_create (EmoVoiceFeat, 0, true);

			SampleList samples;
			SampleList samples_temp;

            ssi_sprint(wavpath, "%saudio_in.wav", params.sourcedir);
			if (!ssi_exists(wavpath)) {
				ssi_wrn("invalid wav path '%s'", wavpath);
				return false;
			}

            ssi_sprint(annopath, "%sstimuli.anno", params.sourcedir);
			if (!ssi_exists(annopath)) {
				ssi_wrn("invalid anno path '%s'", annopath);
				return false;
			}

            ssi_sprint(samplepath, "%saudio_in.wav", params.sourcedir);

			// load annotation
			Annotation anno;
			ModelTools::LoadAnnotation(anno, annopath);

			// load raw data
			ssi_stream_t data;
            WavTools::ReadWavFile(wavpath, data, true);

			// create samples
            ModelTools::LoadSampleList(samples_temp, data, anno, "ger");
			ITransformer **transformers = new ITransformer* [1];
			transformers[0] = feat;
			ModelTools::TransformSampleList(samples_temp, samples, 1, transformers);

			// save samples
			ModelTools::SaveSampleList(samples, samplepath, File::ASCII);
			ModelTools::PrintInfo(samples);

	ssi_msg(SSI_LOG_LEVEL_BASIC, "DEBUG");

	// save samples_full
/*
        ssi_sprint(samplepath, "%sp1.samples", params.sourcedir);
		ModelTools::LoadSampleList(samples_full, samplepath);
*/

	if (params.smote) {
		
		//smote
		ISOverSample sover (&samples_full);
		sover.setOver(ISOverSample::SMOTE);
		ModelTools::SaveSampleList(sover, samplepath_full, File::ASCII);
		ModelTools::PrintInfo(sover);
		ModelTools::SaveSampleList(sover, samplepath_full, File::ASCII);
		
	} else {

			ModelTools::SaveSampleList(samples_full, samplepath_full, File::ASCII);

	}
	
	return true;

}

bool EVAL(params_s &params) {

	//check if parameters are valid
	if (!ssi_exists_dir(params.sourcedir)) {
		ssi_wrn("invalid source directory '%s'", params.sourcedir);
		return false;
	} else {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "EVALUATE SAMPLES IN (%s)", params.sourcedir);
	}
    ssi_char_t model_path[SSI_MAX_CHAR];
    ssi_char_t trainer_path[SSI_MAX_CHAR];
	ssi_char_t samplepath_full[SSI_MAX_CHAR];
	SampleList samples_full;
    ssi_sprint(samplepath_full, "%saudio_in.wav.samples", params.sourcedir);

	ModelTools::LoadSampleList(samples_full, samplepath_full);

    ModelTools::PrintInfo(samples_full);
    IModel *model = ssi_create (IModel, 0, true);
	Evaluation eval;

	//full feature set
	Trainer trainer_full (model, 0);
    eval.evalKFold(&trainer_full, samples_full, 5);
	eval.print();

	//floating search
/*
    Trainer trainer_sel (model, 0);

    FloatingSearch *fsearch = ssi_create (FloatingSearch, 0, true);

        fsearch->getOptions()->nfirst = 100;

    trainer_sel.setSelection (samples_full, fsearch);
    //
	const ISelection::score *scores;
    //
    scores = fsearch->getScores();
	ssi_print("\n\nFeature Scores:\n");
    for (ssi_size_t i = 0; i < 50; i++) {
		ssi_print("\nIndex: %d\tValue: %.2f", scores[i].index, scores[i].value);
	}ssi_print("\n\n");

    eval.evalKFold(&trainer_sel, samples_full, 2);
    eval.print();
*/

    //
    ssi_sprint(model_path, "%sSVM2.model", params.sourcedir);
    ssi_sprint(trainer_path, "%sSVM2.trainer", params.sourcedir);
    model->save(model_path);
    trainer_full.save(trainer_path);
	return true;

}
