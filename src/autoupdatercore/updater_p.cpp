#include "updater.h"
#include "updater_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QTimer>

using namespace QtAutoUpdater;

Q_LOGGING_CATEGORY(logQtAutoUpdater, "QtAutoUpdater")

UpdaterPrivate::UpdaterPrivate(Updater *q_ptr) :
	QObject(nullptr),
	q(q_ptr),
	backend(nullptr),
	toolPath(),
	updateInfos(),
	normalExit(true),
	lastErrorCode(0),
	lastErrorString(),
	running(false),
	scheduler(new SimpleScheduler(this)),
	runOnExit(false),
	runArguments(),
	adminAuth(nullptr)
{
	connect(qApp, &QCoreApplication::aboutToQuit,
			this, &UpdaterPrivate::appAboutToExit,
			Qt::DirectConnection);
	connect(scheduler, &SimpleScheduler::scheduleTriggered,
			this, &UpdaterPrivate::startUpdateCheck);
}

UpdaterPrivate::~UpdaterPrivate()
{
	if(runOnExit)
		qCWarning(logQtAutoUpdater) << "Updater destroyed with run on exit active before the application quit";

	if(running && backend)
		backend->cancelUpdateCheck(0);
}

bool UpdaterPrivate::startUpdateCheck()
{
	if(running || !backend)
		return false;
	else {
		updateInfos.clear();
		normalExit = true;
		lastErrorCode = 0;
		lastErrorString.clear();

		running = true;
		backend->startUpdateCheck();

		emit q->updateInfoChanged(updateInfos);
		emit q->runningChanged(true);
		return true;
	}
}

void UpdaterPrivate::stopUpdateCheck(int delay, bool async)
{
	Q_UNUSED(async); //TODO remove
	if(backend)
		backend->cancelUpdateCheck(delay);
}

void UpdaterPrivate::updateCheckCompleted(const QList<Updater::UpdateInfo> &updates)
{
	running = false;
	normalExit = true;
	lastErrorCode = 0;
	lastErrorString.clear();
	emit q->runningChanged(false);

	updateInfos = updates;
	if(!updateInfos.isEmpty())
		emit q->updateInfoChanged(updateInfos);
	emit q->checkUpdatesDone(!updateInfos.isEmpty(), false);
}

void UpdaterPrivate::updateCheckFailed(const QString &errorString, int errorCode)
{
	normalExit = false;
	lastErrorCode = errorCode;
	lastErrorString = errorString;
	emit q->checkUpdatesDone(false, true);
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



const char *UpdaterPrivate::NoUpdatesXmlException::what() const noexcept
{
	return "The <updates> node could not be found";
}

void UpdaterPrivate::NoUpdatesXmlException::raise() const
{
	throw *this;
}

QException *UpdaterPrivate::NoUpdatesXmlException::clone() const
{
	return new NoUpdatesXmlException();
}

const char *UpdaterPrivate::InvalidXmlException::what() const noexcept
{
	return "The found XML-part is not of a valid updates-XML-format";
}

void UpdaterPrivate::InvalidXmlException::raise() const
{
	throw *this;
}

QException *UpdaterPrivate::InvalidXmlException::clone() const
{
	return new InvalidXmlException();
}
