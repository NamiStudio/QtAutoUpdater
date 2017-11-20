#include "qtifwupdaterplugin.h"
#include "qtifwupdaterbackend.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

using namespace QtAutoUpdater;

QtIfwUpdaterPlugin::QtIfwUpdaterPlugin(QObject *parent) :
	UpdaterPlugin(parent)
{}

UpdateBackend *QtIfwUpdaterPlugin::createInstance(const QByteArray &type, const QString &path, QObject *parent)
{
	if(type == "qtifw") {
		QFileInfo fileInfo(QCoreApplication::applicationDirPath(), QtIfwUpdaterBackend::toSystemExe(path));
		if(fileInfo.exists())
			return new QtIfwUpdaterBackend(fileInfo, parent);
		else
			return nullptr;
	} else
		return nullptr;
}
