// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/06
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "PaintBackground.h"
#include "PaintRandomLines.h"
#include "PaintRandomPoints.h"
#include "PaintRandomShapes.h"
#include "PaintSomeText.h"
#include "MyCanvasClient.h"
#include "ssi.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_window(void *arg);
bool ex_textbox(void *arg);
bool ex_checkbox(void *arg);
bool ex_combobox(void *arg);
bool ex_slider (void *arg);
bool ex_canvas(void *arg);
bool ex_canvas2(void *arg);
bool ex_stream(void *arg);
bool ex_pipeline_1(void *arg);
bool ex_pipeline_2(void *arg);
bool ex_monitor(void *arg);
bool ex_button(void *arg);
bool ex_grid(void *arg);
bool ex_tab(void *arg);
bool ex_tray(void *arg);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssimouse");
	Factory::RegisterDLL ("ssigraphic");

	ssi_random_seed();

	Exsemble ex;	
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	ex.add(&ex_window, 0, "WINDOW", "How to create a window.");
	ex.add(&ex_canvas, 0, "CANVAS-1", "How to use 'Canvas' to draw some random shapes.");	
	ex.add(&ex_canvas2, 0, "CANVAS-2", "How to use 'Canvas' to draw streams and events.");
	ex.add(&ex_stream, 0, "STREAMS", "How to use 'GraphicTools' to draw streams.");
	ex.add(&ex_pipeline_1, 0, "PIPELINE-1", "How to use 'SignalPainter' and 'VideoPainter' to draw the streams in a pipeline.");
	ex.add(&ex_pipeline_2, 0, "PIPELINE-2", "How to use 'EventPainter' to visualize events in a pipeline.");
	ex.add(&ex_monitor, 0, "MONITOR", "How to use 'Monitor' to output text messages.");

#ifndef SSI_USE_SDL

	ex.add(&ex_slider, 0, "SLIDER", "How to use 'Slider' class.");
	ex.add(&ex_combobox, 0, "COMBOBOX", "How to use 'ComboBox' class.");
	ex.add(&ex_checkbox, 0, "CHECKBOX", "How to use 'CheckBox' class.");
	ex.add(&ex_textbox, 0, "TEXTBOX", "How to use 'TextBox' class.");
	ex.add(&ex_button, 0, "BUTTON", "How to use 'Button' class.");
	ex.add(&ex_grid, 0, "GRID", "How to use 'Grid' class.");
	ex.add(&ex_tab, 0, "TAB", "How to use 'Tab' class.");
	ex.add(&ex_tray, 0, "SYSTEM TRAY", "How to use 'SystemTray' class.");

#endif

	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_window(void *arg) {

    ssi::Window window[4];
	ssi_char_t string[SSI_MAX_CHAR];

	for (int i = 0; i < 4; i++)
	{		
		ssi_sprint(string, "#%d", i);
		window[i].setTitle(string);
		window[i].setPosition(ssi_rect(CONSOLE_WIDTH, i * (CONSOLE_HEIGHT / 4), 400, CONSOLE_HEIGHT / 4));
		window[i].setIcons(IWindow::ICONS::CLOSE);
		window[i].create();
		window[i].show();
	}
		
	ssi_print("\n\n\tpress enter to continue\n");
	getchar();
		
	for (int i = 0; i < 4; i++)
	{
		window[i].close();
	}

	return true;
}

