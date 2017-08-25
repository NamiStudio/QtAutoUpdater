#ifndef QTAUTOUPDATER_PLUGINLOADER_P_H
#define QTAUTOUPDATER_PLUGINLOADER_P_H

#include "qtautoupdatercore_global.h"
#include "updatebackend.h"

#include <QtCore/QObject>
#include <QtCore/QPluginLoader>

namespace QtAutoUpdater {

class Q_AUTOUPDATERCORE_EXPORT PluginLoader : public QObject
{
	Q_OBJECT

public:
	PluginLoader();

	static PluginLoader *instance();

	QByteArrayList listTypes() const;
	UpdateBackend *getBackend(const QByteArray &type, const QString &path, QObject *parent);

private:
	QHash<QByteArray, QPluginLoader*> plugins;
};

}

#endif // QTAUTOUPDATER_PLUGINLOADER_P_H
