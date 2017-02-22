#include "qtsoxtest.h"
#include "ui_qtsoxtest.h"

QAudioOutput *m_audioOutput;
QIODevice *m_output;


QBuffer wav_audio_buffer;
QByteArray buffer(32768, 0);
int wav_buffersize = 4096/2;
bool soundOutput = true;

bool drainpipeline = false;


//analyze:
int readSampleCount = 0;
int readCalls = 0;
int writeCalls = 0;
int writeSampleCount = 0;
bool firstWrite = true;
bool lastWasWrite = false;
std::vector<std::pair<int, int>> analyzeReadList;
std::vector<std::pair<int, int>> analyzeWriteList;


QString fileName = NULL;


QtSoXTest::QtSoXTest(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	m_Outputdevice(QAudioDeviceInfo::defaultOutputDevice())
{
	ui.setupUi(this);

	this->ui.labelSoxInfo->setText("SoX Info:\nVersion:\t\t" + QString(sox_version_info()->version) + 
	"\nBuild date:\t" + sox_version_info()->time + 
	"\nCompiler:\t\t" + sox_version_info()->compiler);
}

QtSoXTest::~QtSoXTest()
{
}


void QtSoXTest::initializeAudio(int sampleRate)
{
    m_format.setFrequency(sampleRate); //set frequency to sampleRate
    m_format.setChannels(1); //set channels to mono
    m_format.setSampleSize(16); //set sample size to 16 bit
    m_format.setSampleType(QAudioFormat::SignedInt ); //Sample type as signed integer sample
    m_format.setByteOrder(QAudioFormat::LittleEndian); //Byte order
    m_format.setCodec("audio/pcm"); //set codec as simple audio/pcm

    QAudioDeviceInfo infoOut(QAudioDeviceInfo::defaultOutputDevice());

    if (!infoOut.isFormatSupported(m_format))
    {
       //Default format not supported - trying to use nearest
        m_format = infoOut.nearestFormat(m_format);
    }

	if (sampleRate != m_format.frequency()) 
		QMessageBox::information(this, "Audio output", "The audio device couldn't be set to the requested samplerate.\nInstead it was set to " + 
		QString::number(m_format.frequency()) + " Hz.");

    m_audioOutput = new QAudioOutput(m_Outputdevice, m_format, this);
}


/* The function that will be called to input samples into the effects chain.*/
int QtSoXTest::input_drain(
    sox_effect_t * effp, sox_sample_t * obuf, size_t * osamp)
{
	//while (m_audioOutput->bytesFree() < wav_buffersize) {Sleep(1);};		//wait for next block to play

	if (!drainpipeline) {
		*osamp = wav_audio_buffer.read(buffer.data(), wav_buffersize);			//write bytes into buffer
		*osamp /= 2;															//bytes were read but samples are needed -> assuming 16 bit (short)

		for (int i = 0; i < *osamp; i++) {
			sox_int16_t sample = ((sox_int16_t*)buffer.data())[i];				//get signed 16 bit samples from the bytes
			obuf[i] = SOX_SIGNED_TO_SAMPLE(16,sample);							//convert to the signed 32 bit internal sample format 
		}

		//analyze
		readSampleCount += *osamp;
		readCalls++;
		if (lastWasWrite) {
			//qDebug() << "total output samples since last read: " << writeSampleCount << " in " << writeCalls << " write calls";
			analyzeWriteList.push_back(std::pair<int, int>(writeSampleCount,writeCalls));

			lastWasWrite = false;
			writeSampleCount = 0;
			writeCalls = 0;
		}
	} 
	else *osamp = 0;

	return *osamp? SOX_SUCCESS : SOX_EOF;
}


/* A `stub' effect handler to handle inputting samples to the effects
 * chain; the only function needed for this example is `drain' */
sox_effect_handler_t const * QtSoXTest::input_handler(void)
{
	static sox_effect_handler_t handler = {
	"input", NULL, SOX_EFF_MCHAN, NULL, NULL, NULL, input_drain, NULL, NULL, 0
	};
	return &handler;
}


//always called once at the initialization;
/* The function that will be called to output samples from the effects chain.*/
int QtSoXTest::output_flow(sox_effect_t *effp LSX_UNUSED, sox_sample_t const * ibuf,
    sox_sample_t * obuf LSX_UNUSED, size_t * isamp, size_t * osamp)
{
	assert(*isamp <= 32768);
	sox_int16_t outbuf[32768];												//temporary integer 16 bit sample buffer
	for (int i = 0; i < *isamp; i++) {
		if (ibuf[i]>SOX_SAMPLE_MAX-(1<<(31-16))) outbuf[i] = SOX_INT_MAX(16);
		else outbuf[i] = ((sox_uint32_t)(ibuf[i]+(1<<(31-16))))>>(32-16);		//internally used signed 32 bit -> 16 bit integer
	}
	
	if (soundOutput) {
		while (m_audioOutput->bytesFree() < wav_buffersize) {					//wait for next block to play
			Sleep(1);
			QCoreApplication::processEvents();
		};		
		m_output->write((char*)outbuf, *isamp*2);								//audio output with bytes
	}

	*osamp = 0;																//last chain member -> no output samples

	//analyze
	writeSampleCount += *isamp;
	writeCalls++;
	if (!lastWasWrite && !firstWrite) {
		//qDebug() << "total input samples since last write: " << readSampleCount << " in " << readCalls << " read calls";
		analyzeReadList.push_back(std::pair<int, int>(readSampleCount,readCalls));
		lastWasWrite = true;
		readSampleCount = 0;
		readCalls = 0;
	}

	if (firstWrite) {
		firstWrite = false; 
		writeCalls = 0;		//write calls only with written samples
	}

	return SOX_SUCCESS; /* All samples output successfully */
}


/* A `stub' effect handler to handle outputting samples from the effects
 * chain; the only function needed for this example is `flow' */
sox_effect_handler_t const * QtSoXTest::output_handler(void)
{
	static sox_effect_handler_t handler = {
	"output", NULL, SOX_EFF_MCHAN, NULL, NULL, output_flow, NULL, NULL, NULL, 0
	};
	return &handler;
}



static void consume_whitespace (const char **input)
{
  while (isspace (**input)) (*input)++;
}

/*
adapted version from:
http://gcc.gnu.org/svn/gcc/branches/cilkplus/libiberty/argv.c
*/
char **buildargv (const char *input, int *argc_out)
{
	char *arg;
	char *copybuf;
	int squote = 0, dquote = 0, bsquote = 0;
	int argc = 0;
	int maxargc = 0;
	char **argv = NULL;
	char **nargv;

	if (input != NULL)
	{
		copybuf = (char *) malloc (strlen (input) + 1);
		/* Is a do{}while to always execute the loop once.  Always return an
		argv, even for null strings.*/
		do
		{
			/* Pick off argv[argc] */
			consume_whitespace (&input);

			if ((maxargc == 0) || (argc >= (maxargc - 1)))
			{
				/* argv needs initialization, or expansion */
				if (argv == NULL)
				{
					maxargc = 8;
					nargv = (char **) malloc (maxargc * sizeof (char *));
				}
				else
				{
					maxargc *= 2;
					nargv = (char **) realloc (argv, maxargc * sizeof (char *));
				}
				argv = nargv;
				argv[argc] = NULL;
			}
			/* Begin scanning arg */
			arg = copybuf;
			while (*input != '\0')
			{
				if (isspace (*input) && !squote && !dquote && !bsquote) break;
				else
				{
					if (bsquote)
					{
						bsquote = 0;
						*arg++ = *input;
					}
					else if (*input == '\\') bsquote = 1;
					else if (squote)
					{
						if (*input == '\'') squote = 0;
						else *arg++ = *input;
					}
					else if (dquote)
					{
						if (*input == '"') dquote = 0;
						else *arg++ = *input;
					}
					else
					{
						if (*input == '\'') squote = 1;
						else if (*input == '"') dquote = 1;
						else *arg++ = *input;
					}
					input++;
				}
			}
			*arg = '\0';
			argv[argc] = strdup (copybuf);
			argc++;
			argv[argc] = NULL;

			consume_whitespace (&input);
		}
		while (*input != '\0');

		free (copybuf);
	}
	*argc_out = argc;
	return (argv);
}




void QtSoXTest::playWav(QString fileName, const char* effectArgStr, int soxGlobalBuf, int soxSignalLength, int sampleRate) 
{
	readSampleCount = 0;
	writeSampleCount = 0;
	readCalls = 0;
	writeCalls = 0;
	firstWrite = true;
	lastWasWrite = false;
	drainpipeline = false;


	QByteArray audio_data;
	
	QFile audio_file(fileName);
	if(audio_file.open(QIODevice::ReadOnly)) {
		//read the wav file to an buffer

		audio_file.seek(44); // skip wav header
		audio_data = audio_file.readAll();			//loading the full file into ram
		audio_file.close();

		wav_audio_buffer.setBuffer(&audio_data);
		wav_audio_buffer.open(QIODevice::ReadOnly);
		wav_audio_buffer.seek(0);
		qDebug() << "filename: " << fileName << "size: " << wav_audio_buffer.size();


		qDebug() << "player starting";

		qDebug() << "Using libSoX:\n" << 
					"Version:\t\t" << sox_version_info()->version << "\n" <<
					"Build time:\t" << sox_version_info()->time << "\n" <<
					"Compiler:\t\t" << sox_version_info()->compiler << "\n";


		//init sox
		sox_init();			

		sox_globals.bufsiz = soxGlobalBuf;
		sox_globals.use_threads = sox_bool::sox_true;


		//set properties of the signal

		sox_signalinfo_t signal;
		sox_encodinginfo_t encoding;
		
		signal.channels = 1;
		signal.length = soxSignalLength;
		signal.mult = 0;
		signal.precision = 16;
		signal.rate = sampleRate;

		encoding.bits_per_sample = 16;
		encoding.compression = 0;
		encoding.opposite_endian = sox_false;
		encoding.reverse_bits =  sox_option_t::sox_option_no;
		encoding.reverse_bytes = sox_option_t::sox_option_no;
		encoding.reverse_nibbles = sox_option_t::sox_option_no;
		encoding.encoding = sox_encoding_t::SOX_ENCODING_SIGN2;


		bool noError = true;

		//effect chain
		sox_effects_chain_t * chain;
		sox_effect_t * e;

		//create the effect chain
		chain = sox_create_effects_chain(&encoding, &encoding);
		
		//effect chain input (handler overwrite)
		e = sox_create_effect(input_handler());
		noError = sox_add_effect(chain, e, &signal, &signal) == SOX_SUCCESS;
		assert(noError);
		free(e);

		//create the effects with their options by using the commandline like string
		int argc;
		//convert string to argv
		char** argv = buildargv(effectArgStr, &argc);
		
		if (argc > 0 && strlen(argv[0]) > 0) {
			noError = sox_find_effect(argv[0]);
			assert(noError);
			if (!noError) { QMessageBox::critical(this, "SoX Error", "First argument is no effect name!", QMessageBox::Ok);
				goto end;
			}

			std::vector<char*> argvEffect;
			char *effectName = argv[0];		//first parameter has to be an effect!
			int effectCount = 1;

			for (int i = 1; i < argc+1; i++) {
				bool isEffect = false;

				if (i != argc) {
					isEffect = sox_find_effect(argv[i]);
					if (!isEffect) argvEffect.push_back(argv[i]);
				}

				if (i == argc || isEffect) {
					//create effect
					e = sox_create_effect(sox_find_effect(effectName));
					if (argvEffect.size() == 0) noError = sox_effect_options(e, 0, NULL) == SOX_SUCCESS;
					else noError = sox_effect_options(e, argvEffect.size(), &argvEffect[0]) == SOX_SUCCESS;
					assert(noError);

					if (!noError) { 
						QMessageBox::critical(this, "SoX Error", 
						"Effect option error with effect \"" + QString(effectName) +"\"!\n\nUsage:\n"+ QString(e->handler.usage), QMessageBox::Ok);
						argvEffect.clear();
						free(e);						
						goto end;
					}
					noError = sox_add_effect(chain, e, &signal, &signal) == SOX_SUCCESS;
					assert(noError);
					free(e);

					if (!noError) {
						QMessageBox::critical(this, "SoX Error", "Effect \"" + QString(effectName) +"\" couldn't be added to the effect chain!");
						argvEffect.clear();			
						goto end;
					}

					qDebug() << "Added effect: " << effectName << " with options:";
					for (int j = 0; j < argvEffect.size(); j++) qDebug() << argvEffect.at(j);

					argvEffect.clear();

					if (i == argc) break;

					effectName = argv[i];
					effectCount++;
				}
			}
		}

		
		//effect chain output (handler overwrite)
		e = sox_create_effect(output_handler());
		noError = sox_add_effect(chain, e, &signal, &signal) == SOX_SUCCESS;
		assert(noError);
		free(e);

		//start the signal flow in the effect chain
		sox_flow_effects(chain, NULL, NULL);

end:
		//all data processed -> clean up sox
		sox_delete_effects_chain(chain);
		sox_quit();

		//other cleanup
		audio_data.clear();
		wav_audio_buffer.close();


		qDebug() << "player finished";

	}
}


void QtSoXTest::on_pushButtonLoad_clicked()
{
	fileName = QFileDialog::getOpenFileName(this, tr("Open Wav: 16 bit signed"), ".", tr("Wav files (*.wav)"));
	if (fileName != NULL) this->ui.pushButtonPlay->setEnabled(true);
}


void QtSoXTest::on_pushButtonStop_clicked()
{
	drainpipeline = true;
}


void QtSoXTest::on_pushButtonPlay_clicked()
{
	if (fileName != NULL) {
		this->ui.pushButtonPlay->setEnabled(false);
		this->ui.pushButtonStop->setEnabled(true);

		this->ui.tableWidget->setRowCount(0);	//clear table

		wav_buffersize = this->ui.spinBoxReadBuf->value();
		soundOutput = this->ui.checkBoxSoundOut->isChecked();

		int sampleRate = this->ui.spinBoxSRate->value();

		//audioOutput
		if (soundOutput) {
			initializeAudio(sampleRate);			
			m_output = m_audioOutput->start();	//start audio output
		}

		playWav(fileName, this->ui.lineEditSoXArgs->text().toStdString().c_str(), this->ui.spinBoxGlobalBuf->value(), this->ui.spinBoxSignalBuf->value(), sampleRate);

		//in / out table
		for (int i = 0; i < max(analyzeReadList.size(), analyzeWriteList.size()); i++)
		{
			int lastRow = this->ui.tableWidget->rowCount();
			this->ui.tableWidget->insertRow(lastRow);
			
			if (analyzeReadList.size()-1 < i) for (int j = 0; j < 2; j++) this->ui.tableWidget->setItem(lastRow, j, new QTableWidgetItem("0"));
			else {
				std::pair<int,int> *currPair = &analyzeReadList.at(i);
				this->ui.tableWidget->setItem(lastRow,0, new QTableWidgetItem(QString::number(currPair->first)));
				
				QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(currPair->second));
				if (currPair->second > 1) newItem->setBackgroundColor(QColor(255,0,0,100));
				this->ui.tableWidget->setItem(lastRow,1, newItem);
			}

			if (analyzeWriteList.size()-1 < i) for (int j = 2; j < 4; j++) this->ui.tableWidget->setItem(lastRow, j, new QTableWidgetItem("0"));
			else {
				std::pair<int,int> *currPair = &analyzeWriteList.at(i);
				this->ui.tableWidget->setItem(lastRow,2, new QTableWidgetItem(QString::number(currPair->first)));

				QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(currPair->second));
				if (currPair->second > 1) newItem->setBackgroundColor(QColor(255,0,0,100));
				this->ui.tableWidget->setItem(lastRow,3, newItem);
			}
		}
		
		this->ui.tableWidget->resizeRowsToContents();

		analyzeReadList.clear();
		analyzeWriteList.clear();

		this->ui.pushButtonPlay->setEnabled(true);
		this->ui.pushButtonStop->setEnabled(false);

		//audioOutput
		if (soundOutput) { 	
			m_audioOutput->stop();	//stop audio output
			delete m_audioOutput;
		}
	}
}