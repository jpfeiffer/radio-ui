#include "RadioGui.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QCommandLineOption optStations(QStringList() << "s" << "stations-file", "Read stations from file <file>.", "file");
	QCommandLineOption optCursor(QStringList() << "c" << "cursor", "Display the cursor and do not hide it");

	QCommandLineParser parser;
	parser.addOption(optStations);
	parser.addOption(optCursor);
	parser.addHelpOption();

	parser.process(QCoreApplication::arguments());

	QString stationsFileName = QString("stations.json");
	if(true == parser.isSet(optStations))
	{
		stationsFileName = parser.value(optStations);
	}

	if(false == parser.isSet(optCursor))
	{
		QApplication::setOverrideCursor(Qt::BlankCursor);
	}

	RadioGui w(stationsFileName);
	w.show();

	return a.exec();
}
