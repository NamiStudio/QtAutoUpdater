#ifndef QTAUTOUPDATER_UPDATEBACKEND_H
#define QTAUTOUPDATER_UPDATEBACKEND_H

#include "QtAutoUpdaterCore/qtautoupdatercore_global.h"
#include "QtAutoUpdaterCore/updater.h"

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

namespace QtAutoUpdater {

//! The interface to provide custom updater backends
class Q_AUTOUPDATERCORE_EXPORT UpdateBackend : public QObject
{
	Q_OBJECT

public:
	//! Constructor
	explicit UpdateBackend(QObject *parent = nullptr);

	//! Is called by the updater to start the update tool to perform the actual update
	virtual bool startUpdateTool(const QStringList &arguments, AdminAuthoriser *authoriser) = 0;

	//! Returns an extended error log
	virtual QByteArray extendedErrorLog() const;

public Q_SLOTS:
	//! Is called to start the check for updates
	virtual void startUpdateCheck() = 0;
	//! Is called to cancle the update check within the given delay
	virtual void cancelUpdateCheck(int maxDelay) = 0;

Q_SIGNALS:
	//! Must be emitted to signale that the update check finished without errors
	void updateCheckCompleted(const QList<Updater::UpdateInfo> &updates);
	//! Must be emitted to signale that the update check failed with errors
	void updateCheckFailed(const QString &errorString);
};

}

#endif // QTAUTOUPDATER_UPDATEBACKEND_H
