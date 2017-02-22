#include "qtsoxtest.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtSoXTest w;
	w.show();
	return a.exec();
}
