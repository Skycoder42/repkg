#include <QCoreApplication>
#include <iostream>
#include "clicontroller.h"

static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral(TARGET));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));

	CliController controller;
	controller.parseArguments(a);
	qInstallMessageHandler(messageHandler);

	return a.exec();
}

static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	auto message = qFormatLogMessage(type, context, msg);

	QT_WARNING_PUSH
	QT_WARNING_DISABLE_GCC("-Wimplicit-fallthrough")
	switch (type) {
	case QtDebugMsg:
		if(!CliController::verbose())
			break;
	case QtWarningMsg:
	case QtCriticalMsg:
	case QtFatalMsg:
		std::cerr << message.toStdString() << std::endl;
		break;
	case QtInfoMsg:
		std::cout << message.toStdString() << std::endl;
		break;
	default:
		break;
	}
	QT_WARNING_POP

	if(type == QtFatalMsg)
		abort();
}
