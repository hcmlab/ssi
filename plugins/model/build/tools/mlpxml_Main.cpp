// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/29
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

// --run -signal cursor -anno button -user user D:\wagner\openssi\core\build\tools\xmltrain\mlp\mlp
// --train -eval -1 -kfolds 2 D:\wagner\openssi\core\build\tools\xmltrain\mlp\mlp mlp mlp mlp
// --run -trainer mlp D:\wagner\openssi\core\build\tools\xmltrain\mlp\mlp

#include "ssi.h"
#include "MlpXmlTrain.h"
#include "ssimlpxml.h"

using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define SSI_PIPE_NAME_WRITE "\\\\.\\pipe\\MlpXmlWrite"
#define SSI_PIPE_NAME_READ "\\\\.\\pipe\\MlpXmlRead"

void run (const ssi_char_t *pipeline, const ssi_char_t *signal, const ssi_char_t *anno, const ssi_char_t *trainer, const ssi_char_t *user, bool remote);
void train (const ssi_char_t *pipeline, const ssi_char_t *traindef, const ssi_char_t *training, const ssi_char_t *trainer, 
			int eval, int kfolds, bool reex, bool remote, ssi_size_t reps, ssi_real_t fps);

class MyPipeListener : public NamedPipeListener {
public:
	MyPipeListener (Event &stop)
		: _stop (stop) {};
	void receive (ssi_size_t n_bytes, ssi_byte_t *bytes) {
		_stop.release ();
	}
protected:
	Event &_stop;
};

class MyMlpCallback : public MlpXmlICallback {
public:
	MyMlpCallback (NamedPipe &pipe)
		: _pipe (pipe) {
	}
	void call (ssi_time_t time,
		ssi_time_t duration,
		const char *label,
		bool store,
		bool change) {
		ssi_sprint (string, "%lf,%lf,%s,%d,%d", time, duration, label, store ? 1 : 0, change ? 1 : 0);
		ssi_print ("%s\n", string);
		_pipe.send (string);
	}
protected:
	ssi_char_t string[SSI_MAX_CHAR];
	NamedPipe &_pipe;
};

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	ssi_char_t *pipeline = 0;	
	ssi_char_t *training = 0;
	ssi_char_t *traindef = 0;
	ssi_char_t *trainer = 0;
	ssi_char_t *signal = 0;
	ssi_char_t *anno = 0;
	ssi_char_t *user = 0;
	ssi_char_t *log = 0;
	bool remote = false;
	bool reex = false;
	int eval = -1;
	int kfolds = 2;
	int reps = 1;
	float fps = 30.0f;

	cmd.addMasterSwitch ("--run");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("pipeline", &pipeline, "path to pipeline");	

	cmd.addText("\nOptions:");
	cmd.addBCmdOption("-remote", &remote, false, "remote with process via named pipes");
	cmd.addSCmdOption("-signal", &signal, "", "signal name");
	cmd.addSCmdOption("-anno", &anno, "", "anno name");
	cmd.addSCmdOption("-trainer", &trainer, "", "trainer filepath");
	cmd.addSCmdOption("-user", &user, "", "user name");
	cmd.addSCmdOption("-log", &log, "", "output to log file");

	cmd.addMasterSwitch ("--train");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("pipeline", &pipeline, "path to pipeline");	
	cmd.addSCmdArg("traindef", &traindef, "path to traindef");
	cmd.addSCmdArg("training", &training, "path to training");	
	cmd.addSCmdArg("trainer", &trainer, "path to trainer or evaluation result (if -eval option is set)");

	cmd.addText("\nOptions:");
	cmd.addBCmdOption("-remote", &remote, false, "remote with process via named pipes");
	cmd.addICmdOption("-eval", &eval, -1, "set evaluate mode (0=KFOLD,1=LOO,2=LOUO,3=FULL,4=ContLOUO)");
	cmd.addICmdOption("-kfolds", &kfolds, 2, "set number of folds for KFOLD evaluation");
	cmd.addBCmdOption("-reex", &reex, false, "always re-extract features");
	cmd.addSCmdOption("-log", &log, "", "output to log file");
	cmd.addFCmdOption("-fps", &fps, 30.0f, "simulated frame rate of continuous evaluation");
	cmd.addICmdOption("-reps", &reps, 1, "number of repetitions (continuous evaluation only)");

	if (cmd.read (argc, argv)) {		

		FilePath fp (pipeline);
		ssi_char_t workdir_old[SSI_MAX_CHAR];
		::GetCurrentDirectory (SSI_MAX_CHAR, workdir_old);
		::SetCurrentDirectory (fp.getDir ());

		if (log[0] != '\0') {
			ssi_log_file_begin (log);
		}

		switch (cmd.master_switch) {

			case 1: {							

				run (pipeline, signal, anno, trainer, user, remote);
				break;
			}
			case 2: {

				train (pipeline, traindef, training, trainer, eval, kfolds, reex, remote, reps, fps);
				break;
			}	
		}

		Factory::Clear ();
		if (!remote) {
			ssi_print ("\n\n\tpress enter to quit!\n");
			getchar ();
		}

		if (log[0] != '\0') {
			ssi_log_file_end ();
		}

		::SetCurrentDirectory (workdir_old);
	}

	delete[] pipeline;	
	delete[] training;
	delete[] traindef;
	delete[] trainer;
	delete[] signal;
	delete[] anno;
	delete[] user;
	delete[] log;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void run (const ssi_char_t *pipeline, const ssi_char_t *signal, const ssi_char_t *anno, const ssi_char_t *trainer, const ssi_char_t *user, bool remote) {

	Factory::RegisterDLL ("ssiframe.dll", ssiout);
	Factory::RegisterDLL ("ssievent.dll", ssiout);

	Event *stop_event = 0;
	MyPipeListener *pipe_listener = 0;
	MyMlpCallback *mlp_callback = 0;
	NamedPipe *pipe_read = 0;
	NamedPipe *pipe_write = 0;
	IConsumer *consumer = 0;
	IMlpXml *mlp = 0;
	XMLPipeline *xmlpipe = 0;

	if (remote) {	
		stop_event = new Event ();
		pipe_listener = new MyPipeListener (*stop_event);
		pipe_read = NamedPipe::Create (SSI_PIPE_NAME_READ, NamedPipe::CLIENT, NamedPipe::READ);
		pipe_write = NamedPipe::Create (SSI_PIPE_NAME_WRITE, NamedPipe::CLIENT, NamedPipe::WRITE);
	
		pipe_read->open();
		pipe_write->open();

		if (pipe_read->isOpen()) {
			pipe_read->startListening(pipe_listener);
		}
	}

	xmlpipe = ssi_create (XMLPipeline, 0, false);
	if (!xmlpipe) {
		ssi_wrn ("could not create pipeline parser");
		goto end;
	}
	xmlpipe->SetRegisterDllFptr(Factory::RegisterDLL);
	if (!xmlpipe->parse(pipeline)) {
		ssi_wrn ("could not parse pipeline '%s'", pipeline);
		goto end;
	} 

	consumer = xmlpipe->getConsumer(MlpXml::GetCreateName());
	if (!consumer) {
		ssi_wrn ("could not parse mlp '%s'", pipeline);
		goto end;
	}
	mlp = ssi_pcast (IMlpXml, consumer); 
	mlp->getOptions()->setOptionValue("trainer", ssi_ccast(ssi_char_t *, trainer));
	mlp->getOptions()->setOptionValue("anno", ssi_ccast(ssi_char_t *, anno));
	mlp->getOptions()->setOptionValue("signal", ssi_ccast(ssi_char_t *, signal));
	if (user[0] != '\0') {
		mlp->getOptions()->setOptionValue("user", ssi_ccast(ssi_char_t *, user));
	}

	if (remote) {
		if (pipe_write->isOpen()) {
			mlp_callback = new MyMlpCallback (*pipe_write);
			mlp->setCallback(mlp_callback);
		}		
	}

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	if (remote) {
		stop_event->wait();
	} else {
		ssi_print ("\n\n\t\tpress enter to stop!\n\n");
		getchar ();
	}
	frame->Stop();
	frame->Clear();

end:

	if (remote) {
		if (pipe_read->isOpen()) {
			pipe_read->stopListening();
		}
		if (mlp) {
			mlp->setCallback(0);
		}
	}

	delete xmlpipe;
	delete pipe_write;
	delete pipe_read;
	delete stop_event;
	delete pipe_listener;
	delete mlp_callback;
}

