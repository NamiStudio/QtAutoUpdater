#ifndef QTAUTOUPDATER_QTIFWUPDATERBACKEND_H
#define QTAUTOUPDATER_QTIFWUPDATERBACKEND_H

#include "QtAutoUpdaterCore/updatebackend.h"

namespace QtAutoUpdater {

class QtIfwUpdaterBackend : public UpdateBackend
{
	Q_OBJECT

public:
	explicit QtIfwUpdaterBackend(const QString &maintenanceToolPath, QObject *parent = nullptr);

	void startUpdateTool(const QVariantList &arguments) override;

public slots:
	void startUpdateCheck() override;
	void cancelUpdateCheck(int maxDelay) override;

private:
	const QString maintenanceToolPath;
};

}

#endif // QTAUTOUPDATER_QTIFWUPDATERBACKEND_H
