#include "pacmanrunner.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <unistd.h>
#include <errno.h>

PacmanRunner::PacmanRunner(QObject *parent) :
	QObject(parent)
{}

QStringList PacmanRunner::frontend() const
{
	QSettings settings;
	if(settings.contains(QStringLiteral("frontend")))
	   return settings.value(QStringLiteral("frontend")).toStringList();
	else {
		QList<QStringList> defaultFn = {
			{QStringLiteral("pacaur"), QStringLiteral("--rebuild")},
			{QStringLiteral("yaourt")}
		};
		foreach(auto fn, defaultFn) {
			if(!QStandardPaths::findExecutable(fn.first()).isNull())
				return fn;
		}
		return {QStringLiteral("pacman")};
	}
}

void PacmanRunner::setFrontend(const QStringList &cli)
{
	QSettings settings;
	settings.setValue(QStringLiteral("frontend"), cli);
	qDebug() << "Updated pacman frontend to" << cli.first();
}

void PacmanRunner::run(const QStringList &pkgs)
{
	auto cli = frontend();
	cli[0] = QStandardPaths::findExecutable(cli[0]);
	if(cli[0].isNull())
		throw QStringLiteral("Unable to find binary \"%1\"").arg(pkgs.first());
	cli.append(QStringLiteral("-S"));
	cli.append(pkgs);

	QByteArrayList byteArgs;
	foreach (auto arg, cli)
		byteArgs.append(arg.toUtf8());

	auto args = new char*[cli.size() + 1];
	for(auto i = 0; i < cli.size(); i++)
		args[i] = byteArgs[i].data();
	args[cli.size()] = nullptr;

	execv(args[0], args);
	//unexcepted error
	throw QStringLiteral("execv failed with error code %1").arg(errno);
}
