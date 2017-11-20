#include "updater.h"
#include "updater_p.h"
#include "pluginloader_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

using namespace QtAutoUpdater;

Q_LOGGING_CATEGORY(logQtAutoUpdater, "QtAutoUpdater")

#ifdef Q_OS_OSX
#define DEFAULT_TOOL_PATH QStringLiteral("../../maintenancetool")
#else
#define DEFAULT_TOOL_PATH QStringLiteral("./maintenancetool")
#endif

const QStringList Updater::NormalUpdateArguments = {QStringLiteral("--updater")};
const QStringList Updater::PassiveUpdateArguments = {QStringLiteral("--updater"), QStringLiteral("skipPrompt=true")};
const QStringList Updater::HiddenUpdateArguments = {QStringLiteral("--silentUpdate")};

Updater::Updater(QObject *parent) :
	Updater(DEFAULT_TOOL_PATH, "qtifw", parent)
{}

Updater::Updater(const QString &maintenanceToolPath, QObject *parent) :
	Updater(maintenanceToolPath, "qtifw", parent)
{}

Updater::Updater(const QString &maintenanceToolPath, const QByteArray &type, QObject *parent) :
	QObject(parent),
	d(new UpdaterPrivate(this))
{
	d->toolPath = maintenanceToolPath;
	d->backend = PluginLoader::instance()->getBackend(type, maintenanceToolPath, this);

	if(d->backend) {
		connect(d->backend, &UpdateBackend::updateCheckCompleted,
				d.data(), &UpdaterPrivate::updateCheckCompleted);
		connect(d->backend, &UpdateBackend::updateCheckFailed,
				d.data(), &UpdaterPrivate::updateCheckFailed);
	}

	connect(qApp, &QCoreApplication::aboutToQuit,
			d.data(), &UpdaterPrivate::appAboutToExit,
			Qt::DirectConnection);
	connect(d->scheduler, &SimpleScheduler::scheduleTriggered,
			this, &Updater::checkForUpdates);

	//MAJOR remove deprecated
	QT_WARNING_PUSH
	QT_WARNING_DISABLE_DEPRECATED
	connect(this, &Updater::updateCheckDone, this, [this](bool hasUpdates){
		emit checkUpdatesDone(hasUpdates, state() == HasError);
	});
	connect(this, &Updater::stateChanged, this, [this](UpdaterState state) {
		emit runningChanged(state == Running);
	});
	QT_WARNING_POP
}

Updater::~Updater()
{
	if(d->runOnExit)
		qCWarning(logQtAutoUpdater) << "Updater destroyed with run on exit active before the application quit";

	if(d->state == Running && d->backend)
		d->backend->cancelUpdateCheck(0);
}

bool Updater::isValid() const
{
	return d->backend;
}

QString Updater::errorString() const
{
	return d->lastErrorString;
}

QByteArray Updater::extendedErrorLog() const
{
	if(d->backend)
		return d->backend->extendedErrorLog();
	else
		return QByteArray();
}

bool Updater::willRunOnExit() const
{
	return d->runOnExit;
}

QString Updater::maintenanceToolPath() const
{
	return d->toolPath;
}

Updater::UpdaterState Updater::state() const
{
	return d->state;
}

QList<Updater::UpdateInfo> Updater::updateInfo() const
{
	return d->updateInfos;
}

bool Updater::exitedNormally() const
{
	return state() != HasError;
}

QByteArray Updater::errorLog() const
{
	return extendedErrorLog();
}

bool Updater::isRunning() const
{
	return d->state == Running;
}

bool Updater::checkForUpdates()
{
	if(d->state == Running || !d->backend)
		return false;
	else {
		d->updateInfos.clear();
		d->state = Running;
		d->lastErrorString.clear();

		emit stateChanged(d->state);
		emit updateInfoChanged(d->updateInfos);

		d->backend->startUpdateCheck();

		return true;
	}
}

void Updater::abortUpdateCheck(int maxDelay)
{
	if(d->backend && d->state == Running)
		d->backend->cancelUpdateCheck(maxDelay);
}

int Updater::scheduleUpdate(int delaySeconds, bool repeated)
{
	if((((qint64)delaySeconds) * 1000ll) > (qint64)INT_MAX) {
		qCWarning(logQtAutoUpdater) << "delaySeconds to big to be converted to msecs";
		return 0;
	}
	return d->scheduler->startSchedule(delaySeconds * 1000, repeated);
}

int Updater::scheduleUpdate(const QDateTime &when)
{
	return d->scheduler->startSchedule(when);
}

void Updater::cancelScheduledUpdate(int taskId)
{
	d->scheduler->cancelSchedule(taskId);
}

void Updater::runUpdaterOnExit(AdminAuthoriser *authoriser)
{
	runUpdaterOnExit(NormalUpdateArguments, authoriser);
}

void Updater::runUpdaterOnExit(const QStringList &arguments, AdminAuthoriser *authoriser)
{
	d->runOnExit = true;
	d->runArguments = arguments;
	d->adminAuth.reset(authoriser);
}

void Updater::cancelExitRun()
{
	d->runOnExit = false;
	d->adminAuth.reset();
}



Updater::UpdateInfo::UpdateInfo() :
	name(),
	version(),
	size(0ull)
{}

Updater::UpdateInfo::UpdateInfo(const Updater::UpdateInfo &other) :
	name(other.name),
	version(other.version),
	size(other.size)
{}

Updater::UpdateInfo::UpdateInfo(QString name, QVersionNumber version, quint64 size) :
	name(name),
	version(version),
	size(size)
{}

QDebug &operator<<(QDebug &debug, const Updater::UpdateInfo &info)
{
	QDebugStateSaver state(debug);
	Q_UNUSED(state);

	debug.noquote() << QStringLiteral("{Name: \"%1\"; Version: %2; Size: %3}")
					   .arg(info.name)
					   .arg(info.version.toString())
					   .arg(info.size);
	return debug;
}

// ------------- PRIVATE IMPLEMENTATION -------------

UpdaterPrivate::UpdaterPrivate(Updater *q_ptr) :
	QObject(nullptr),
	q(q_ptr),
	backend(nullptr),
	toolPath(),
	state(Updater::NoUpdates),
	updateInfos(),
	lastErrorString(),
	scheduler(new SimpleScheduler(this)),
	runOnExit(false),
	runArguments(),
	adminAuth(nullptr)
{}

void UpdaterPrivate::updateCheckCompleted(const QList<Updater::UpdateInfo> &updates)
{
	updateInfos = updates;
	state = updates.isEmpty() ? Updater::NoUpdates : Updater::HasUpdates;
	lastErrorString.clear();
	emit q->stateChanged(state);
	if(!updateInfos.isEmpty())
		emit q->updateInfoChanged(updateInfos);
	emit q->updateCheckDone(state == Updater::HasUpdates);
}

void UpdaterPrivate::updateCheckFailed(const QString &errorString)
{
	state = Updater::HasError;
	lastErrorString = errorString;
	emit q->stateChanged(state);
	emit q->updateCheckDone(false);
}

void UpdaterPrivate::appAboutToExit()
{
	if(runOnExit && backend) {
		if(!backend->startUpdateTool(runArguments, adminAuth.data())) {
			qCWarning(logQtAutoUpdater) << "Unable to start" << toolPath
										<< "with arguments" << runArguments
										<< "as" << (adminAuth ? "admin/root" : "current user");
		}

		runOnExit = false;//prevent warning
	}
}
