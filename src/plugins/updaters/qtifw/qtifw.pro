TARGET  = qtifw

QT += autoupdatercore_private
QT -= gui

HEADERS += \
		qtifwupdaterplugin.h \
	qtifwupdaterbackend.h
SOURCES += \
		qtifwupdaterplugin.cpp \
	qtifwupdaterbackend.cpp

DISTFILES += qtifw.json

PLUGIN_TYPE = updaters
PLUGIN_EXTENDS = autoupdatercore
PLUGIN_CLASS_NAME = QtAutoUpdater::QtIfwUpdaterPlugin
load(qt_plugin)
