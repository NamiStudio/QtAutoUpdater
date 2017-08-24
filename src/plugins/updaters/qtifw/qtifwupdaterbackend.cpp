#include "qtifwupdaterbackend.h"
using namespace QtAutoUpdater;

QtIfwUpdaterBackend::QtIfwUpdaterBackend(const QString &maintenanceToolPath, QObject *parent) :
	UpdateBackend(parent),
	maintenanceToolPath(maintenanceToolPath)
{}

void QtIfwUpdaterBackend::startUpdateTool(const QVariantList &arguments)
{
}

void QtIfwUpdaterBackend::startUpdateCheck()
{
}

void QtIfwUpdaterBackend::cancelUpdateCheck(int maxDelay)
{
}
