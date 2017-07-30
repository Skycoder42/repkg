#include "global.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <unistd.h>

bool global::isRoot()
{
	return ::getuid() == 0;
}

QDir global::userPath()
{
	auto user = qgetenv("SUDO_USER");
	if(user.isEmpty())
		user = qgetenv("USER");//TODO get username with function?

	QDir dir = QStringLiteral("/home/%1/.config/%2/rules")
			   .arg(QString::fromUtf8(user))
			   .arg(QCoreApplication::applicationName());
	dir.mkpath(QStringLiteral("."));
	if(dir.exists())
		return dir;
	else
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
