// MainFile.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/07/21
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
#include "MyOscListener.h"
using namespace ssi;

#include "curl/ssicurl.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_file(void *arg);
bool ex_lz4(void *arg);
bool ex_filepath(void *arg);
bool ex_memory(void *arg);
bool ex_csv(void *arg);
bool ex_writer(void *arg);
bool ex_simulator(void *arg);
bool ex_stream(void *arg);
bool ex_event(void *arg);
bool ex_samples(void *arg);
bool ex_annotation(void *arg);
bool ex_socket(void *arg);
bool ex_tcp2way(void *arg);
bool ex_sender(void *arg);
bool ex_sender_events(void *arg);
bool ex_sender_video(void *arg);
bool ex_sender_file(void *arg);
bool ex_download_file(void *arg);

void test (File &file, int *data_out, ssi_size_t size);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssimouse");
	Factory::RegisterDLL ("ssigraphic");

#if SSI_RANDOM_LEGACY_FLAG	
	ssi_random_seed();
#endif

	Socket::TYPE type_udp = Socket::UDP; 
	Socket::TYPE type_tcp = Socket::TCP;	

	Exsemble ex;
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	ex.add(&ex_filepath, 0, "FILEPATH", "How to use 'FilePath' to decompose a filepath.");
	ex.add(&ex_file, 0, "FILE", "How to use 'File' to write to an ascii/binary file.");
	ex.add(&ex_lz4, 0, "FILE-LZ4", "How to use 'File' to write to a compressed file.");
	ex.add(&ex_memory, 0, "MEMORY", "How to use 'FileMem' to write into memory.");
	ex.add(&ex_writer, 0, "WRITER", "How to use 'FileWriter' to store a stream to a file from a pipeline.");
	ex.add(&ex_stream, 0, "STREAM", "How to write/read a stream to a file.");
	ex.add(&ex_event, 0, "EVENT", "How to write/read events to a file.");
	ex.add(&ex_csv, 0, "CSV", "How to read a comma separated file.");
	ex.add(&ex_simulator, 0, "SIMULATOR", "How to use 'FileReader' to feed a stream from a file into a pipeline.");
	ex.add(&ex_annotation, 0, "ANNOTATION", "How to use 'FileAnnotationWriter' to create annotations from events.");
	ex.add(&ex_samples, 0, "SAMPLES", "How to use 'FileSampleWriter' to store samples to a file from a pipeline.");
	ex.add(&ex_socket, 0, "BASICS", "How to send/receive socket messages.");
	ex.add(&ex_tcp2way, 0, "TCP2WAY", "How to establish a two way tcp connection.");
	ex.add(&ex_sender, &type_udp, "STREAM UDP", "How to stream a signal from a pipeline using UDP.");
	ex.add(&ex_sender_events, &type_udp, "SEND EVENTS UDP", "How to send events from a pipeline using UDP.");
	ex.add(&ex_sender, &type_tcp, "STREAM TCP", "How to stream a signal from a pipeline using TCP.");
	ex.add(&ex_sender_events, &type_tcp, "SEND EVENTS TCP", "How to send events from a pipeline using TCP.");
	ex.add(&ex_sender_video, 0, "STREAM VIDEO", "How to stream a video from a pipeline.");
	//ex.add(&ex_sender_file, 0, "FILE", "How to transfer the content of a file.");
	ex.add(&ex_download_file, 0, "DOWNLOAD FILE", "How to donwload a file.");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_filepath (void *arg) {

	{
		ssi_char_t *filepath = "C:/mydir/subdir/myfile.ext";
		FilePath fp (filepath);
		fp.print ();
	}
	{
		ssi_char_t *filepath = "C:/mydir/subdir/";
		FilePath fp (filepath);
		fp.print ();
	}
	{
		ssi_char_t *filepath = "myfile.ext";
		FilePath fp (filepath);
		fp.print ();
	}
	{
		ssi_char_t *filepath = "myfile";
		FilePath fp (filepath);
		fp.print ();
	}
	{
		ssi_char_t *filepath = "C:/.myfile";
		FilePath fp (filepath);
		fp.print ();
	}

	return true;
}

