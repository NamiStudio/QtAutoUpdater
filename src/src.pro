TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += autoupdatercore \
	plugins \
	autoupdatergui

autoupdatergui.depends += autoupdatercore

docTarget.target = doxygen
QMAKE_EXTRA_TARGETS += docTarget
