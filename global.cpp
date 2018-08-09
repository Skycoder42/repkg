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
	QByteArray user;

	auto login = ::getlogin();
	if(login)
		user = login;
	else {
		user = qgetenv("SUDO_USER");
		if(user.isEmpty())
			user = qgetenv("USER");
	}

	QDir dir = QStringLiteral("/home/%1/.config/%2/rules")
			   .arg(QString::fromUtf8(user), QCoreApplication::applicationName());
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
	if(isRoot())
		dir.mkpath(QStringLiteral("."));
	if(dir.exists())
		return dir;
	else
		return QDir();
}
