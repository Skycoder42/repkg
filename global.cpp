#include "global.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <unistd.h>

bool global::isRoot()
{
	return ::geteuid() == 0;
}

QDir global::userPath()
{
	QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	if(dir.mkpath(QStringLiteral("rules")) &&
	   dir.cd(QStringLiteral("rules")))
		return dir;

	return QDir();
}

QDir global::rootPath()
{
	QDir dir = QStringLiteral("/etc/%1/rules")
			   .arg(QCoreApplication::applicationName());
	if(::geteuid() == 0)
		dir.mkpath(QStringLiteral("."));
	if(dir.exists())
		return dir;
	else
		return QDir();
}
