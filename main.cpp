#include <QCoreApplication>
#include "clicontroller.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral(TARGET));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	//QCoreApplication::setOrganizationName(QStringLiteral(COMPANY)); DO not set to get the "correct" settings path
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));

	CliController cli;

	QMetaObject::invokeMethod(&cli, "parseCli", Qt::QueuedConnection);
	return a.exec();
}