void train (const ssi_char_t *pipeline, const ssi_char_t *traindef, const ssi_char_t *training, const ssi_char_t *trainer, 
			int eval, int kfolds, bool reex, bool remote, ssi_size_t reps, ssi_real_t fps) {

	Factory::RegisterDLL ("ssiframe.dll", ssiout);
	Factory::RegisterDLL ("ssievent.dll", ssiout);

	NamedPipe *pipe_write = 0;
	IConsumer *consumer = 0;
	IMlpXml *mlp = 0;
	MlpXmlTrain train (traindef);
	XMLPipeline *xmlpipe = 0;

	if (remote) {
		pipe_write = NamedPipe::Create (SSI_PIPE_NAME_WRITE, NamedPipe::CLIENT, NamedPipe::WRITE);
		pipe_write->open();
	}

	xmlpipe = ssi_create (XMLPipeline, 0, false);
	if (!xmlpipe) {
		ssi_wrn ("could not create pipeline parser");
		goto end;
	}
	xmlpipe->SetRegisterDllFptr(Factory::RegisterDLL);
	if (!xmlpipe->parse(pipeline)) {
		ssi_wrn ("could not parse pipeline '%s'", pipeline);
		goto end;
	}

	consumer = xmlpipe->getConsumer(MlpXml::GetCreateName());
	if (!consumer) {
		ssi_wrn ("could not parse mlp '%s'", pipeline);
		goto end;
	}
	mlp = ssi_pcast (IMlpXml, consumer); 
	int type = 0;
	mlp->getOptions()->getOptionValue("type", &type);
	
	//train.load_transf ();	
	if (!train.load (training, reex, eval)) {
		ssi_wrn ("could not load training '%s'", training);
		goto end;			
	}		
	if (eval != -1) {
		if (!train.eval (trainer, eval, kfolds, reps, fps)) {
			goto end;
		}
	} else {
		if (!train.train (trainer)) {
			goto end;
		}
	}

end:

	Factory::GetFramework ()->Clear();
	delete pipe_write;	
	delete xmlpipe;

	return;
}
