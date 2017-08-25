#include "updatebackend.h"
using namespace QtAutoUpdater;

UpdateBackend::UpdateBackend(QObject *parent) :
	QObject(parent)
{}

QByteArray UpdateBackend::extendedErrorLog() const
{
	return QByteArray();
}
