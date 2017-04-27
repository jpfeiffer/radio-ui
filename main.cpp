#include "RadioGui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	RadioGui w;
	w.show();

	return a.exec();
}
