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
	class Q_AUTOUPDATERCORE_EXPORT NoUpdatesXmlException : public QException {
	public:
		const char *what() const noexcept override;

		void raise() const override;
		QException *clone() const override;
	};

	class Q_AUTOUPDATERCORE_EXPORT InvalidXmlException : public QException {
	public:
		const char *what() const noexcept override;

		void raise() const override;
		QException *clone() const override;
	};

	Updater *q;
	UpdateBackend *backend;

	QString toolPath;
	QList<Updater::UpdateInfo> updateInfos;
	bool normalExit;
	QString lastErrorString;

	bool running;

	SimpleScheduler *scheduler;

	bool runOnExit;
	QStringList runArguments;
	QScopedPointer<AdminAuthoriser> adminAuth;

	UpdaterPrivate(Updater *q_ptr);
	~UpdaterPrivate();

	bool startUpdateCheck();
	void stopUpdateCheck(int delay, bool async);

public Q_SLOTS:
	void updateCheckCompleted(const QList<Updater::UpdateInfo> &updates);
	void updateCheckFailed(const QString &errorString);

	void appAboutToExit();
};

}

Q_AUTOUPDATERCORE_EXPORT Q_DECLARE_LOGGING_CATEGORY(logQtAutoUpdater)

#endif // QTAUTOUPDATER_UPDATER_P_H
