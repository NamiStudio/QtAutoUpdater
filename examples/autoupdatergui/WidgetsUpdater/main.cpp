#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/main.ico")));
	QApplication::setApplicationDisplayName(QStringLiteral("Widgets-Test"));

	qputenv("QTAUTOUPDATERCORE_PLUGIN_OVERWRITE",
			(QCoreApplication::applicationDirPath() + QStringLiteral("/../../../plugins/updaters/")).toUtf8());

	MainWindow w;
	w.show();

	return a.exec();
}