bool ex_canvas(void *arg) {

	PaintBackground background;
	PaintRandomPoints points(1000);
	PaintRandomLines lines(30);
	PaintRandomShapes shapes(10);
	PaintSomeText text(5, 0, "Hello World");

	Canvas canvas[5];
	canvas[0].addClient(&background);
	canvas[0].addClient(&points); 	
	canvas[1].addClient(&background);
	canvas[1].addClient(&lines);	
	canvas[2].addClient(&background);
	canvas[2].addClient(&shapes);	
	canvas[3].addClient(&background);
	canvas[3].addClient(&text);
	canvas[4].addClient(&background);
	canvas[4].addClient(&points);
	canvas[4].addClient(&lines);
	canvas[4].addClient(&shapes);
	canvas[4].addClient(&text);

	ssi::Window window[5];	
	ssi_char_t *titles[] = { "points", "lines", "shapes", "text", "all" };
    for (ssi_size_t i = 0; i < 5; i++) {
		window[i].setClient(&canvas[i]);
		window[i].setPosition(ssi_rect(CONSOLE_WIDTH, i * 200, 400, 200));
		window[i].setTitle(titles[i]);
		window[i].setIcons(IWindow::ICONS::MINIMIZE | IWindow::ICONS::MAXIMIZE);
		window[i].create();
		window[i].show();
	}

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

    for (ssi_size_t i = 0; i < 5; i++) {
		window[i].close();
	}

	return true;
}

bool ex_canvas2(void *arg) {

	ssi_stream_t data;
	
	FileTools::ReadStreamFile("eye", data);

    ssi::Window window[5];
	Canvas canvas[5];
	
	PaintData paint_signal;	
	paint_signal.setData (data, PaintData::TYPE::SIGNAL);
	canvas[0].addClient(&paint_signal);
	window[0].setClient(&canvas[0]);
	window[0].setTitle("SIGNAL");
		
	PaintData paint_path;
	paint_path.setData (data, PaintData::TYPE::PATH);
	canvas[1].addClient(&paint_path);
	window[1].setClient(&canvas[1]);
	window[1].setTitle("PATH");

	PaintData paint_scatter;
	paint_scatter.setData(data, PaintData::TYPE::SCATTER);
	paint_scatter.setLabel("label");	
	canvas[2].addClient(&paint_scatter);
	window[2].setClient(&canvas[2]);
	window[2].setTitle("SCATTER");

	ssi_stream_destroy(data);
	
	FileTools::ReadStreamFile("audio", data);
	PaintData paint_audio;
	paint_audio.setData(data, PaintData::TYPE::AUDIO);
	canvas[3].addClient(&paint_audio);
	window[3].setClient(&canvas[3]);
	window[3].setTitle("AUDIO");

	ssi_stream_destroy(data);
	
	FileTools::ReadStreamFile("image", data);
	PaintData imagePlot;
	imagePlot.setData(data, PaintData::TYPE::IMAGE);	
	ssi_stream_t colormap;
	FileTools::ReadStreamFile("colormap", colormap);
	canvas[4].addClient(&imagePlot);
	window[4].setClient(&canvas[4]);
	window[4].setTitle("IMAGE");

	ssi_stream_destroy(colormap);
	ssi_stream_destroy(data);
	
	for (ssi_size_t i = 0; i < 5; i++) {
		window[i].setPosition(ssi_rect(CONSOLE_WIDTH, i * 200, 400, 200));
		window[i].create();
		window[i].show();
	}

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	for (ssi_size_t i = 0; i < 5; i++) {
		window[i].close();		
	}

	return true;
}

