#include "qtifwupdaterplugin.h"
#include "qtifwupdaterbackend.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QtIfwUpdaterPlugin::QtIfwUpdaterPlugin(QObject *parent) :
	QObject(parent),
	UpdaterPlugin()
{}

QtAutoUpdater::UpdateBackend *QtIfwUpdaterPlugin::createInstance(const QByteArray &type, const QString &path, QObject *parent)
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