bool ex_file(void *arg) {

	File::SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	{
		char string[512];

		char *longfilepath = "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong/longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong";		
		
		File *file = 0;		
		ssi_char_t *filename = 0;
		
		filename = ssi_strcat(longfilepath, "/hello.txt");
		file = File::CreateAndOpen(File::ASCII, File::WRITE, filename);
		file->writeLine("hello world!");		
		delete file;
		file = File::CreateAndOpen(File::ASCII, File::READ, filename);
		file->readLine(512, string);
		printf("%s\n", string);
		delete file;
		delete[] filename;

		filename = ssi_strcat(longfilepath, "/hello.bin");
		file = File::CreateAndOpen(File::BINARY, File::WRITE, filename);
		file->writeLine("hello world!");
		delete file;
		file = File::CreateAndOpen(File::BINARY, File::READ, filename);
		file->readLine(512, string);
		printf("%s\n", string);
		delete file;
		delete[] filename;
	}

	int data[] = {1,2,3,4,5,6,7,8,9,10};

	{
		ssi_char_t filename[] = "out.txt";
		File *file = File::CreateAndOpen (File::ASCII, File::WRITEPLUS, filename);
		file->setType(SSI_INT);
		file->setFormat(", ", "010");
		test (*file, data, 10);
		ssi_pcast (FileAscii, file)->show();
		delete file;
	}

	{
		ssi_char_t filename[] = "out.bin";
		File *file = File::CreateAndOpen (File::BINARY, File::WRITEPLUS, filename);
		file->setType(SSI_INT);
		file->setFormat(", ", "010");
		test (*file, data, 10);
		delete file;
	}

	return true;
}

bool ex_lz4(void *arg) {

	ssi_real_t data[10000];
	for (ssi_size_t i = 0; i < 10000; i++)
	{
		data[i] = i % 100 ? 0.0f : 1.0f;
	}
	ssi_size_t bytes_compressed = 0;

	{
		ssi_char_t filename[] = "raw.bin";
		File *file = File::CreateAndOpen(File::BINARY, File::WRITE, filename);		
		ssi_size_t bytes = file->write(data, sizeof(ssi_real_t), 10000);
		ssi_print("raw bytes written: %u == %I64d bytes\n", bytes, file->tell());
		delete file;
	}

	{
		ssi_char_t filename[] = "lz4.bin";
		File *file = File::CreateAndOpen(File::BIN_LZ4, File::WRITE, filename);	
		bytes_compressed = file->write(data, sizeof(ssi_real_t), 10000);
		ssi_print("lz4 bytes written: %u == %I64d bytes\n", bytes_compressed, file->tell());
		delete file;
	}

	{
		ssi_char_t filename[] = "raw.bin";
		File *file = File::CreateAndOpen(File::BINARY, File::READ, filename);
		ssi_size_t bytes = file->read(data, sizeof(ssi_real_t), 10000);		
		ssi_print("raw bytes read: %u == %u bytes\n", bytes, (ssi_size_t) (10000 * sizeof(ssi_real_t)));
		delete file;
	}

	{
		ssi_char_t filename[] = "raw.bin";
		File *file = File::CreateAndOpen(File::BIN_LZ4, File::READ, filename);
		ssi_size_t bytes = file->read(data, 10000 * sizeof(ssi_real_t), bytes_compressed);
		ssi_print("raw bytes read: %u == %u bytes\n", bytes, (ssi_size_t) (10000 * sizeof(ssi_real_t)));
		delete file;
	}

	return true;
}

bool ex_memory(void *arg) {

	FileMem *mem = FileMem::Create (FileMem::ASCII);

	int arr[10];
	for (int i = 0; i < 10; i++) {
		arr[i] = i;
	}

	ssi_size_t n_string = 50;
	ssi_char_t *string = new ssi_char_t[n_string];
	mem->set(n_string, string, true);
	mem->setType(SSI_INT);

	mem->writeLine("");
	mem->writeLine("array=[");
	mem->write(arr, 2, 10);
	mem->writeLine("]");
	printf ("%s (%u characters)\n", (ssi_char_t*)mem->getMemory(), mem->getPosition());

	mem->set(ssi_strlen(string) + 1, string, false);

	mem->readLine(n_string, string);
	printf ("%s", string);
	mem->read(arr, 2, 10);
	for (int i = 0; i < 10; i++) {
		printf ("%d ", arr[i]);
	}
	mem->readLine(n_string, string);
	printf ("%s\n", string);

	delete mem;
	delete[] string;

	return true;
}

bool ex_stream(void *arg) {

	if (!ssi_exists("cursor_bin.stream")) {
		ex_writer(0);
	}

	ssi_stream_t data;
	FileTools::ReadStreamFile("cursor_bin", data);
	FileTools::WriteStreamFile(File::ASCII, "cursor_bin_check", data);

	ssi_stream_print(data, ssiout);
	ssi_stream_destroy(data);

	return true;
}

