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
#include "ssiml.h"
#include "ssimodel.h"
using namespace ssi;

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

void ex_kmeans ();

void PlotSamples (ISamples &samples, const ssi_char_t *name);
void CreateMissingData (SampleList &samples, double prob);

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

	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssisignal.dll");

	ssi_random_seed ();

	Factory::GetPainter ()->MoveConsole(0,600,600,400);

	ex_kmeans ();

	ssi_print ("\n\n\tpress enter to quit\n\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_kmeans () {

	SampleList samples;
	ssi_size_t n_classes = 4;
	ssi_size_t n_sampels = 200;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
	ModelTools::CreateTestSamples (samples, n_classes, n_sampels, n_streams, distr);

	// training
	{
		KMeans *model = ssi_create (KMeans, "kmeans", true);
		model->getOptions()->k = n_classes;
		Trainer trainer (model);
		trainer.train (samples);
		trainer.save ("kmeans");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "kmeans");			
		trainer.cluster (samples);

		PlotSamples (samples, "kmeans");
	} 

	// split
	{
		KMeans *model = ssi_create (KMeans, "kmeans", true);
		model->load("kmeans.trainer.ssi_model_KMeans.model");

		ISSelectSample ss (&samples);
		ss.setSelection (model->getIndicesPerClusterSize(1), model->getIndicesPerCluster(1));
		PlotSamples (ss, "kmeans");
		
	}
}

void CreateMissingData (SampleList &samples, double prob) {

	ssi_size_t n_streams = samples.getStreamSize ();
	ssi_sample_t *sample = 0;
	samples.reset ();
	while (sample = samples.next ()) {
		for (ssi_size_t nstrm = 0; nstrm < n_streams; nstrm++) {
			if (ssi_random () > prob) {							
				ssi_stream_reset (*sample->streams[nstrm]);
			}
		}
	}
	samples.setMissingData (true);
}

void PlotSamples(ISamples &samples, const ssi_char_t *name) {

	ssi_size_t n_classes = samples.getClassSize();
	PaintData **plots = new PaintData *[n_classes];

	for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {

		plots[n_class] = new PaintData;

		ssi_size_t n_samples = samples.getSize(n_class);
		ssi_size_t n_streams = samples.getStreamSize();

		ssi_stream_t stream;
		ssi_stream_init(stream, n_samples * n_streams, 2, sizeof(ssi_real_t), SSI_REAL, 0);

		ssi_real_t *data = ssi_pcast(ssi_real_t, stream.ptr);

		ISSelectClass samples_sel(&samples);
		samples_sel.setSelection(n_class);
		samples_sel.reset();
		ssi_sample_t *sample = 0;
		ssi_real_t *data_ptr = data;
		while (sample = samples_sel.next()) {
			for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
				memcpy(data_ptr, sample->streams[n_stream]->ptr, 2 * sizeof(ssi_real_t));
				data_ptr += 2;
			}
		}

		plots[n_class]->setBackground(n_class == 0, IPainter::ITool::COLORS::BLACK);
		plots[n_class]->setPointSize(10);
		plots[n_class]->setLimits(-0.2f, 1.2f);
		plots[n_class]->setData(stream, PaintData::TYPE::SCATTER);
		plots[n_class]->setBrush(ssi_rgb(COLORS[n_class][0], COLORS[n_class][1], COLORS[n_class][2]));
		plots[n_class]->setPen(IPainter::ITool::COLORS::BLACK);

		delete[] data;
	}

	ssi_print("\n\n\tpress enter to continue\n\n");
	getchar();

	for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
		delete plots[n_class];
	}
	delete[] plots;

}