bool ex_stream (void *arg) {

    Canvas canvas[4];
	MyCanvasClient *client[4];
	ssi::Window window[4];

	{
		ssi_stream_t signal;
		ssi_stream_init(signal, 0, 3, sizeof(ssi_real_t), SSI_REAL, 100.0);
		SignalTools::Series(signal, 3.0);
		ssi_time_t freqs[] = { 1.0, 5.0, 10.0 };
		ssi_real_t amps[] = { 0.5f, 1.0f, 2.0f };
		SignalTools::Sine(signal, freqs, amps);

		ssi_norm(signal.num, signal.dim, ssi_pcast(ssi_real_t, signal.ptr));
		
		client[0] = new MyCanvasClient(signal, MyCanvasClient::TYPE::SIGNAL);

		ssi_stream_destroy(signal);
	}

	{
		ssi_stream_t signal;
		ssi_stream_init(signal, 200, 2, sizeof(ssi_real_t), SSI_REAL, 100.0);
		SignalTools::Random(signal);

		client[1] = new MyCanvasClient(signal, MyCanvasClient::TYPE::PATH);
		client[2] = new MyCanvasClient(signal, MyCanvasClient::TYPE::SCATTER);
		
		ssi_stream_destroy(signal);
	}

	{
		ssi_stream_t signal;
		ssi_stream_init(signal, 200, 200, sizeof(ssi_real_t), SSI_REAL, 100.0);
		SignalTools::Random(signal);

		client[3] = new MyCanvasClient(signal, MyCanvasClient::TYPE::IMAGE);

		ssi_stream_destroy(signal);
	}

	ssi_rect_t rect;
	rect.width = 400;
	rect.height = 200;
	rect.left = CONSOLE_WIDTH;
	rect.top = 0;
	for (ssi_size_t i = 0; i < 4; i++) {

		canvas[i].addClient(client[i]);
		window[i].setPosition(rect);
		window[i].setClient(&canvas[i]);
		window[i].create();
		window[i].show();

		rect.top += rect.height;
	}

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	for (ssi_size_t i = 0; i < 4; i++) {
		window[i].close();
		delete client[i];
	}

	return true;
}

