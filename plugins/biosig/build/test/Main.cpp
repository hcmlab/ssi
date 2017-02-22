// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
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
#include "ssibiosig.h"
#include "signal/include/ssisignal.h"
#include "OverlapBuffer.h"
using namespace ssi;

// load libraries
#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

bool ex_buffer(void *args);

bool ex_gsr_peaks(void *args);
bool ex_gsr_peaks_online(void *args);
bool ex_gsr_baseline(void *args);
bool ex_gsr_baseline_mean(void *args);
bool ex_gsr_arousal(void *args);
bool ex_gsr_response_amplitude_events(void *args);

bool ex_ecg_detection(void *args);
bool ex_qrs_detection_online(void *args);
bool ex_qrs_pulse_event(void *args);
bool ex_qrs_hrv_event(void *args);

bool ex_qrs_heartrate(void *args);
bool ex_qrs_heartrate_online(void *args);
bool ex_qrs_heartrate_mean(void *args);
bool ex_qrs_hr_features(void *args);

bool ex_bvp_beat_events_raw(void *args);
bool ex_bvp_beat_events_statistical(void *args);

int main(int argc, char* const argv[]){

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("ssiframe.dll");
		Factory::RegisterDLL("ssievent.dll");
		Factory::RegisterDLL("ssibiosig.dll");
		Factory::RegisterDLL("ssiioput.dll");
		Factory::RegisterDLL("ssisignal.dll");
		Factory::RegisterDLL("ssigraphic.dll");

		Exsemble ex;
		ex.console(0, 0, 650, 600);

		ex.add(ex_buffer, 0, "BUFFER", "");

		ex.add(ex_bvp_beat_events_raw, 0, "BVP", "Beat detection");
		ex.add(ex_bvp_beat_events_statistical, 0, "BVP", "Beat events statistics");

		ex.add(ex_gsr_peaks, 0, "GSR", "Peaks");
		ex.add(ex_gsr_peaks_online, 0, "GSR", "Peaks online");
		ex.add(ex_gsr_baseline, 0, "GSR", "Baseline");
		ex.add(ex_gsr_baseline_mean, 0, "GSR", "Baseline mean");
		ex.add(ex_gsr_arousal, 0, "GSR", "Arousal");
		ex.add(ex_gsr_response_amplitude_events, 0, "GSR", "Response amplitude events");

		ex.add(ex_ecg_detection, 0, "ECG", "Beat detection");

		ex.add(ex_qrs_detection_online, 0, "QRS", "Beat detection");
		ex.add(ex_qrs_pulse_event, 0, "QRS", "Pulse");
		ex.add(ex_qrs_hrv_event, 0, "QRS", "HRV");
		ex.add(ex_qrs_heartrate, 0, "QRS", "Heartrate");
		ex.add(ex_qrs_heartrate_online, 0, "QRS", "Heartrate online");
		ex.add(ex_qrs_heartrate_mean, 0, "QRS", "Heartrate mean");
		ex.add(ex_qrs_hr_features, 0, "QRS", "HR features");

		ex.show();

		return true;

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_gsr_response_amplitude_events(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data\\gsr");
	reader->getOptions()->block = 0.2;
	reader->getOptions()->loop = false;

	ITransformable *gsr_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr(lowpassed)");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(gsr_p, plot, "1");

	GSRResponseEventSender *gsr_event = ssi_create(GSRResponseEventSender, 0, true);
	gsr_event->getOptions()->tuple = true;
	gsr_event->getOptions()->minRisingTime = 0;
	gsr_event->getOptions()->minAllowedRegression = 0.1f;
	gsr_event->getOptions()->minAmplitude = 0.5f;
	gsr_event->getOptions()->print = false;

	gsr_event->setLogLevel(SSI_LOG_LEVEL_DEBUG);

	frame->AddConsumer(gsr_p, gsr_event, "0.2s");

	board->RegisterSender(*gsr_event);

	GSREventListener *event_listener = ssi_create(GSREventListener, 0, true);
	event_listener->getOptions()->statisticalFn = GSR_SUM;
	event_listener->getOptions()->window = SSI_GSR_EVENTLISTENER_NO_WINDOW;

	ITransformable *nr_of_responses = frame->AddProvider(event_listener, SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_PROVIDER_NAME);
	ITransformable *amp = frame->AddProvider(event_listener, SSI_GSR_EVENTLISTENER_AMPLITUDE_PROVIDER_NAME);
	ITransformable *power = frame->AddProvider(event_listener, SSI_GSR_EVENTLISTENER_POWER_PROVIDER_NAME);

	frame->AddSensor(event_listener);

	board->RegisterListener(*event_listener, gsr_event->getEventAddress());

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("nr_of_responses");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(nr_of_responses, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("amp");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(amp, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("power");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(power, plot, "1");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, gsr_event->getEventAddress());

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;

}

bool ex_bvp_beat_events_statistical(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data\\bvp");
	reader->getOptions()->block = 0.2;
	reader->getOptions()->loop = false;

	ITransformable *bvp_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("bvp");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(bvp_p, plot, "1");

	BVPBeatEventSender *bvp_event = ssi_create(BVPBeatEventSender, 0, true);
	bvp_event->getOptions()->beep = false;
	bvp_event->getOptions()->mean_window = 1;
	bvp_event->getOptions()->tuple = false;

	bvp_event->setLogLevel(SSI_LOG_LEVEL_DEBUG);

	frame->AddConsumer(bvp_p, bvp_event, "1");

	board->RegisterSender(*bvp_event);

	BVPBeatEventStatisticalListener *event_listener = ssi_create(BVPBeatEventStatisticalListener, 0, true);
	event_listener->getOptions()->sr = 9;
	event_listener->getOptions()->statisticalFn = BVP_MEAN;
	event_listener->getOptions()->window = 10000;

	BVPBeatEventRMSSDListener *event_rmmsd_listener = ssi_create(BVPBeatEventRMSSDListener, 0, true);
	event_rmmsd_listener->getOptions()->sr = 9;
	event_rmmsd_listener->getOptions()->window = 10000;

	ITransformable *heartRate = frame->AddProvider(event_listener, SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_PROVIDER_NAME);
	ITransformable *amp = frame->AddProvider(event_listener, SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_PROVIDER_NAME);
	ITransformable *interBeat = frame->AddProvider(event_listener, SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_PROVIDER_NAME);
	ITransformable *rmmsd = frame->AddProvider(event_rmmsd_listener, SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_PROVIDER_NAME);

	frame->AddSensor(event_listener);
	frame->AddSensor(event_rmmsd_listener);

	board->RegisterListener(*event_listener, bvp_event->getEventAddress());
	board->RegisterListener(*event_rmmsd_listener, bvp_event->getEventAddress());

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("heartRate(mean 60s)");
	plot->getOptions()->size = 60.0;
	frame->AddConsumer(heartRate, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("amp(mean 60s)");
	plot->getOptions()->size = 60.0;
	frame->AddConsumer(amp, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("interBeat(mean 60s)");
	plot->getOptions()->size = 60.0;
	frame->AddConsumer(interBeat, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("RMMSD");
	plot->getOptions()->size = 60.0;
	frame->AddConsumer(rmmsd, plot, "1");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, bvp_event->getEventAddress());

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_bvp_beat_events_raw(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data\\bvp");
	reader->getOptions()->block = 0.2;
	reader->getOptions()->loop = false;

	ITransformable *bvp_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("bvp");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(bvp_p, plot, "1");

	BVPBeatEventSender *bvp_event = ssi_create(BVPBeatEventSender, 0, true);
	bvp_event->getOptions()->beep = true;
	bvp_event->getOptions()->mean_window = 1;
	bvp_event->getOptions()->tuple = true;

	bvp_event->setLogLevel(SSI_LOG_LEVEL_DEBUG);

	frame->AddConsumer(bvp_p, bvp_event, "0.2s");

	board->RegisterSender(*bvp_event);

	BVPBeatEventRawListener *event_listener = ssi_create(BVPBeatEventRawListener, 0, true);
	event_listener->getOptions()->sr = 9;

	ITransformable *heartRate = frame->AddProvider(event_listener, SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_PROVIDER_NAME);
	ITransformable *amp = frame->AddProvider(event_listener, SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_PROVIDER_NAME);
	ITransformable *interBeat = frame->AddProvider(event_listener, SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_PROVIDER_NAME);

	frame->AddSensor(event_listener);

	board->RegisterListener(*event_listener, bvp_event->getEventAddress());

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("heartRate");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(heartRate, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("amp");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(amp, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("interBeat");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(interBeat, plot, "1");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, bvp_event->getEventAddress());

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_buffer(void *args) {

	ssi_size_t n = 20;
	ssi_size_t d = 2;
	ssi_real_t *samples = new ssi_real_t[n*d];
	for (ssi_size_t i = 0; i < n*d; i++) {
		samples[i] = ssi_cast(ssi_real_t, i);
	}

	ssi_size_t n_overlap = 2;
	ssi_size_t n_frame = 4;
	ssi_size_t n_recover = (n / n_frame) * n_frame - n_overlap;
	ssi_real_t *recover = new ssi_real_t[n_recover * d];
	OverlapBuffer ob(n_overlap, d);
	for (ssi_size_t i = 0; i < n / n_frame; i++) {
		ob.push(n_frame, samples + i * n_frame * d);
		for (ssi_size_t j = 0; j < ob.size() - n_overlap; j++) {
			for (ssi_size_t k = 0; k < d; k++) {
				ssi_print("%u %f\n", ob.convertRelativeToAbsoluteSampleIndex(j) * d + k, ob[j * d + k]);
				recover[ob.convertRelativeToAbsoluteSampleIndex(j) * d + k] = ob[j * d + k];
			}
		}
	}

	return true;
}

bool ex_gsr_peaks(void *args) {

	ssi_stream_t gsr, gsr_low;
	FileTools::ReadStreamFile("data\\gsr", gsr);

	GSREventSender *gsr_event = ssi_create(GSREventSender, 0, true);
	gsr_event->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	FileAnnotationWriter awrite("data\\peaks.txt", "peak");
	gsr_event->setEventListener(&awrite);

	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->low = 0.01;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	lowpass->getOptions()->zero = true;
	SignalTools::Transform(gsr, gsr_low, *lowpass, ssi_cast(ssi_size_t, 0.2 * gsr.sr + 0.5));

	FileTools::WriteStreamFile(File::BINARY, "data\\gsr_low", gsr_low);

	SignalTools::Consume(gsr_low, *gsr_event, ssi_cast(ssi_size_t, 4.0 * gsr.sr + 0.5));

	return true;
}

bool ex_gsr_peaks_online(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data\\gsr");
	reader->getOptions()->block = 0.2;
	ITransformable *gsr_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->low = 0.01;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	lowpass->getOptions()->zero = true;
	ITransformable *gsr_low = frame->AddTransformer(gsr_p, lowpass, "0.2s");

	GSREventSender *gsr_event = ssi_create(GSREventSender, 0, true);
	gsr_event->getOptions()->tuple = true;
	gsr_event->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddConsumer(gsr_low, gsr_event, "4.0s");
	board->RegisterSender(*gsr_event);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, gsr_event->getEventAddress());

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(gsr_p, plot, "0.2s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr(lowpassed)");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(gsr_low, plot, "0.2s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("peak");
	plot->getOptions()->reset = true;
	frame->AddEventConsumer(gsr_low, plot, board, "peak@");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("slope");
	plot->getOptions()->reset = true;
	frame->AddEventConsumer(gsr_low, plot, board, "slope@");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("drop");
	plot->getOptions()->reset = true;
	frame->AddEventConsumer(gsr_low, plot, board, "drop@");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_gsr_baseline(void *args) {

	ssi_stream_t gsr_stream_raw;
	ssi_stream_t gsr_stream_lowpass;
	ssi_stream_t gsr_stream_baseline;
	ssi_stream_t gsr_stream_baseline_norm;

	FileWriter *writer = ssi_create(FileWriter, 0, true);

	//raw
	FileTools::ReadStreamFile("data/gsr.stream", gsr_stream_raw);
	ssi_size_t framesize = 0.5 * gsr_stream_raw.sr;

	//lowpass filter
	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->low = 0.01;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	lowpass->getOptions()->zero = true;
	SignalTools::Transform(gsr_stream_raw, gsr_stream_lowpass, *lowpass, framesize);
	writer->getOptions()->setPath("data/gsr_lowpass");
	SignalTools::Consume(gsr_stream_lowpass, *writer, framesize);

	//baseline
	GSRRemoveBaseline *detrend = ssi_create(GSRRemoveBaseline, 0, true);
	SignalTools::Transform(gsr_stream_lowpass, gsr_stream_baseline, *detrend, framesize);
	writer->getOptions()->setPath("data/gsr_detrend");
	SignalTools::Consume(gsr_stream_baseline, *writer, framesize);
	//norm
	MvgNorm *norm = ssi_create(MvgNorm, 0, true);
	norm->getOptions()->norm = MvgNorm::SUBMIN;
	norm->getOptions()->method = MvgNorm::SLIDING;
	norm->getOptions()->win = 15.0;
	norm->getOptions()->rangea = 0.0f;
	norm->getOptions()->rangeb = 1.0f;
	SignalTools::Transform(gsr_stream_baseline, gsr_stream_baseline_norm, *norm, framesize);
	writer->getOptions()->setPath("data/gsr_detrend_norm");
	SignalTools::Consume(gsr_stream_baseline_norm, *writer, framesize);

	return true;
}

bool ex_gsr_baseline_mean(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/gsr");
	reader->getOptions()->block = 0.2;
	ITransformable *gsr_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	//lowpass
	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->low = 0.01;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	lowpass->getOptions()->zero = true;
	ITransformable *gsr_low_t = frame->AddTransformer(gsr_p, lowpass, "0.25s");

	//detrend
	GSRRemoveBaseline *detrend = ssi_create(GSRRemoveBaseline, 0, true);
	ITransformable *gsr_detrend_t = frame->AddTransformer(gsr_low_t, detrend, "0.25s");

	//norm
	MvgNorm *norm = ssi_create(MvgNorm, 0, true);
	norm->getOptions()->norm = MvgNorm::SUBMIN;
	norm->getOptions()->method = MvgNorm::SLIDING;
	norm->getOptions()->win = 15.0;
	ITransformable *gsr_detrend_norm_t = frame->AddTransformer(gsr_detrend_t, norm, "0.25s");

	GSRBaselineMean *mean = ssi_create(GSRBaselineMean, 0, true);
	mean->getOptions()->winsize = 15.0;
	frame->AddConsumer(gsr_detrend_norm_t, mean, "0.25s");
	board->RegisterSender(*mean);

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 100.0f;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr(lowpassed)");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(gsr_low_t, plot, "0.3s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr(detrended)");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(gsr_detrend_t, plot, "0.3s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr(detrended & normed)");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(gsr_detrend_norm_t, plot, "0.3s");

	//events
	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, mean->getEventAddress());

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_ecg_detection(void *args) {

	ssi_stream_t ecg_stream_raw;
	ssi_stream_t ecg_stream_bandpass;
	ssi_stream_t ecg_stream_diff;
	ssi_stream_t ecg_stream_QRSpre;
	ssi_stream_t ecg_stream_low;
	ssi_stream_t ecg_stream_qrs;

	FileWriter *writer_ecg = ssi_create(FileWriter, 0, true);

	//raw
	FileTools::ReadStreamFile("data/ecg.stream", ecg_stream_raw);
	ssi_size_t framesize = 0.75 * ecg_stream_raw.sr;

	//band filter
	Butfilt *ecg_band = ssi_create(Butfilt, 0, true);
	ecg_band->getOptions()->type = Butfilt::BAND;
	ecg_band->getOptions()->norm = false;
	ecg_band->getOptions()->high = 15;
	ecg_band->getOptions()->low = 5;
	ecg_band->getOptions()->order = 13;
	SignalTools::Transform(ecg_stream_raw, ecg_stream_bandpass, *ecg_band, framesize);
	writer_ecg->getOptions()->setPath("data/ecg_bandpass");
	SignalTools::Consume(ecg_stream_bandpass, *writer_ecg, framesize);

	//differentiator
	Derivative *ecg_diff = ssi_create(Derivative, 0, true);
	ssi_strcpy(ecg_diff->getOptions()->names, "1st");
	SignalTools::Transform(ecg_stream_bandpass, ecg_stream_diff, *ecg_diff, framesize);
	writer_ecg->getOptions()->setPath("data/ecg_diff");
	SignalTools::Consume(ecg_stream_diff, *writer_ecg, framesize);

	//PreProcess
	QRSPreProcess *ecg_QRSpre = ssi_create(QRSPreProcess, 0, true);
	SignalTools::Transform(ecg_stream_diff, ecg_stream_QRSpre, *ecg_QRSpre, framesize);
	writer_ecg->getOptions()->setPath("data/ecg_QRSpre");
	SignalTools::Consume(ecg_stream_QRSpre, *writer_ecg, framesize);

	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->zero = true;
	lowpass->getOptions()->norm = false;
	lowpass->getOptions()->low = 6.4;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	SignalTools::Transform(ecg_stream_QRSpre, ecg_stream_low, *lowpass, framesize);
	writer_ecg->getOptions()->setPath("data/ecg_low");
	SignalTools::Consume(ecg_stream_low, *writer_ecg, framesize);

	//qrs detection
	QRSDetect *ecg_qrs = ssi_create(QRSDetect, 0, true);
	SignalTools::Transform(ecg_stream_low, ecg_stream_qrs, *ecg_qrs, framesize);
	writer_ecg->getOptions()->setPath("data/ecg_qrs");
	SignalTools::Consume(ecg_stream_qrs, *writer_ecg, framesize);

	return true;
};

bool ex_qrs_detection_online(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/ecg");
	reader->getOptions()->block = 0.2;
	ITransformable *ecg_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	//bandpass
	Butfilt *ecg_band = ssi_create(Butfilt, 0, true);
	ecg_band->getOptions()->type = Butfilt::BAND;
	ecg_band->getOptions()->norm = false;
	ecg_band->getOptions()->high = 15;
	ecg_band->getOptions()->low = 5;
	ecg_band->getOptions()->order = 13;
	ITransformable *ecg_band_t = frame->AddTransformer(ecg_p, ecg_band, "0.75s");

	//diff
	Derivative *ecg_diff = ssi_create(Derivative, 0, true);
	ssi_strcpy(ecg_diff->getOptions()->names, "1st");
	ITransformable *ecg_diff_t = frame->AddTransformer(ecg_band_t, ecg_diff, "0.75s");

	//qrs-pre-process
	QRSPreProcess *ecg_QRSpre = ssi_create(QRSPreProcess, 0, true);
	ITransformable *ecg_QRSpre_t = frame->AddTransformer(ecg_diff_t, ecg_QRSpre, "0.75s");

	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->zero = true;
	lowpass->getOptions()->norm = false;
	lowpass->getOptions()->low = 6.4;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	ITransformable *ecg_QRSpre_low_t = frame->AddTransformer(ecg_QRSpre_t, lowpass, "0.75s");

	//qrs-detect
	QRSDetect *ecg_qrs = ssi_create(QRSDetect, 0, true);
	ecg_qrs->getOptions()->sendEvent = true;
	ecg_qrs->getOptions()->tuple = true;
	ITransformable *ecg_qrs_t = frame->AddTransformer(ecg_QRSpre_low_t, ecg_qrs, "0.75s");
	board->RegisterSender(*ecg_qrs);

	//qrs-detect-chain
	QRSDetection *ecg_chain = ssi_create(QRSDetection, 0, true);
	ecg_chain->getOptions()->sendEvent = true;
	ITransformable *ecg_qrs_chain_t = frame->AddTransformer(ecg_p, ecg_chain, "0.75s");
	board->RegisterSender(*ecg_chain);

	//pulse
	QRSHeartRate *pulse = ssi_create(QRSHeartRate, 0, true);
	ITransformable *pulse_t = frame->AddTransformer(ecg_qrs_t, pulse, "0.75s");

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 10.0f;
	//raw
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("raw");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_p, plot, "1.0s");
	//bandpass
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("bandpass");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_band_t, plot, "1.0s");
	//diff
	/*plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("diff");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_diff_t, plot, "1.0s");*/
	//qrs-pre
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs-pre");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_QRSpre_low_t, plot, "1.0s");
	//qrs
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_qrs_t, plot, "1.0s");
	////qrs-chain
	//plot = ssi_create_id (SignalPainter, 0, "plot");
	//plot->getOptions()->setTitle("qrs(chain)");
	//plot->getOptions()->size = p_size;		
	//frame->AddConsumer(ecg_qrs_chain_t, plot, "1.0s");
	////pulse
	//plot = ssi_create_id (SignalPainter, 0, "plot");
	//plot->getOptions()->setTitle("pulse");
	//plot->getOptions()->size = p_size;		
	//frame->AddConsumer(pulse_t, plot, "1.0s");

	//events
	EventMonitor *monitor1 = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor1, ecg_qrs->getEventAddress());
	/*EventMonitor *monitor2 = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor2, ecg_chain->getEventAddress());*/

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_qrs_pulse_event(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/ecg");
	reader->getOptions()->block = 0.2;
	ITransformable *ecg_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	//bandpass
	Butfilt *ecg_band = ssi_create(Butfilt, 0, true);
	ecg_band->getOptions()->type = Butfilt::BAND;
	ecg_band->getOptions()->norm = false;
	ecg_band->getOptions()->high = 15;
	ecg_band->getOptions()->low = 5;
	ecg_band->getOptions()->order = 13;
	ITransformable *ecg_band_t = frame->AddTransformer(ecg_p, ecg_band, "0.75s");

	//diff
	Derivative *ecg_diff = ssi_create(Derivative, 0, true);
	ssi_strcpy(ecg_diff->getOptions()->names, "1st");
	ITransformable *ecg_diff_t = frame->AddTransformer(ecg_band_t, ecg_diff, "0.75s");

	//qrs-pre-process
	QRSPreProcess *ecg_QRSpre = ssi_create(QRSPreProcess, 0, true);
	ITransformable *ecg_QRSpre_t = frame->AddTransformer(ecg_diff_t, ecg_QRSpre, "0.75s");

	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->zero = true;
	lowpass->getOptions()->norm = false;
	lowpass->getOptions()->low = 6.4;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	ITransformable *ecg_QRSpre_low_t = frame->AddTransformer(ecg_QRSpre_t, lowpass, "0.75s");

	//qrs-detect
	QRSDetect *ecg_qrs = ssi_create(QRSDetect, 0, true);
	ecg_qrs->getOptions()->sendEvent = true;
	ITransformable *ecg_qrs_t = frame->AddTransformer(ecg_QRSpre_low_t, ecg_qrs, "0.75s");
	board->RegisterSender(*ecg_qrs);

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 10.0f;
	//raw
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("raw");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_p, plot, "0.2s");
	//bandpass
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("bandpass");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_band_t, plot, "0.2s");
	//diff
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("diff");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_diff_t, plot, "0.2s");
	//qrs-pre
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs-pre");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_QRSpre_low_t, plot, "0.2s");
	//qrs
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_qrs_t, plot, "0.2s");

	//events
	QRSPulseEventListener *pulse = ssi_create(QRSPulseEventListener, 0, true);
	pulse->getOptions()->tuple = true;
	board->RegisterListener(*pulse, ecg_qrs->getEventAddress(), 60000);
	board->RegisterSender(*pulse);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, pulse->getEventAddress(), 10000);

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_qrs_hrv_event(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/ecg");
	reader->getOptions()->block = 0.2;
	ITransformable *ecg_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	//bandpass
	Butfilt *ecg_band = ssi_create(Butfilt, 0, true);
	ecg_band->getOptions()->type = Butfilt::BAND;
	ecg_band->getOptions()->norm = false;
	ecg_band->getOptions()->high = 15;
	ecg_band->getOptions()->low = 5;
	ecg_band->getOptions()->order = 13;
	ITransformable *ecg_band_t = frame->AddTransformer(ecg_p, ecg_band, "0.75s");

	//diff
	Derivative *ecg_diff = ssi_create(Derivative, 0, true);
	ssi_strcpy(ecg_diff->getOptions()->names, "1st");
	ITransformable *ecg_diff_t = frame->AddTransformer(ecg_band_t, ecg_diff, "0.75s");

	//qrs-pre-process
	QRSPreProcess *ecg_QRSpre = ssi_create(QRSPreProcess, 0, true);
	ITransformable *ecg_QRSpre_t = frame->AddTransformer(ecg_diff_t, ecg_QRSpre, "0.75s");

	Butfilt *lowpass = ssi_create(Butfilt, 0, true);
	lowpass->getOptions()->zero = true;
	lowpass->getOptions()->norm = false;
	lowpass->getOptions()->low = 6.4;
	lowpass->getOptions()->order = 3;
	lowpass->getOptions()->type = Butfilt::LOW;
	ITransformable *ecg_QRSpre_low_t = frame->AddTransformer(ecg_QRSpre_t, lowpass, "0.75s");

	//qrs-detect
	QRSDetect *ecg_qrs = ssi_create(QRSDetect, 0, true);
	ecg_qrs->getOptions()->sendEvent = true;
	ITransformable *ecg_qrs_t = frame->AddTransformer(ecg_QRSpre_low_t, ecg_qrs, "0.75s");
	board->RegisterSender(*ecg_qrs);

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 10.0f;
	//raw
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("raw");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_p, plot, "0.2s");
	//bandpass
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("bandpass");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_band_t, plot, "0.2s");
	//diff
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("diff");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_diff_t, plot, "0.2s");
	//qrs-pre
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs-pre");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_QRSpre_low_t, plot, "0.2s");
	//qrs
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_qrs_t, plot, "0.2s");

	//events
	QRSHrvEventListener *hrv = ssi_create(QRSHrvEventListener, 0, true);
	hrv->getOptions()->span = 120000;
	hrv->getOptions()->update_ms = 5000;
	board->RegisterListener(*hrv, ecg_qrs->getEventAddress(), 10000);
	board->RegisterSender(*hrv);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, hrv->getEventAddress(), 10000);

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_qrs_heartrate(void *args) {

	if (!ssi_exists("data/ecg_qrs.stream")) {
		ex_ecg_detection(args);
	}

	ssi_stream_t ecg_qrs;
	FileTools::ReadStreamFile("data/ecg_qrs.stream", ecg_qrs);

	ssi_stream_t ecg_hr;
	QRSHeartRate *qrshr = ssi_create(QRSHeartRate, 0, true);
	SignalTools::Transform(ecg_qrs, ecg_hr, *qrshr, 0.25 * ecg_qrs.sr);

	FileTools::WriteStreamFile(File::BINARY, "data/ecg_hr.stream", ecg_hr);

	return true;
}

