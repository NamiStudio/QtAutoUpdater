#include "qtifwupdaterbackend.h"
#include "QtAutoUpdaterCore/private/updater_p.h"

#include <QtCore/QTimer>
#include <QtCore/QXmlStreamReader>

using namespace QtAutoUpdater;

QtIfwUpdaterBackend::QtIfwUpdaterBackend(const QFileInfo &toolInfo, QObject *parent) :
	UpdateBackend(parent),
	toolInfo(toolInfo),
	process(nullptr),
	lastErrorLog()
{}

bool QtIfwUpdaterBackend::startUpdateTool(const QStringList &arguments, AdminAuthoriser *authoriser)
{
	if(authoriser && !authoriser->hasAdminRights())
		return authoriser->executeAsAdmin(toolInfo.absoluteFilePath(), arguments);
	else
		return QProcess::startDetached(toolInfo.absoluteFilePath(), arguments);
}

QByteArray QtIfwUpdaterBackend::extendedErrorLog() const
{
	return lastErrorLog;
}

const QString QtIfwUpdaterBackend::toSystemExe(QString basePath)
{
#if defined(Q_OS_WIN32)
	if(!basePath.endsWith(QStringLiteral(".exe")))
		return basePath + QStringLiteral(".exe");
	else
		return basePath;
#elif defined(Q_OS_OSX)
	if(basePath.endsWith(QStringLiteral(".app")))
		basePath.truncate(basePath.lastIndexOf(QStringLiteral(".")));
	return basePath + QStringLiteral(".app/Contents/MacOS/") + QFileInfo(basePath).fileName();
#elif defined(Q_OS_UNIX)
	return basePath;
#endif
}

void QtIfwUpdaterBackend::startUpdateCheck()
{
	//should not be possible, but to be shure
	if(process) {
		process->deleteLater();
		process = nullptr;
	}

	process = new QProcess(this);
	process->setProgram(toolInfo.absoluteFilePath());
	process->setArguments({QStringLiteral("--checkupdates")});

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			this, &QtIfwUpdaterBackend::updaterReady, Qt::QueuedConnection);
	connect(process, &QProcess::errorOccurred,
			this, &QtIfwUpdaterBackend::updaterError, Qt::QueuedConnection);

	process->start(QIODevice::ReadOnly);
}

void QtIfwUpdaterBackend::cancelUpdateCheck(int maxDelay)
{
	if(process && process->state() != QProcess::NotRunning) {
		if(maxDelay > 0) {
			process->terminate();
			QTimer::singleShot(maxDelay, this, [this](){
				cancelUpdateCheck(0);
			});
		} else {
			process->kill();
			process->waitForFinished(100);
		}
	}
}

void QtIfwUpdaterBackend::updaterReady(int exitCode, QProcess::ExitStatus exitStatus)
{
	if(!process)
		return;

	if(exitStatus == QProcess::NormalExit) {
		lastErrorLog = process->readAllStandardError();
		const auto updateOut = process->readAllStandardOutput();
		process->deleteLater();
		process = nullptr;

		try {
			auto updateInfos = parseResult(updateOut);
			emit updateCheckCompleted(updateInfos);
		} catch (NoUpdatesXmlException &) {
			emit updateCheckCompleted({});
		} catch (InvalidXmlException &e) {
			emit updateCheckFailed(QString::fromUtf8(e.what()), exitCode);
		}
	} else
		updaterError(QProcess::Crashed);
}

void QtIfwUpdaterBackend::updaterError(QProcess::ProcessError error)
{
	if(!process)
		return;

	auto errorString = process->errorString();
	process->deleteLater();
	process = nullptr;

	emit updateCheckFailed(errorString, error);
}

QList<Updater::UpdateInfo> QtIfwUpdaterBackend::parseResult(const QByteArray &output)
{
	const auto outString = QString::fromUtf8(output);
	const auto xmlBegin = outString.indexOf(QStringLiteral("<updates>"));
	if(xmlBegin < 0)
		throw NoUpdatesXmlException();
	const auto xmlEnd = outString.indexOf(QStringLiteral("</updates>"), xmlBegin);
	if(xmlEnd < 0)
		throw NoUpdatesXmlException();

	QList<Updater::UpdateInfo> updates;
	QXmlStreamReader reader(outString.mid(xmlBegin, (xmlEnd + 10) - xmlBegin));

	reader.readNextStartElement();
	//should always work because it was search for
	Q_ASSERT(reader.name() == QStringLiteral("updates"));

	while(reader.readNextStartElement()) {
		if(reader.name() != QStringLiteral("update"))
			throw InvalidXmlException();

		auto ok = false;
		Updater::UpdateInfo info(reader.attributes().value(QStringLiteral("name")).toString(),
								 QVersionNumber::fromString(reader.attributes().value(QStringLiteral("version")).toString()),
								 reader.attributes().value(QStringLiteral("size")).toULongLong(&ok));

		if(info.name.isEmpty() || info.version.isNull() || !ok)
			throw InvalidXmlException();
		if(reader.readNextStartElement())
			throw InvalidXmlException();

		updates.append(info);
	}

	if(reader.hasError()) {
		qCWarning(logQtAutoUpdater) << "XML-reader-error:" << reader.errorString();
		throw InvalidXmlException();
	}

	return updates;
}



const char *QtIfwUpdaterBackend::NoUpdatesXmlException::what() const noexcept
{
	return "The <updates> node could not be found";
}

void QtIfwUpdaterBackend::NoUpdatesXmlException::raise() const
{
	throw *this;
}

QException *QtIfwUpdaterBackend::NoUpdatesXmlException::clone() const
{
	return new NoUpdatesXmlException();
}

const char *QtIfwUpdaterBackend::InvalidXmlException::what() const noexcept
{
	return "The found XML-part is not of a valid updates-XML-format";
}

void QtIfwUpdaterBackend::InvalidXmlException::raise() const
{
	throw *this;
}

QException *QtIfwUpdaterBackend::InvalidXmlException::clone() const
{
	return new InvalidXmlException();
}