bool ex_pipeline_1 (void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	decorator->getOptions()->setScale(1.0f, 0.5f);
	frame->AddDecorator(decorator);

	// sensor
	Mouse *mouse = ssi_create (Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;	
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);
	
	// trigger
	ZeroEventSender *zero_trigger = ssi_create (ZeroEventSender, 0, true);	 
	zero_trigger->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, zero_trigger, "0.2s");
	board->RegisterSender(*zero_trigger);

	// faked signal
	FakeSignal *sine = ssi_create(FakeSignal, 0, true);
	sine->getOptions()->type = FakeSignal::SIGNAL::SINE;
	ITransformable *sine_p = frame->AddProvider(sine, "fake");
	frame->AddSensor(sine);

	FakeSignal *random = ssi_create(FakeSignal, 0, true);
	random->getOptions()->type = FakeSignal::SIGNAL::RANDOM;
	ITransformable *random_p = frame->AddProvider(random, "random");
    frame->AddSensor(random);

	FakeSignal *video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_SSI;
	ITransformable *video_p = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	Selector *selector = 0;
	
	selector = ssi_create(Selector, 0, true);
	ssi_size_t n_select = 16;
	ssi_size_t select[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 };
	ssi_random_shuffle(n_select, select);
	selector->getOptions()->set(n_select, select);
	ITransformable *select_t = frame->AddTransformer(cursor_p, selector, "1");

	selector = ssi_create(Selector, 0, true);
	ssi_size_t n_select_multiples = 2;
	ssi_size_t select_multiples[] = { 0, 2 };	
	selector->getOptions()->set(n_select_multiples, select_multiples);
	selector->getOptions()->multiples = 4;
	ITransformable *select_multiples_t = frame->AddTransformer(select_t, selector, "1");

	selector = ssi_create(Selector, 0, true);
	ssi_size_t select_rectangle[] = { 0, 1, 0, 0, 1, 0, 1, 1 };
	selector->getOptions()->set(8, select_rectangle);
	ITransformable *select_rectangle_t = frame->AddTransformer(cursor_p, selector, "1");

	// plot
    SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR");
	plot->getOptions()->size = 2.0;	
	ITransformable *pp[2] = { cursor_p, button_p };
	frame->AddConsumer(2, pp, plot, "0.1s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR(trigger)");
	plot->getOptions()->type = PaintSignalType::PATH;
	frame->AddEventConsumer(cursor_p, plot, board, zero_trigger->getEventAddress());

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("SINE");
	plot->getOptions()->size = 20.0;
	frame->AddConsumer(sine_p, plot, "0.1s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("IMAGE");
	plot->getOptions()->size = 2.0;
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(select_t, plot, "0.1s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("IMAGE(static)");
	plot->getOptions()->type = PaintSignalType::IMAGE;
	plot->getOptions()->staticImage = true;
	plot->getOptions()->indx = 2;
	plot->getOptions()->indy = 4;
	frame->AddConsumer(select_multiples_t, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("BARS");
	plot->getOptions()->type = PaintSignalType::BAR_POS;
	plot->getOptions()->autoscale = true;
	frame->AddConsumer(select_multiples_t, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("BARS(fix)");
	plot->getOptions()->type = PaintSignalType::BAR_POS;		
	plot->getOptions()->fix[0] = 1.0f;
	plot->getOptions()->autoscale = false;
	frame->AddConsumer(select_multiples_t, plot, "1");


	PointsPainter *pplot = 0;
	
    pplot = ssi_create_id(PointsPainter, 0, "plot");
    pplot->getOptions()->setTitle("LINES");
    pplot->getOptions()->relative = true;
    pplot->getOptions()->swap = true;
    pplot->getOptions()->type = PaintPointsType::LINES;
    frame->AddConsumer(select_rectangle_t, pplot, "1");

    pplot = ssi_create_id(PointsPainter, 0, "plot");
    pplot->getOptions()->setTitle("POINTS");
    pplot->getOptions()->relative = true;
    pplot->getOptions()->swap = true;
    pplot->getOptions()->type = PaintPointsType::DOTS;
    frame->AddConsumer(select_rectangle_t, pplot, "1");

	decorator->add("plot*", 0, 0, 400, 800);

	VideoPainter *vplot = 0;
	
	vplot = ssi_create_id(VideoPainter, 0, "video");
	vplot->getOptions()->setTitle("VIDEO");
	vplot->getOptions()->type = PaintPointsType::LINES;
	vplot->getOptions()->relative = true;
	vplot->getOptions()->swap = true;
	vplot->getOptions()->scale = false;
	ITransformable *vplot_inds[] = { video_p, select_rectangle_t };
	frame->AddConsumer(2, vplot_inds, vplot, "1");

	vplot = ssi_create_id(VideoPainter, 0, "video");
	vplot->getOptions()->setTitle("VIDEO");
	vplot->getOptions()->type = PaintPointsType::DOTS;
	vplot->getOptions()->relative = true;
	vplot->getOptions()->swap = true;
	frame->AddConsumer(2, vplot_inds, vplot, "1");

	decorator->add("video*", 400, 0, 400, 800);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_pipeline_2(void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	// sensor
	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// trigger
	ZeroEventSender *zero_trigger = ssi_create(ZeroEventSender, 0, true);
	zero_trigger->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, zero_trigger, "0.2s");
	board->RegisterSender(*zero_trigger);

	MapEventSender *map_sender = ssi_create(MapEventSender, 0, true);
	map_sender->getOptions()->setAddress("cursor@mouse");
	map_sender->getOptions()->setKeys("x,y");
	frame->AddConsumer(cursor_p, map_sender, "0.2s");
	board->RegisterSender(*map_sender);

	EventPainter *eplot = 0;
	
	eplot = ssi_create_id(EventPainter, 0, "plot");
	eplot->getOptions()->setTitle("events");
	eplot->getOptions()->type = PaintBars::TYPE::BAR_POS;
	board->RegisterListener(*eplot, map_sender->getEventAddress());

	eplot = ssi_create_id(EventPainter, 0, "plot");
	eplot->getOptions()->setTitle("events(global limit)");
	eplot->getOptions()->type = PaintBars::TYPE::BAR_POS;
	eplot->getOptions()->global = true;	
	board->RegisterListener(*eplot, map_sender->getEventAddress());

	eplot = ssi_create_id(EventPainter, 0, "plot");
	eplot->getOptions()->setTitle("events(reset limit)");
	eplot->getOptions()->type = PaintBars::TYPE::BAR_POS;
	eplot->getOptions()->global = true;
	eplot->getOptions()->reset = true;	
	board->RegisterListener(*eplot, map_sender->getEventAddress());

	eplot = ssi_create_id(EventPainter, 0, "plot");
	eplot->getOptions()->setTitle("events(fixed limit)");
	eplot->getOptions()->type = PaintBars::TYPE::BAR;
	eplot->getOptions()->autoscale = false;
	eplot->getOptions()->reset = false;
	eplot->getOptions()->global = true;
	eplot->getOptions()->fix = 0.1f;
	eplot->getOptions()->setBarName("line1|line2|line3,line1|line2|line3");
	board->RegisterListener(*eplot, map_sender->getEventAddress());

	decorator->add("plot*", 1, 4, CONSOLE_WIDTH, 0, 400, 800);

	board->Start();
	frame->Start();		
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_monitor(void *arg) {

    //needed on Linux for windowmanaging
    //alternativly use sdlWindowManager directly (not recommended)
    Decorator *decorator = ssi_create(Decorator, 0, true);
	
    Monitor monitor;
	monitor.setFont(SSI_DEFAULT_FONT_NAME, SSI_DEFAULT_FONT_SIZE);

    ssi::Window window;
    window.setTitle("monitor");
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 400, 400));
    window.setClient(&monitor);
    window.create();
    window.show();

	monitor.print("Hello World!\r\nNew Line");
	monitor.update();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	monitor.clear();
	monitor.setFont(0, 20);

	monitor.print("Good bye!");
	monitor.update();
	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

#ifndef SSI_USE_SDL

class SliderCallback : public Slider::ICallback {
public:
	void update(ssi_real_t value) {
		ssi_print("value: %.2f\n", value);
	}
};

bool ex_slider(void *arg) {

	Slider slider("slider", 0, -20, 20, 40);
	SliderCallback callback;
	slider.setCallback(&callback);

	Window window;
	window.setClient(&slider);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 200, 75));
	window.create();
	window.show();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	slider.set(10.0f);
	printf("mew slider value: %.2f\n", slider.get());

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

class ComboBoxCallback : public ComboBox::ICallback {
public:
	void update(const ssi_char_t *item) {
		ssi_print("item: %s\n", item);
	}
};

bool ex_combobox(void *arg) {

	ssi_size_t n_items = 5;
	const ssi_char_t *items[] = {
		{ "Item A" },
		{ "Item B" },
		{ "Item C" },
		{ "Item D" },
		{ "Item E" }
	};

	ComboBox combo(n_items, items, 2);
	ComboBoxCallback callback;
	combo.setCallback(&callback);

	Window window;
	window.setClient(&combo);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 200, 200));
	window.create();
	window.show();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	combo.set(1);
	printf("new combobox value: %s\n", combo.get());

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

class CheckBoxCallback : public CheckBox::ICallback {
public:
	void update(bool checked) {
		ssi_print("%s\n", checked ? "checked" : "unchecked");
	}
};

bool ex_checkbox(void *arg) {

	CheckBox check("checkbox", true);
	CheckBoxCallback callback;
	check.setCallback(&callback);

	Window window;
	window.setClient(&check);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 200, 75));
	window.create();
	window.show();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	check.set(!check.get());
	printf("new combobox value: %s\n", check.get() ? "true" : "false");

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();	

	return true;
}

