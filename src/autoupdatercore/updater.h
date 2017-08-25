#ifndef QTAUTOUPDATER_UPDATER_H
#define QTAUTOUPDATER_UPDATER_H

#include "QtAutoUpdaterCore/qtautoupdatercore_global.h"
#include "QtAutoUpdaterCore/adminauthoriser.h"

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qversionnumber.h>
#include <QtCore/qscopedpointer.h>

namespace QtAutoUpdater
{

class UpdaterPrivate;
//! The main updater. Can check for updates and run the maintenancetool as updater
class Q_AUTOUPDATERCORE_EXPORT Updater : public QObject
{
	Q_OBJECT

	//! Holds the path of the attached maintenancetool
	Q_PROPERTY(QString maintenanceToolPath READ maintenanceToolPath CONSTANT FINAL)
	Q_PROPERTY(UpdaterState state READ state NOTIFY stateChanged)
	//! Holds extended information about the last update check
	Q_PROPERTY(QList<UpdateInfo> updateInfo READ updateInfo NOTIFY updateInfoChanged)

public:
	enum UpdaterState {
		NoUpdates,
		Running,
		HasUpdates,
		HasError
	};
	Q_ENUM(UpdaterState)

	//! Provides information about updates for components
	struct Q_AUTOUPDATERCORE_EXPORT UpdateInfo
	{
		//! The name of the component that has an update
		QString name;
		//! The new version for that compontent
		QVersionNumber version;
		//! The update download size (in Bytes)
		quint64 size;

		//! Default Constructor
		UpdateInfo();
		//! Copy Constructor
		UpdateInfo(const UpdateInfo &other);
		//! Constructor that takes name, version and size
		UpdateInfo(QString name, QVersionNumber version, quint64 size);
	};

	static const QStringList NormalUpdateArguments;
	static const QStringList PassiveUpdateArguments;
	static const QStringList HiddenUpdateArguments;

	//! Default constructor
	explicit Updater(QObject *parent = nullptr);
	//! Constructer with an explicitly set path
	explicit Updater(const QString &maintenanceToolPath, QObject *parent = nullptr);
	explicit Updater(const QString &maintenanceToolPath, const QByteArray &type, QObject *parent = nullptr);
	//! Destroyes the updater and kills the update check (if running)
	~Updater();

	bool isValid() const;
	QByteArray updaterType() const;
	//! Returns the mainetancetools error string of the last update
	QString errorString() const;
	QByteArray extendedErrorLog() const;

	//! Returns `true` if the maintenancetool will be started on exit
	bool willRunOnExit() const;

	//! readAcFn{Updater::maintenanceToolPath}
	QString maintenanceToolPath() const;
	UpdaterState state() const;
	//! readAcFn{Updater::updateInfo}
	QList<UpdateInfo> updateInfo() const;

	static QByteArrayList supportedUpdaterTypes();

	QT_DEPRECATED bool exitedNormally() const;
	QT_DEPRECATED QByteArray errorLog() const;
	QT_DEPRECATED bool isRunning() const;

public Q_SLOTS:
	//! Starts checking for updates
	bool checkForUpdates();
	//! Aborts checking for updates
	void abortUpdateCheck(int maxDelay = 5000);

	//! Schedules an update after a specific delay, optionally repeated
	int scheduleUpdate(int delaySeconds, bool repeated = false);
	//! Schedules an update for a specific timepoint
	int scheduleUpdate(const QDateTime &when);
	//! Cancels the scheduled update with taskId
	void cancelScheduledUpdate(int taskId);

	//! Runs the maintenancetool as updater on exit, using the given admin authorisation
	void runUpdaterOnExit(AdminAuthoriser *authoriser = nullptr);
	//! Runs the maintenancetool as updater on exit, using the given arguments and admin authorisation
	void runUpdaterOnExit(const QStringList &arguments, AdminAuthoriser *authoriser = nullptr);
	//! The updater will not run the maintenancetool on exit anymore
	void cancelExitRun();

Q_SIGNALS:
	void updateCheckDone(bool hasUpdates);
	void stateChanged(UpdaterState state);
	//! notifyAcFn{Updater::updateInfo}
	void updateInfoChanged(QList<QtAutoUpdater::Updater::UpdateInfo> updateInfo);

	QT_DEPRECATED void checkUpdatesDone(bool hasUpdates, bool hasError);
	QT_DEPRECATED void runningChanged(bool running);

private:
	QScopedPointer<UpdaterPrivate> d;
};

}

Q_DECLARE_METATYPE(QtAutoUpdater::Updater::UpdateInfo)

QDebug Q_AUTOUPDATERCORE_EXPORT &operator<<(QDebug &debug, const QtAutoUpdater::Updater::UpdateInfo &info);

#endif // QTAUTOUPDATER_UPDATER_H
