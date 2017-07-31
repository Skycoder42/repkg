#include <QCoreApplication>
#include <iostream>
#include "clicontroller.h"

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral(TARGET));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));

	qInstallMessageHandler(messageHandler);

	CliController cli;

	QMetaObject::invokeMethod(&cli, "parseCli", Qt::QueuedConnection);
	return a.exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	auto message = qFormatLogMessage(type, context, msg);
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

	if(type == QtFatalMsg)
		abort();
}