bool ex_event(void *arg) {

	FileEventsOut eout;
	FileEventsIn ein;

	ssi_event_t empty_event;
	ssi_event_init(empty_event, SSI_ETYPE_EMPTY, Factory::AddString("empty"), Factory::AddString("event"), 0, 5);

	ssi_event_t string_event;
	ssi_event_init(string_event, SSI_ETYPE_STRING, Factory::AddString("string"), Factory::AddString("event"), 100, 10);
	ssi_event_adjust(string_event, 100);
	ssi_sprint(string_event.ptr, "hello world");

	ssi_event_t tuple_event;
	ssi_event_init(tuple_event, SSI_ETYPE_TUPLE, Factory::AddString("floats"), Factory::AddString("event"), 200, 20);
	ssi_event_adjust(tuple_event, 3 * sizeof(ssi_event_tuple_t));
	ssi_real_t *tuple = ssi_pcast(ssi_event_tuple_t, tuple_event.ptr);
	tuple[0] = 0.0f; tuple[1] = 1.0f; tuple[2] = 2.0f;

	ssi_event_t map_event;
	ssi_event_init(map_event, SSI_ETYPE_MAP, Factory::AddString("ntuple"), Factory::AddString("event"), 500, 40);
	ssi_event_adjust(map_event, 3 * sizeof(ssi_event_map_t));
	ssi_event_map_t *map = ssi_pcast(ssi_event_map_t, map_event.ptr);
	map[0].id = Factory::AddString("a"); map[0].value = 0.0f;
	map[1].id = Factory::AddString("b"); map[1].value = 1.0f;
	map[2].id = Factory::AddString("c"); map[2].value = 2.0f;

	eout.open("my", File::ASCII);
	eout.write(empty_event);
	eout.write(string_event);
	eout.write(tuple_event);
	eout.write(map_event);
	eout.close();

	ein.open("my");
	ssi_print("found %u events\n", ein.getSize());
	ssi_event_t *e;
	while (e = ein.next()) {
		ssi_print("> sender=%s, event=%s, from=%u, dur=%u, prob=%.2f, type=%s, state=%s, glue=%u\n",
			Factory::GetString(e->sender_id),
			Factory::GetString(e->event_id),
			e->time,
			e->dur,
			e->prob,
			SSI_ETYPE_NAMES[e->type],
			SSI_ESTATE_NAMES[e->state],
			e->glue_id);
		switch (e->type) {
		case SSI_ETYPE_STRING: {
			ssi_print(" %s\n", e->ptr);
			break;
		}
		case SSI_ETYPE_TUPLE: {
			ssi_size_t n = e->tot / sizeof(ssi_event_tuple_t);
			ssi_event_tuple_t *ptr = ssi_pcast(ssi_event_tuple_t, e->ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				ssi_print("%.2f ", ptr[i]);
			}
			ssi_print("\n");
			break;
		}
		case SSI_ETYPE_MAP: {
			ssi_size_t n = e->tot / sizeof(ssi_event_map_t);
			ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, e->ptr);
			for (ssi_size_t i = 0; i < n; i++) {
				ssi_print("%s=%.2f ", Factory::GetString (ptr[i].id), ptr[i].value);
			}
			ssi_print("\n")
			break;
		}
		}
	};
	ein.close();

	ssi_event_destroy(empty_event);
	ssi_event_destroy(string_event);
	ssi_event_destroy(tuple_event);
	ssi_event_destroy(map_event);

	return true;
}

bool ex_csv(void *arg) {

	// create csv string
	FileMem *mem = FileMem::Create(FileMem::ASCII);
	ssi_size_t n_string = 500;
	ssi_char_t *string = new ssi_char_t[n_string];
	mem->set(n_string, string, true);

	mem->writeLine("A;B;C");
	mem->setFormat(";", ".2");
	mem->setType(SSI_DOUBLE);
	double values[3];
	Randomf random(0, 1);
	for (ssi_size_t i = 0; i < 10; i++) {
		for (ssi_size_t j = 0; j < 3; j++) {
			values[j] = random.next();
		}
		mem->write(values, sizeof(double), 3);	
		mem->writeLine("");
	}

	// read csv file
	FileCSV csv;
	csv.parseString(string, ';', true);
	csv.print();

	delete mem;
	delete[] string;

	return true;
}

