#include "qtifwupdaterplugin.h"
#include "qtifwupdaterbackend.h"
using namespace QtAutoUpdater;

QtIfwUpdaterPlugin::QtIfwUpdaterPlugin(QObject *parent) :
	UpdaterPlugin(parent)
{}

UpdateBackend *QtIfwUpdaterPlugin::createInstance(const QByteArray &type, const QString &path, QObject *parent)
{
	if(type == "qtifw")
		return new QtIfwUpdaterBackend(path, parent);
	else
		return nullptr;
}
