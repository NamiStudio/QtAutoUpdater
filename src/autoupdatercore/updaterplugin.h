#ifndef QTAUTOUPDATER_UPDATERPLUGIN_H
#define QTAUTOUPDATER_UPDATERPLUGIN_H

#include "QtAutoUpdaterCore/qtautoupdatercore_global.h"
#include "QtAutoUpdaterCore/updatebackend.h"

#include <QtCore/qobject.h>

namespace QtAutoUpdater {

//! The plugin interface for custom updater backend plugins
class Q_AUTOUPDATERCORE_EXPORT UpdaterPlugin
{
public:
	virtual inline ~UpdaterPlugin() = default;

	//! Called by the updater to create a backend instance
	virtual UpdateBackend *createInstance(const QString &type, const QString &path, QObject *parent) = 0;
};

}

#define QtAutoUpdater_UpdaterPlugin_iid "de.skycoder42.qtautoupdater.core.UpdaterPlugin"

Q_DECLARE_INTERFACE(QtAutoUpdater::UpdaterPlugin, QtAutoUpdater_UpdaterPlugin_iid)

#endif // QTAUTOUPDATER_UPDATERPLUGIN_H
