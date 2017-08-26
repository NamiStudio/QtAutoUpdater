#ifndef QTAUTOUPDATER_UPDATERPLUGIN_H
#define QTAUTOUPDATER_UPDATERPLUGIN_H

#include "QtAutoUpdaterCore/qtautoupdatercore_global.h"
#include "QtAutoUpdaterCore/updatebackend.h"

#include <QtCore/qobject.h>

namespace QtAutoUpdater {

//! The plugin interface for custom updater backend plugins
class Q_AUTOUPDATERCORE_EXPORT UpdaterPlugin : public QObject
{
	Q_OBJECT

public:
	//! Constructor
	explicit UpdaterPlugin(QObject *parent = nullptr);

	//! Called by the updater to create a backend instance
	virtual UpdateBackend *createInstance(const QByteArray &type, const QString &path, QObject *parent) = 0;
};

#define UpdaterPlugin_iid "de.skycoder42.qtautoupdater.core.UpdaterPlugin"

}

#endif // QTAUTOUPDATER_UPDATERPLUGIN_H