class TextBoxCallback : public TextBox::ICallback {
public:
	void update(const ssi_char_t *text) {
		ssi_print("update: %s\n", text);
	}
};

bool ex_textbox(void *arg) {

	TextBox text("some text", false, 256);
	TextBoxCallback callback;
	text.setCallback(&callback);

	Window window;
	window.setClient(&text);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 200, 75));
	window.create();
	window.show();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	text.set("some new text");
	printf("new text: %s\n", text.get());

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

class ButtonCallback : public Button::ICallback {
public:
	void update() {
		ssi_print("update\n");
	}
};

bool ex_button(void *arg) {

	Button button("button");
	ButtonCallback callback;
	button.setCallback(&callback);

	Window window;
	window.setClient(&button);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 200, 75));
	window.create();
	window.show();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

class GridCallback : public Grid::ICallback {
public:
	void update(ssi_size_t row, ssi_size_t col, const ssi_char_t *text) {
		ssi_print("[%u,%u] %s\n", row, col, text);
	}
	void update(ssi_size_t row, ssi_size_t col, bool checked) {
		ssi_print("[%u,%u] %s\n", row, col, checked ? "true" : "false");
	}
};

bool ex_grid(void *arg) {

	Grid grid("grid");
	GridCallback callback;
	grid.setCallback(&callback);	

	Window window;
	window.setClient(&grid);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 400, 400));
	window.create();
	window.show();	

	grid.setEditable(true);		
	grid.setShowHilight(false);
	grid.setColumnWidth(3, 20);	
	grid.setHeaderRowHeight(50);
	grid.setAllowColumnResize(true);
	grid.setExtendLastColumn(false);
	grid.setColumnsAutoWidth(true);
	grid.setColumnsNumbered(false);
	grid.setProtectColor (ssi_rgb(255, 0, 0));	
	grid.setGridDim(10, 4);

	grid.setText(0, 1, "Protected");
	grid.setText(1, 1, "hello world", false);

	grid.setText(0, 2, "Editable");
	grid.setText(1, 2, "change me", true);

	grid.setText(0, 3, "Checkbox");
	grid.setCheckBox(1, 3, true);
	grid.setCheckBox(2, 3, true, false);

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

