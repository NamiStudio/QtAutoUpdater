#ifndef QTAUTOUPDATER_QTIFWUPDATERPLUGIN_H
#define QTAUTOUPDATER_QTIFWUPDATERPLUGIN_H

#include "QtAutoUpdaterCore/updaterplugin.h"

class QtIfwUpdaterPlugin : public QObject, public QtAutoUpdater::UpdaterPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtAutoUpdater_UpdaterPlugin_iid FILE "qtifw.json")
	Q_INTERFACES(QtAutoUpdater::UpdaterPlugin)

public:
	QtIfwUpdaterPlugin(QObject *parent = nullptr);

	QtAutoUpdater::UpdateBackend *createInstance(const QString &type, const QString &path, QObject *parent) override;
};

#endif // QTAUTOUPDATER_QTIFWUPDATERPLUGIN_H