bool ex_writer(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, 0, true);
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	File::SetLogLevel(SSI_LOG_LEVEL_DEBUG);

	bool continuous = true;

	FileWriter *writer;

	writer = ssi_create(FileWriter, 0, true);
	writer->getOptions()->setPath("");
	writer->getOptions()->setDelim(" ; ");
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->stream = continuous;
	frame->AddConsumer(cursor_p, writer, "0.5s");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("cursor_txt");
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setDelim(";");
	writer->getOptions()->stream = continuous;
	writer->getOptions()->setMeta("some=meta;hello=world");
	frame->AddConsumer(cursor_p, writer, "0.5s");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("cursor_bin");
	writer->getOptions()->type = File::BINARY;
	writer->getOptions()->stream = continuous;
	frame->AddConsumer(cursor_p, writer, "0.5s");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("cursor_lz4");
	writer->getOptions()->type = File::BIN_LZ4;
	writer->getOptions()->stream = continuous;
	frame->AddConsumer(cursor_p, writer, "0.5s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_simulator(void *arg) {

	if (!ssi_exists("cursor_txt.stream")) {
		ex_writer(0);
	}

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	File::SetLogLevel (SSI_LOG_LEVEL_DEBUG);

	FileReader *reader = ssi_create (FileReader, 0, true);
	reader->getOptions()->setPath("cursor_txt");
	reader->getOptions()->block = 0.05;
	reader->getOptions()->loop = true;
	reader->getOptions()->offset = 0.1;
	ITransformable *cursor_p = frame->AddProvider(reader, SSI_FILEREADER_PROVIDER_NAME);
	frame->AddSensor(reader);

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(cursor_p, writer, "0.05s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	reader->wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_annotation(void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->sr = 50.0;
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = 0;
	
	ezero = ssi_create(ZeroEventSender, 0, true);
	ezero->getOptions()->setAddress("click@mouse");
	ezero->getOptions()->mindur = 0.2;
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	ezero = ssi_create(ZeroEventSender, 0, true);
	ezero->getOptions()->setAddress("click@mouse");
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->empty = false;
	ezero->getOptions()->setString("string");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	StringEventSender *ssender = ssender = ssi_create(StringEventSender, 0, true);
	ssender->getOptions()->setAddress("cursor@string");
	ssender->getOptions()->mean = true;
	frame->AddEventConsumer(cursor_p, ssender, board, ezero->getEventAddress());
	board->RegisterSender(*ssender);

	MapEventSender *msender = 0;
	
	msender = ssi_create(MapEventSender, 0, true);
	msender->getOptions()->setAddress("cursor@map");
	msender->getOptions()->setKeys("x,y");
	frame->AddEventConsumer(cursor_p, msender, board, ezero->getEventAddress());
	board->RegisterSender(*msender);

	msender = ssi_create(MapEventSender, 0, true);
	msender->getOptions()->setAddress("cursor@map");
	msender->getOptions()->setKeys("y,x");
	frame->AddEventConsumer(cursor_p, msender, board, ezero->getEventAddress());
	board->RegisterSender(*msender);
	
	FileAnnotationWriter *annotation = 0;
			
	annotation = ssi_create(FileAnnotationWriter, 0, true);	
	annotation->getOptions()->setAnnoPath("discrete_empty_and_string");
	annotation->getOptions()->setSchemePath("scheme_discrete.annotation");
	annotation->getOptions()->setDefaultLabel("empty");
	annotation->getOptions()->defaultConfidence = 0.5f;
	annotation->getOptions()->setMeta("annotator=hans;role=wurscht");
	board->RegisterListener(*annotation, ezero->getEventAddress());	

	annotation = ssi_create(FileAnnotationWriter, 0, true);
	annotation->getOptions()->setAnnoPath("discrete_map");
	annotation->getOptions()->setSchemePath("scheme_discrete.annotation");
	annotation->getOptions()->mapKeyIndex = 1;
	annotation->getOptions()->setMeta("annotator=hans;role=wurscht");
	board->RegisterListener(*annotation, msender->getEventAddress());

	annotation = ssi_create(FileAnnotationWriter, 0, true);
	annotation->getOptions()->setAnnoPath("free_empty_and_string");
	annotation->getOptions()->setSchemePath("scheme_free");
	annotation->getOptions()->setDefaultLabel("empty");
	annotation->getOptions()->defaultConfidence = 0.5f;
	annotation->getOptions()->setMeta("annotator=hans;role=wurscht");
	board->RegisterListener(*annotation, ezero->getEventAddress());

	annotation = ssi_create(FileAnnotationWriter, 0, true);
	annotation->getOptions()->setAnnoPath("free_map");
	annotation->getOptions()->setSchemePath("scheme_free");
	annotation->getOptions()->mapKeyIndex = 1;
	annotation->getOptions()->setMeta("annotator=hans;role=wurscht");
	board->RegisterListener(*annotation, msender->getEventAddress());

	annotation = ssi_create(FileAnnotationWriter, 0, true);
	annotation->getOptions()->setAnnoPath("continuous");
	annotation->getOptions()->setSchemePath("scheme_continuous");
	annotation->getOptions()->setMeta("annotator=hans;role=wurscht");
	frame->AddConsumer(cursor_p, annotation, "1.0s");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->setPos(CONSOLE_WIDTH, 0, 600, 300);
	board->RegisterListener(*monitor, 0, 60000);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_samples(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	ssi_event_t e_class;
	ssi_event_init(e_class, SSI_ETYPE_STRING, Factory::AddString("sender"), Factory::AddString("class"), 0, 0, SSI_MAX_CHAR);

	Mouse *mouse = ssi_create(Mouse, 0, false);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	FileSampleWriter *writer = 0;
	
	writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setClasses("A;B");
	writer->getOptions()->setUser("user");
	frame->AddConsumer(cursor_p, writer, "1.0s");
	board->RegisterListener(*writer, "class@");

	writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->setPath("test");
	writer->getOptions()->type = File::BINARY;
	writer->getOptions()->setClasses("A;B");
	writer->getOptions()->setUser("user");
	frame->AddConsumer(cursor_p, writer, "1.0s");
	board->RegisterListener(*writer, "class@");

	writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setClasses("A;B");
	writer->getOptions()->setUser("user");
	writer->getOptions()->streamClassIndex = 1;
	ITransformable *input[2] = { cursor_p, cursor_p };
	frame->AddConsumer(2, input, writer, "1.0s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	Sleep(2000);
	ssi_strcpy(e_class.ptr, "B");
	board->update(e_class);

	Sleep(2000);
	ssi_strcpy(e_class.ptr, "A");
	board->update(e_class);

	Sleep(2000);
	ssi_strcpy(e_class.ptr, "");
	board->update(e_class);

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

void test (File &file, int *data_out, ssi_size_t size) {

	int *data_in = new int[size];

	int64_t pos = file.tell ();
	if (file.getType () == File::ASCII) {
		if (!file.write (data_out, 2, size)) {
			ssi_err ("write() failed");
		}
	} else {
		if (!file.write (data_out, sizeof (int), size)) {
			ssi_err ("write() failed");
		}
	}
	if (!file.writeLine ("a b c d e f g h i j k l m n o p q r s t u v w x y z")) {
		ssi_err ("writeLine() failed");
	}

	file.seek (pos);
	if (file.getType () == File::ASCII) {
		if (!file.read (data_in, 2, size)) {
			ssi_err ("read() failed");
		}
	} else {
		if (!file.read (data_in, sizeof (int), size)) {
			ssi_err ("read() failed");
		}
	}
	char buffer[100];
	if (!file.readLine (100, buffer)) {
		ssi_err ("readLine() failed");
	}

	for (ssi_size_t i = 0; i < size; i++) {
		ssi_print ("%d ", data_in[i]);
	}
	ssi_print ("\n");
	ssi_print ("%s\n", buffer);

	delete[] data_in;
}

bool ex_socket(void *arg) {

	Socket::SetLogLevel(SSI_LOG_LEVEL_DEBUG);

	{
		Socket *sender = Socket::CreateAndConnect(Socket::UDP, Socket::CLIENT, 1111, "localhost");
		Socket *receiver = Socket::CreateAndConnect(Socket::UDP, Socket::SERVER, 1111, "localhost");

		ssi_char_t msg[] = "hello world!";
		sender->send(msg, ssi_cast(ssi_size_t, strlen(msg) + 1));
		ssi_char_t string[512];

		if (receiver->recv(string, 512) > 0) {
			ssi_print("received message from %s:\n%s\n", receiver->getRecvAddress(), string);
		}
		else {
			ssi_print("couldn't receive message\n");
		}
		int data[5] = { 1, 2, 3, 4, 5 };

		sender->send(data, 5 * sizeof(int));
		int data_recv[512];
		int result = receiver->recv(data_recv, 512 * sizeof(int));
		if (result > 0) {
			ssi_print("received data from %s:\n", receiver->getRecvAddress());
			for (size_t i = 0; i < result / sizeof(int); i++) {
				ssi_print("%d ", data_recv[i]);
			}
			ssi_print("\n");
		}
		else {
			ssi_print("couldn't receive data\n");
		}

		delete sender;
		delete receiver;

	}

	{

		Socket *sender_socket = Socket::Create(Socket::UDP, Socket::CLIENT, 1111, "localhost");
		SocketOsc sender(*sender_socket, 2000);
		Socket *receiver_socket = Socket::Create(Socket::UDP, Socket::SERVER, 1111);
		SocketOsc receiver(*receiver_socket, 2000);

		sender.connect();
		receiver.connect();

		sender.send_message("ssi", "string", 0, 0, "hello world!");

		MyOscListener listener;

		if ((receiver.recv(&listener, 2000)) <= 0) {
			ssi_print("couldn't receive message\n");
		}

		float data[9] = { 1.0f, 2.0f, 3.0f,
			4.0f, 5.0f, 6.0f,
			7.0f, 8.0f, 9.0f };

		sender.send_stream("ssi", 5000, 20.0f, 3, 3, sizeof(float), SSI_FLOAT, data);

		if ((receiver.recv(&listener, 2000)) <= 0) {
			ssi_print("couldn't receive message\n");
		}

		ssi_char_t *event_name[] = { "event_1",
			"event_2",
			"event_3" };
		ssi_real_t event_value[] = { 1.0f,
			2.0f,
			3.0f };

		sender.send_event("ssi", "values", 5000, 20000, SSI_ESTATE_COMPLETED, 3, event_name, event_value);

		if ((receiver.recv(&listener, 2000)) <= 0) {
			ssi_print("couldn't receive message\n");
		}

		sender.disconnect();
		receiver.disconnect();

		delete sender_socket;
		delete receiver_socket;

	}

	{

		ssi_video_params_t params;
		params.heightInPixels = 80;
		params.widthInPixels = 60;
		params.depthInBitsPerChannel = 8;
		params.numOfChannels = 1;

		ssi_size_t n_image = ssi_video_size(params);
		ssi_byte_t *image = new ssi_byte_t[n_image];
		memset(image, 0, n_image);
		ssi_byte_t *check = new ssi_byte_t[n_image];
		memset(check, 1, n_image);

		int port = 9999;
		Socket *socksend = Socket::CreateAndConnect(Socket::UDP, Socket::CLIENT, port, "localhost");
		Socket *sockrecv = Socket::CreateAndConnect(Socket::UDP, Socket::SERVER, port);

		SocketImage imgsend(*socksend);
		imgsend.setLogLevel(SSI_LOG_LEVEL_DEBUG);
		imgsend.sendImage(params, image, n_image);

		SocketImage imgrecv(*sockrecv);
		imgrecv.setLogLevel(SSI_LOG_LEVEL_DEBUG);
		imgrecv.recvImage(params, check, n_image, 5000);

		delete socksend;
		delete sockrecv;

	}

	return true;
}

bool ex_tcp2way(void *arg) {

	Socket::SetLogLevel(SSI_LOG_LEVEL_DEBUG);

	{
		Socket *server = Socket::CreateAndConnect(Socket::TCP, Socket::SERVER, 1234, "localhost");
		::Sleep(1000);

		Socket *client = Socket::CreateAndConnect(Socket::TCP, Socket::CLIENT, 1234, "localhost");
		::Sleep(1000);

		// client to server
		ssi_char_t msg[] = "hello mr server!";
		client->send(msg, ssi_cast(ssi_size_t, strlen(msg) + 1));

		ssi_char_t string[512];
		int len = server->recv(string, 512);
		if (len < 0)
		{
			ssi_wrn("couldn't receive message");
			return false;
		}
		ssi_print("[server____] received from client: %s\n", string);

		//server to client
		ssi_char_t msg2[] = "hello mr client!";
		server->send(msg2, ssi_cast(ssi_size_t, len));

		ssi_char_t string2[512];
		len = client->recv(string2, 512);
		if (len < 0)
		{
			ssi_wrn("couldn't receive message");
			return false;
		}

		string2[len] = '\0';
		ssi_print("[client____] received from server: %s\n", string2);

		delete client;
		delete server;
	}

	return true;
}

bool ex_sender(void *arg) {

	Socket::TYPE *type = ssi_pcast(Socket::TYPE, arg);

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);

	// start mouse

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// start sender and receiver

	SocketWriter *socket_writer_bin = ssi_create (SocketWriter, 0, true);
	socket_writer_bin->getOptions()->port = 1111;
	socket_writer_bin->getOptions()->setHost("localhost");
	socket_writer_bin->getOptions()->type = *type;
	socket_writer_bin->getOptions()->format = SocketWriter::Options::FORMAT::BINARY;
	frame->AddConsumer(cursor_p, socket_writer_bin, "0.25s");

	SocketReader *socket_reader_bin = ssi_create (SocketReader, 0, true);
	socket_reader_bin->getOptions()->port = 1111;
	socket_reader_bin->getOptions()->type = *type;
	socket_reader_bin->getOptions()->format = SocketReader::Options::FORMAT::BINARY;
	socket_reader_bin->getOptions()->setSampleInfo(cursor_p->getSampleRate(), cursor_p->getSampleDimension(), cursor_p->getSampleBytes(), cursor_p->getSampleType());
	ITransformable *socket_reader_bin_p = frame->AddProvider(socket_reader_bin, SSI_SOCKETREADER_PROVIDER_NAME);
	frame->AddSensor(socket_reader_bin);

	SocketWriter *socket_writer_asc = ssi_create (SocketWriter, 0, true);
	socket_writer_asc->getOptions()->port = 2222;
	socket_writer_asc->getOptions()->setHost("localhost");
	socket_writer_asc->getOptions()->type = *type;
	socket_writer_asc->getOptions()->format = SocketWriter::Options::FORMAT::ASCII;
	frame->AddConsumer(cursor_p, socket_writer_asc, "0.25s");

	SocketReader *socket_reader_asc = ssi_create (SocketReader, 0, true);
	socket_reader_asc->getOptions()->port = 2222;
	socket_reader_asc->getOptions()->type = *type;
	socket_reader_asc->getOptions()->format = SocketReader::Options::FORMAT::ASCII;
	socket_reader_asc->getOptions()->setSampleInfo(cursor_p->getSampleRate(), cursor_p->getSampleDimension(), cursor_p->getSampleBytes(), cursor_p->getSampleType());
	ITransformable *socket_reader_asc_p = frame->AddProvider(socket_reader_asc, SSI_SOCKETREADER_PROVIDER_NAME);
	frame->AddSensor(socket_reader_asc);

	SocketWriter *socket_writer_osc = ssi_create (SocketWriter, 0, true);
	socket_writer_osc->getOptions()->port = 3333;
	socket_writer_osc->getOptions()->setHost("localhost");
	socket_writer_osc->getOptions()->type = *type;
	socket_writer_osc->getOptions()->format = SocketWriter::Options::FORMAT::OSC;
	socket_writer_osc->getOptions()->setId("mouse");
	frame->AddConsumer(cursor_p, socket_writer_osc, "0.25s");

	SocketReader *socket_reader_osc = ssi_create (SocketReader, 0, true);
	socket_reader_osc->getOptions()->port = 3333;
	socket_reader_osc->getOptions()->type = *type;
	socket_reader_osc->getOptions()->format = SocketReader::Options::FORMAT::OSC;
	socket_reader_osc->getOptions()->setSampleInfo(cursor_p->getSampleRate(), cursor_p->getSampleDimension(), cursor_p->getSampleBytes(), cursor_p->getSampleType());
	ITransformable *socket_reader_osc_p = frame->AddProvider(socket_reader_osc, SSI_SOCKETREADER_PROVIDER_NAME);
	frame->AddSensor(socket_reader_osc);

	SignalPainter *plot;

	plot =ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->size = 10;
	plot->getOptions()->setTitle("Local Sensor");
	frame->AddConsumer(cursor_p, plot, "5");

	plot =ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->size = 10;
	plot->getOptions()->setTitle("Socker Reader(Binary)");
	frame->AddConsumer(socket_reader_bin_p, plot, "5");

	plot =ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->size = 10;
	plot->getOptions()->setTitle("Socker Reader(ASCII)");
	frame->AddConsumer(socket_reader_asc_p, plot, "5");

	plot =ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->size = 10;
	plot->getOptions()->setTitle("Socker Reader(OSC)");
	frame->AddConsumer(socket_reader_osc_p, plot, "5");

	decorator->add("plot*", 0, 0, 400, CONSOLE_HEIGHT);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_sender_events (void *arg) {

	Socket::TYPE *type = ssi_pcast(Socket::TYPE, arg);

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	// start mouse

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, 0, true);
	ezero->getOptions()->setAddress("click@mouse");
	ezero->getOptions()->mindur = 0.2;
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	MapEventSender *msender = ssi_create(MapEventSender, 0, true);
	msender->getOptions()->setAddress("cursor@mouse");
	msender->getOptions()->setKeys("x,y");
	frame->AddEventConsumer(cursor_p, msender, board, ezero->getEventAddress());
	board->RegisterSender(*msender);

	// start sender and receiver

	SocketEventWriter *socket_event_writer = ssi_create(SocketEventWriter, 0, true);
	socket_event_writer->getOptions()->port = 1234;
	socket_event_writer->getOptions()->setHost("localhost");
	socket_event_writer->getOptions()->type = *type;
	socket_event_writer->getOptions()->osc = true;
	board->RegisterListener(*socket_event_writer, msender->getEventAddress());

	SocketEventReader *socket_event_reader = ssi_create(SocketEventReader, 0, true);
	socket_event_reader->getOptions()->port = 1234;
	socket_event_reader->getOptions()->type = *type;
	socket_event_reader->getOptions()->osc = true;
	board->RegisterSender(*socket_event_reader);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->setPos(CONSOLE_WIDTH, 0, 600, 300);
	board->RegisterListener(*monitor, 0, 60000);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_sender_video(void *arg) {

#if _WIN32|_WIN64

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);

	FakeSignal *camera = ssi_create(FakeSignal, 0, true);
	camera->getOptions()->type = FakeSignal::SIGNAL::IMAGE_SSI;
	camera->getOptions()->sr = 1;
	ITransformable *camera_p = frame->AddProvider(camera, "fake");
	camera->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor(camera);

	SocketWriter *socket_writer_img = ssi_create (SocketWriter, 0, true);
	socket_writer_img->getOptions()->port = 4444;
	socket_writer_img->getOptions()->setHost("localhost");
	socket_writer_img->getOptions()->type = Socket::UDP;
	socket_writer_img->getOptions()->format = SocketWriter::Options::FORMAT::IMAGE;
	frame->AddConsumer(camera_p, socket_writer_img, "1");

	SocketReader *socket_reader_img = ssi_create (SocketReader, 0, true);
	socket_reader_img->getOptions()->port = 4444;
	socket_reader_img->getOptions()->type = Socket::UDP;
	socket_reader_img->getOptions()->format = SocketReader::Options::FORMAT::IMAGE;
	socket_reader_img->getOptions()->setSampleInfo(camera->getFormat());
	ITransformable *socket_reader_img_p = frame->AddProvider(socket_reader_img, SSI_SOCKETREADER_PROVIDER_NAME);
	frame->AddSensor(socket_reader_img);

	VideoPainter *plot;

	plot = ssi_create_id(VideoPainter, 0, "plot");
	frame->AddConsumer(camera_p, plot, "1");

	plot = ssi_create_id(VideoPainter, 0, "plot");
	frame->AddConsumer(socket_reader_img_p, plot, "1");

	decorator->add("plot*", 1, 2, 0, 0, 320, 480);

    frame->Start();
	frame->Wait();
	frame->Stop();
    frame->Clear();

#endif

	return true;
}

void recvFile(void *ptr) {
	Socket *s = Socket::CreateAndConnect(Socket::UDP, Socket::SERVER, 1112);
	ssi_char_t *filepath = 0;
	s->recvFile(&filepath, 2000);
	delete[] filepath;
}

void sendFile(void *ptr) {
	Socket *s = Socket::CreateAndConnect(Socket::UDP, Socket::CLIENT, 1112, "localhost");
	ssi_char_t *filepath = ssi_pcast(ssi_char_t, ptr);
	s->sendFile(filepath);
}

bool ex_sender_file(void *arg) {

    char* name = "file/hello.txt";
	RunAsThread recv(&recvFile, 0, true);
	RunAsThread send(&sendFile, name, true);

	send.start();
	Sleep(1000);
	recv.start();
	recv.stop();
	send.stop();

	return true;
}

bool ex_download_file(void *arg)
{
	WebTools::DownloadFile("https://curl.haxx.se/ca/cacert.pem", "cacert.pem");
	WebTools::DownloadFile("https://github.com/hcmlab/ssi/raw/master/README", "README", "cacert.pem");
	WebTools::DownloadFile("https://github.com/hcmlab/ssi/raw/master/README2", "README", "cacert.pem");

	return true;
}