TEMPLATE = app

QT += core
QT -= gui

CONFIG += c++17 console warning_clean exceptions
CONFIG -= app_bundle

TARGET = repkg
VERSION = 1.3.0

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
	repkg.hook \
	completitions/bash/repkg \
	completitions/zsh/_repkg

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target

hook.path = /usr/share/libalpm/hooks
hook.files += repkg.hook
script.path = /usr/share/libalpm/scripts
script.files += repkg.sh
bashcomp.path = /usr/share/bash-completion/completions
bashcomp.files += completitions/bash/repkg
zshcomp.path = /usr/share/zsh/site-functions/
zshcomp.files = completitions/zsh/_repkg
INSTALLS += hook script bashcomp zshcomp

QDEP_DEPENDS += Skycoder42/QCliParser
!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
