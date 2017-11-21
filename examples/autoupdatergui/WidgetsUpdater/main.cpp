#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/main.ico")));
	QApplication::setApplicationDisplayName(QStringLiteral("Widgets-Test"));

	qputenv("PLUGIN_UPDATERS_PATH", DBG_PLUGIN_DIR);

	MainWindow w;
	w.show();

	return a.exec();
}