class TabCallback : public Tab::ICallback {
public:
	void update(ssi_size_t index) {
		ssi_print("selected tab '%u'\n", index);
	}
};

bool ex_tab(void *arg) {

	Tab tab("tab");
	TabCallback callback;
	tab.setCallback(&callback);

	Button button("Button");	
	ButtonCallback buttonCallback;
	button.setCallback(&buttonCallback);
	tab.addClient("Button", &button);

	CheckBox checkBox("CheckBox", false);
	CheckBoxCallback checkBoxCallback;
	checkBox.setCallback(&checkBoxCallback);
	tab.addClient("CheckBox", &checkBox);

	TextBox textBox("", false, 512);
	TextBoxCallback textBoxCallback;
	textBox.setCallback(&textBoxCallback);
	tab.addClient("TextBox", &textBox);

	Grid grid("Grid");
	GridCallback gridCallback;
	grid.setCallback(&gridCallback);
	tab.addClient("Grid", &grid);

	Window window;
	window.setClient(&tab);
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 400, 400));
	window.create();
	window.show();

	grid.setEditable(true);
	grid.setGridDim(2, 2);

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

class MyShowAndHideCallback : public Window::ShowAndHideCallback{

public:

	MyShowAndHideCallback() {
		_console = ssi_pcast(Console, Factory::GetObjectFromId(SSI_FACTORY_CONSOLE_ID));
		_isVisible = true;
	}

	bool isVisible() {		
		return _isVisible;
	}
	void show(Window *w) {
		_console->show();
		_isVisible = true;
	}
	void hide(Window *w) {
		_console->hide();
		_isVisible = false;
	}
	bool isMinMaxVisible() {
		return true;
	}
	void minmax_show(Window *w) {
	}
	void minmax_hide(Window *w) {
	}

protected:

	Console *_console;
	bool _isVisible;
};

bool ex_tray(void *arg) {

	MyShowAndHideCallback callback;

	Window window;
	window.setPosition(ssi_rect(CONSOLE_WIDTH, 0, 400, 400));
	window.setIcons(IWindow::ICONS::SYSTEMTRAY);
	window.setCallback(&callback);
	window.create();

	ssi_print("\n\n\tpress enter to continue\n");
	getchar();

	window.close();

	return true;
}

#endif

