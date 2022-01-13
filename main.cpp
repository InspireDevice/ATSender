#include <QtGui/QApplication>
#include "compretender.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ComPretender w;
	w.show();
	return a.exec();
}
