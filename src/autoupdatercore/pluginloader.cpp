#include "pluginloader_p.h"
#include "updater_p.h"
#include "updaterplugin.h"

#include <QtCore/QDir>
#include <QtCore/QGlobalStatic>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QLibraryInfo>

using namespace QtAutoUpdater;

Q_GLOBAL_STATIC(PluginLoader, loader)

PluginLoader::PluginLoader() :
	QObject(nullptr),
	plugins()
{
	//find all available plugins
	QDir pluginDir = QLibraryInfo::location(QLibraryInfo::PluginsPath);
	if(!pluginDir.cd(QStringLiteral("updater")))
		return;

	foreach(auto info, pluginDir.entryInfoList(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot)) {
		auto plugin = new QPluginLoader(info.absoluteFilePath(), this);
		auto metaData = plugin->metaData();
		if(metaData[QStringLiteral("IID")].toString() != QStringLiteral(UpdaterPlugin_iid)) {
			qCWarning(logQtAutoUpdater) << "File" << info.absoluteFilePath() << "is not an updater plugin";
			plugin->deleteLater();
			continue;
		}

		auto data = metaData[QStringLiteral("MetaData")].toObject();
		auto keys = data[QStringLiteral("Keys")].toArray();
		if(keys.isEmpty()) {
			qCWarning(logQtAutoUpdater) << "Plugin" << info.absoluteFilePath() << "is does not provide any updaters";
			plugin->deleteLater();
			continue;
		}

		foreach(auto key, keys)
			plugins.insert(key.toString().toUtf8(), plugin);
	}
}

PluginLoader *PluginLoader::instance()
{
	return loader;
}

UpdateBackend *PluginLoader::getBackend(const QByteArray &type, const QString &path, QObject *parent)
{
	auto loader = plugins.value(type, nullptr);
	if(!loader) {
		qCCritical(logQtAutoUpdater) << "Unable to find a plugin for updater type" << type;
		return nullptr;
	}

	auto instance = qobject_cast<UpdaterPlugin*>(loader->instance());
	if(!instance) {
		qCCritical(logQtAutoUpdater) << "Unable to load plugin for updater type" << type;
		return nullptr;
	}

	return instance->createInstance(type, path, parent);
}
