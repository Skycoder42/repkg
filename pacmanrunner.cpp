#include "pacmanrunner.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QProcess>
#include <QCoreApplication>

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
			{QStringLiteral("trizen")},
			{QStringLiteral("pacaur"), QStringLiteral("--rebuild")},
			{QStringLiteral("yaourt")}
		};
		for(auto fn : defaultFn) {
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

int PacmanRunner::run(const QStringList &pkgs)
{
	if(pkgs.isEmpty()) {
		qWarning() << "No packages need to be rebuilt";
		return EXIT_SUCCESS;
	}

	//check if all packages are installed
	QProcess proc;
	auto pacman = QStandardPaths::findExecutable(QStringLiteral("pacman"));
	if(pacman.isNull())
		throw QStringLiteral("Unable to find pacman binary in PATH");
	proc.setProgram(pacman);
	QStringList pacArgs {QStringLiteral("-Qi")};
	pacArgs.append(pkgs);
	proc.setArguments(pacArgs);
	proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
	proc.setStandardOutputFile(QProcess::nullDevice());

	qDebug() << "Checking if all packages are still installed...";
	proc.start();
	proc.waitForFinished(-1);
	if(proc.exitCode() != EXIT_SUCCESS)
		throw QStringLiteral("Please remove repkg files of uninstalled packages and mark the unchanged via `repkg update <pkg>`");

	// run the frontend to reinstall packages
	auto cliArgs = frontend();
	auto bin = QStandardPaths::findExecutable(cliArgs.takeFirst());
	if(bin.isNull())
		throw QStringLiteral("Unable to find binary \"%1\" in PATH").arg(bin);
	cliArgs.append(QStringLiteral("-S"));
	cliArgs.append(pkgs);
	return QProcess::execute(bin, cliArgs);
}
