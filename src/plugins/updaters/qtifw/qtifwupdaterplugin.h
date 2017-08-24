#ifndef QTAUTOUPDATER_QTIFWUPDATERPLUGIN_H
#define QTAUTOUPDATER_QTIFWUPDATERPLUGIN_H

#include "QtAutoUpdaterCore/updaterplugin.h"

namespace QtAutoUpdater {

class QtIfwUpdaterPlugin : public UpdaterPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID UpdaterPlugin_iid FILE "qtifw.json")

public:
	QtIfwUpdaterPlugin(QObject *parent = nullptr);

	UpdateBackend *createInstance(const QByteArray &type, const QString &path, QObject *parent) override;
};

}

#endif // QTAUTOUPDATER_QTIFWUPDATERPLUGIN_H
