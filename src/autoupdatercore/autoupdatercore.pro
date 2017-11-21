TARGET = QtAutoUpdaterCore

QT = core

HEADERS += \
	adminauthoriser.h \
	updater_p.h \
	updater.h \
	simplescheduler_p.h \
	qtautoupdatercore_global.h \
	updaterplugin.h \
	updatebackend.h

SOURCES += \
	simplescheduler.cpp \
	updater.cpp \
	updaterplugin.cpp \
	updatebackend.cpp

MODULE_PLUGIN_TYPES = updaters

load(qt_module)

win32 {
	QMAKE_TARGET_PRODUCT = "QtAutoUpdaterCore"
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_COPYRIGHT = "Felix Barz"
} else:mac {
	QMAKE_TARGET_BUNDLE_PREFIX = "de.skycoder42."
}

QPMX_EXTRA_OPTIONS += --stderr
!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
