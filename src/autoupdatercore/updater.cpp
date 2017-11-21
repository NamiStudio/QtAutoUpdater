#include "updater.h"
#include "updater_p.h"
#include "updaterplugin.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include "qpluginfactory.h"

using namespace QtAutoUpdater;

Q_LOGGING_CATEGORY(logQtAutoUpdater, "QtAutoUpdater")

const QStringList Updater::NormalUpdateArguments = {QStringLiteral("--updater")};
const QStringList Updater::PassiveUpdateArguments = {QStringLiteral("--updater"), QStringLiteral("skipPrompt=true")};
const QStringList Updater::HiddenUpdateArguments = {QStringLiteral("--silentUpdate")};

typedef QPluginObjectFactory<UpdaterPlugin, UpdateBackend> Factory;
Q_GLOBAL_STATIC_WITH_ARGS(Factory, factory, (QLatin1String("updaters")))

Updater::Updater(QObject *parent) :
	QObject(parent),
	d(new UpdaterPrivate(UpdaterPrivate::DefaultToolPath, UpdaterPrivate::DefaultUpdaterType, this))
{}

Updater::Updater(const QString &maintenanceToolPath, QObject *parent)  :
	QObject(parent),
	d(new UpdaterPrivate(maintenanceToolPath, UpdaterPrivate::DefaultUpdaterType, this))
{}

Updater::Updater(const QString &maintenanceToolPath, const QString &type, QObject *parent) :
	QObject(parent),
	d(new UpdaterPrivate(maintenanceToolPath, type, this))
{}

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

QString Updater::updaterType() const
{
	return d->type;
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

QStringList Updater::supportedUpdaterTypes()
{
	return factory->allKeys();
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

#ifdef Q_OS_OSX
const QString UpdaterPrivate::DefaultToolPath = QStringLiteral("../../maintenancetool");
#else
const QString UpdaterPrivate::DefaultToolPath = QStringLiteral("./maintenancetool");
#endif
const QString UpdaterPrivate::DefaultUpdaterType = QStringLiteral("qtifw");

UpdaterPrivate::UpdaterPrivate(const QString &maintenanceToolPath, const QString &type, Updater *q_ptr) :
	QObject(nullptr),
	q(q_ptr),
	backend(nullptr),
	toolPath(maintenanceToolPath),
	type(type),
	state(Updater::NoUpdates),
	updateInfos(),
	lastErrorString(),
	scheduler(new SimpleScheduler(this)),
	runOnExit(false),
	runArguments(),
	adminAuth(nullptr)
{
	backend = factory->createInstance(type, maintenanceToolPath, this);
	if(!backend) {
		state = Updater::HasError;
		return;
	}

	connect(backend, &UpdateBackend::updateCheckCompleted,
			this, &UpdaterPrivate::updateCheckCompleted);
	connect(backend, &UpdateBackend::updateCheckFailed,
			this, &UpdaterPrivate::updateCheckFailed);

	connect(qApp, &QCoreApplication::aboutToQuit,
			this, &UpdaterPrivate::appAboutToExit,
			Qt::DirectConnection);
	connect(scheduler, &SimpleScheduler::scheduleTriggered,
			q, &Updater::checkForUpdates);

	//MAJOR remove deprecated
	QT_WARNING_PUSH
	QT_WARNING_DISABLE_DEPRECATED
	connect(q, &Updater::updateCheckDone, q, [this](bool hasUpdates){
		emit q->checkUpdatesDone(hasUpdates, state == Updater::HasError);
	});
	connect(q, &Updater::stateChanged, q, [this](Updater::UpdaterState state) {
		emit q->runningChanged(state == Updater::Running);
	});
	QT_WARNING_POP
}

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
