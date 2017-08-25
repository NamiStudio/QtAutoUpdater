#ifndef QTAUTOUPDATER_UPDATEBACKEND_H
#define QTAUTOUPDATER_UPDATEBACKEND_H

#include "QtAutoUpdaterCore/qtautoupdatercore_global.h"
#include "QtAutoUpdaterCore/updater.h"

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

namespace QtAutoUpdater {

class Q_AUTOUPDATERCORE_EXPORT UpdateBackend : public QObject
{
	Q_OBJECT

public:
	explicit UpdateBackend(QObject *parent = nullptr);

	virtual bool startUpdateTool(const QStringList &arguments, AdminAuthoriser *authoriser) = 0;

	virtual QByteArray extendedErrorLog() const;

public Q_SLOTS:
	virtual void startUpdateCheck() = 0;
	virtual void cancelUpdateCheck(int maxDelay) = 0;

Q_SIGNALS:
	void updateCheckCompleted(const QList<Updater::UpdateInfo> &updates);
	void updateCheckFailed(const QString &errorString);
};

}

#endif // QTAUTOUPDATER_UPDATEBACKEND_H
