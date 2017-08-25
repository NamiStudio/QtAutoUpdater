#ifndef QTAUTOUPDATER_QTIFWUPDATERBACKEND_H
#define QTAUTOUPDATER_QTIFWUPDATERBACKEND_H

#include "QtAutoUpdaterCore/updatebackend.h"

#include <QtCore/QException>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>

namespace QtAutoUpdater {

class QtIfwUpdaterBackend : public UpdateBackend
{
	Q_OBJECT

public:
	class NoUpdatesXmlException : public QException {
	public:
		const char *what() const noexcept override;

		void raise() const override;
		QException *clone() const override;
	};

	class InvalidXmlException : public QException {
	public:
		const char *what() const noexcept override;

		void raise() const override;
		QException *clone() const override;
	};

	explicit QtIfwUpdaterBackend(const QFileInfo &toolInfo, QObject *parent = nullptr);

	bool startUpdateTool(const QStringList &arguments, AdminAuthoriser *authoriser) override;
	QByteArray extendedErrorLog() const override;

	static const QString toSystemExe(QString basePath);

public Q_SLOTS:
	void startUpdateCheck() override;
	void cancelUpdateCheck(int maxDelay) override;

private Q_SLOTS:
	void updaterReady(int exitCode, QProcess::ExitStatus exitStatus);
	void updaterError(QProcess::ProcessError error);

private:
	const QFileInfo &toolInfo;
	QProcess *process;
	QByteArray lastErrorLog;

	static QList<Updater::UpdateInfo> parseResult(const QByteArray &output);
};

}

#endif // QTAUTOUPDATER_QTIFWUPDATERBACKEND_H
