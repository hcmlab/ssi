#ifndef QTSOXTEST_H
#define QTSOXTEST_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <qaudiooutput.h>

#include <QBuffer>
#include <QtCore>

#include <QDebug>

#include "ui_qtsoxtest.h"

#include <assert.h>

#include "sox.h"


#include <windows.h>


class QtSoXTest : public QMainWindow
{
	Q_OBJECT

public:
	QtSoXTest(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QtSoXTest();

private:
	Ui::QtSoXTestClass ui;


	QAudioDeviceInfo m_Outputdevice;
	QAudioFormat m_format;
	

	static int input_drain(sox_effect_t * effp, sox_sample_t * obuf, size_t * osamp);
	static sox_effect_handler_t const * input_handler(void);
	static int output_flow(sox_effect_t *effp LSX_UNUSED, sox_sample_t const * ibuf,sox_sample_t * obuf LSX_UNUSED, size_t * isamp, size_t * osamp);
	static sox_effect_handler_t const * output_handler(void);


	void initializeAudio(int sampleRate);
	void playWav(QString fileName, const char* effectArgStr, int soxGlobalBuf, int soxSignalLength, int sampleRate);

private slots:
	void on_pushButtonLoad_clicked();
	void on_pushButtonPlay_clicked();
	void on_pushButtonStop_clicked();

};

#endif // QTSOXTEST_H
