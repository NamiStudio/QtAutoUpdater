TARGET = QtAutoUpdaterCore

QT = core

HEADERS += \
	adminauthoriser.h \
	updater_p.h \
	updater.h \
	simplescheduler_p.h \
	qtautoupdatercore_global.h \
	updaterplugin.h \
	updatebackend.h \
	pluginloader_p.h

SOURCES += \
	simplescheduler.cpp \
	updater.cpp \
	updaterplugin.cpp \
	updatebackend.cpp \
	pluginloader.cpp

MODULE_PLUGIN_TYPES = updaters

load(qt_module)

win32 {
	QMAKE_TARGET_PRODUCT = "QtAutoUpdaterCore"
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_COPYRIGHT = "Felix Barz"
} else:mac {
	QMAKE_TARGET_BUNDLE_PREFIX = "de.skycoder42."
}
