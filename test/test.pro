include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG += thread

SOURCES += \
        main.cpp \
        tst_qrange.cpp
	
HEADERS += \
	qrange.h
