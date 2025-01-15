QT -= gui

TARGET = $$qtLibraryTarget(QRange)
TEMPLATE = lib
CONFIG += lib

CONFIG += c++17

HEADERS += \
	qrange.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
