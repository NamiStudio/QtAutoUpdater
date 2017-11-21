#ifndef QTAUTOUPDATER_UPDATER_P_H
#define QTAUTOUPDATER_UPDATER_P_H

#include "qtautoupdatercore_global.h"
#include "updater.h"
#include "simplescheduler_p.h"
#include "updatebackend.h"

#include <QtCore/QProcess>
#include <QtCore/QLoggingCategory>
#include <QtCore/QException>

namespace QtAutoUpdater
{

class Q_AUTOUPDATERCORE_EXPORT UpdaterPrivate : public QObject
{
public:
	static const QString DefaultToolPath;
	static const QString DefaultUpdaterType;

	Updater *q;
	UpdateBackend *backend;

	QString toolPath;
	QString type;
	Updater::UpdaterState state;
	QList<Updater::UpdateInfo> updateInfos;
	QString lastErrorString;

	SimpleScheduler *scheduler;

	bool runOnExit;
	QStringList runArguments;
	QScopedPointer<AdminAuthoriser> adminAuth;

	UpdaterPrivate(const QString &maintenanceToolPath, const QString &type, Updater *q_ptr);

public Q_SLOTS:
	void updateCheckCompleted(const QList<Updater::UpdateInfo> &updates);
	void updateCheckFailed(const QString &errorString);

	void appAboutToExit();
};

}

Q_AUTOUPDATERCORE_EXPORT Q_DECLARE_LOGGING_CATEGORY(logQtAutoUpdater)

#endif // QTAUTOUPDATER_UPDATER_P_H
