TEMPLATE = app

QT += core
QT -= gui

CONFIG += c++11 console warning_clean exceptions
CONFIG -= app_bundle

TARGET = repkg
VERSION = 1.0.0

RC_ICONS += ./icons/repkg.ico
QMAKE_TARGET_COMPANY = "Skycoder42"
QMAKE_TARGET_PRODUCT = $$TARGET
QMAKE_TARGET_DESCRIPTION = $$TARGET
QMAKE_TARGET_COPYRIGHT = "Felix Barz"
QMAKE_TARGET_BUNDLE_PREFIX = de.skycoder42

DEFINES += "TARGET=\\\"$$TARGET\\\""
DEFINES += "VERSION=\\\"$$VERSION\\\""
DEFINES += "COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
DEFINES += "BUNDLE=\"\\\"$$QMAKE_TARGET_BUNDLE_PREFIX\\\"\""

DEFINES += QT_DEPRECATED_WARNINGS QT_ASCII_CAST_WARNINGS

HEADERS += \
	clicontroller.h \
	rulecontroller.h \
	pkgresolver.h \
    pacmanrunner.h \
    global.h

SOURCES += main.cpp \
	clicontroller.cpp \
	rulecontroller.cpp \
	pkgresolver.cpp \
    pacmanrunner.cpp \
    global.cpp

DISTFILES += \
    README.md \
    repkg.sh \
    repkg.hook