bool ex_qrs_heartrate_online(void *args) {

	if (!ssi_exists("data/ecg_qrs.stream")) {
		ex_ecg_detection(args);
	}

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/ecg_qrs");
	reader->getOptions()->block = 0.2;
	ITransformable *qrs_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	// heart rate
	QRSHeartRate *qrshr = ssi_create(QRSHeartRate, 0, true);
	ITransformable *ecg_hr_t = frame->AddTransformer(qrs_p, qrshr, "0.25s");

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 100.0f;

	//qrs
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(qrs_p, plot, "0.3s");

	//heart rate
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("pulse");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_hr_t, plot, "0.3s");

	//events
	//EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	//board->RegisterListener(*monitor, ecg_qrs->getEventAddress());

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_qrs_heartrate_mean(void *args) {

	if (!ssi_exists("data/ecg_qrs.stream")) {
		ex_ecg_detection(args);
	}

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/ecg_qrs");
	reader->getOptions()->block = 0.2;
	ITransformable *qrs_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	// heart rate
	QRSHeartRate *qrshr = ssi_create(QRSHeartRate, 0, true);
	ITransformable *ecg_hr_t = frame->AddTransformer(qrs_p, qrshr, "0.25s");

	//heart rate mean
	QRSHeartRateMean *qrshr_mean = ssi_create(QRSHeartRateMean, 0, true);
	qrshr_mean->getOptions()->winsize = 15.0;
	frame->AddConsumer(ecg_hr_t, qrshr_mean, "0.25s");
	board->RegisterSender(*qrshr_mean);

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 100.0f;

	//qrs
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(qrs_p, plot, "0.3s");

	//heart rate
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("heartrate");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_hr_t, plot, "0.3s");

	//events
	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, qrshr_mean->getEventAddress());

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_qrs_hr_features(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//raw
	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data/ecg");
	reader->getOptions()->block = 0.2;
	ITransformable *ecg_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	// qrs detection
	QRSDetection *ecg_chain = ssi_create(QRSDetection, 0, true);
	ecg_chain->getOptions()->sendEvent = false;
	ITransformable *ecg_qrs_t = frame->AddTransformer(ecg_p, ecg_chain, "0.75s", "0", "60.0s");

	// heart rate
	QRSHeartRate *qrshr = ssi_create(QRSHeartRate, 0, true);
	ITransformable *ecg_hr_t = frame->AddTransformer(ecg_qrs_t, qrshr, "1.0s", "0", "60.0s");

	// heart rate features
	Spectrogram *spectogram = ssi_create(Spectrogram, 0, true);
	ssi_strcpy(spectogram->getOptions()->file, "hrspect.banks");
	spectogram->getOptions()->nbanks = 3;
	ITransformable *ecg_hr_spec_t = frame->AddTransformer(ecg_hr_t, spectogram, "1", "29", "60.0s");

	//hrv_spectral
	QRSHRVspectral *ecg_hr_spectralfeatures = ssi_create(QRSHRVspectral, 0, true);
	ecg_hr_spectralfeatures->getOptions()->print = true;
	ITransformable *ecg_hr_spectralfeatures_t = frame->AddTransformer(ecg_hr_spec_t, ecg_hr_spectralfeatures, "1", "0", "60.0s");

	//hrv_time
	QRSHRVtime *ecg_hr_timefeatures = ssi_create(QRSHRVtime, 0, true);
	ecg_hr_timefeatures->getOptions()->print = true;
	ITransformable *ecg_hr_timefeatures_t = frame->AddTransformer(ecg_qrs_t, ecg_hr_timefeatures, "1.0s", "6.5s", "60.0s");

	//visual
	SignalPainter *plot = 0;
	ssi_real_t p_size = 100.0f;

	//qrs
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("qrs");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_qrs_t, plot, "0.75s");

	//heart rate
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("hr");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(ecg_hr_t, plot, "1");

	//spectrgram
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("hr spect");
	plot->getOptions()->size = p_size;
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(ecg_hr_spec_t, plot, "1");

	//framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_gsr_arousal(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FileReader *reader = ssi_create(FileReader, 0, true);
	reader->getOptions()->setPath("data\\gsr");
	reader->getOptions()->block = 0.2;
	ITransformable *gsr_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	GSRArousalEstimation *arousal = ssi_create(GSRArousalEstimation, 0, true);
	ITransformable *arousal_p = frame->AddTransformer(gsr_p, arousal, "1");

	ThresClassEventSender *classifier = ssi_create(ThresClassEventSender, 0, true);
	classifier->getOptions()->setClasses("low,med,high");
	classifier->getOptions()->setThresholds("0.01,0.3,0.7");
	frame->AddConsumer(arousal_p, classifier, "1");
	board->RegisterSender(*classifier);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, "@");

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(gsr_p, plot, "0.2s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("arousal");
	plot->getOptions()->size = 150.0;
	frame->AddConsumer(arousal_p, plot, "0.2s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
