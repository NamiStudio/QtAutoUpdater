#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QtAutoUpdaterCore/Updater>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	if(a.arguments().size() < 2) {
		qCritical() << "Usage: ConsoleUpdater <path_to_maintenancetool>";
		return EXIT_FAILURE;
	}

	qputenv("QTAUTOUPDATERCORE_PLUGIN_OVERWRITE",
			(QCoreApplication::applicationDirPath() + QStringLiteral("/../../../plugins/updaters/")).toUtf8());

	QtAutoUpdater::Updater *updater = new QtAutoUpdater::Updater(a.arguments()[1], nullptr);
	QObject::connect(updater, &QtAutoUpdater::Updater::updateCheckDone, [updater](bool hasUpdate){
		qDebug() << "Has updates:" << hasUpdate
				 << "\nState:" << updater->state()
				 << "\nError string:" << updater->errorString()
				 << "\nExtended error log:\n" << updater->errorString();
		qDebug() << "Updates:" << updater->updateInfo();
		if(hasUpdate)
			updater->runUpdaterOnExit(QtAutoUpdater::Updater::PassiveUpdateArguments);
		qApp->quit();
	});

	updater->checkForUpdates();
	return a.exec();
}
