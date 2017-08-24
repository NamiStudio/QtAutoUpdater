#ifndef QTAUTOUPDATER_UPDATERPLUGIN_H
#define QTAUTOUPDATER_UPDATERPLUGIN_H

#include "QtAutoUpdaterCore/qtautoupdatercore_global.h"
#include "QtAutoUpdaterCore/updatebackend.h"

#include <QtCore/qobject.h>

namespace QtAutoUpdater {

class Q_AUTOUPDATERCORE_EXPORT UpdaterPlugin : public QObject
{
	Q_OBJECT

public:
	explicit UpdaterPlugin(QObject *parent = nullptr);

	virtual UpdateBackend *createInstance(const QByteArray &type, const QString &path, QObject *parent) = 0;
};

#define UpdaterPlugin_iid "de.skycoder42.qtautoupdater.core.UpdaterPlugin"

}

#endif // QTAUTOUPDATER_UPDATERPLUGIN_H
