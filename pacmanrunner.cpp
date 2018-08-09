#include "pacmanrunner.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QProcess>
#include <QCoreApplication>

PacmanRunner::PacmanRunner(QObject *parent) :
	QObject(parent)
{}

std::tuple<QStringList, bool> PacmanRunner::frontend() const
{
	QSettings settings;
	if(settings.contains(QStringLiteral("frontend"))) {
		return std::make_tuple(settings.value(QStringLiteral("frontend")).toStringList(),
							   settings.value(QStringLiteral("frontend/waved"), false).toBool());
	} else {
		static const QList<std::tuple<QStringList, bool>> defaultFn = {
			std::make_tuple(QStringList{QStringLiteral("trizen")}, false),
			std::make_tuple(QStringList{QStringLiteral("pacaur"), QStringLiteral("--rebuild")}, false),
			std::make_tuple(QStringList{QStringLiteral("yaourt")}, true)
		};
		for(auto fn : defaultFn) {
			if(!QStandardPaths::findExecutable(std::get<0>(fn).first()).isNull())
				return fn;
		}
		return std::make_tuple(QStringList{QStringLiteral("pacman")}, true);
	}
}

QString PacmanRunner::frontendDescription() const
{
	auto fn = frontend();
	return QStringLiteral("%1 (%2)")
			.arg(std::get<0>(fn).join(QLatin1Char(' ')),
				 std::get<1>(fn) ? QStringLiteral("waved") : QStringLiteral("grouped"));
}

void PacmanRunner::setFrontend(const QStringList &cli, bool waved)
{
	QSettings settings;
	settings.setValue(QStringLiteral("frontend"), cli);
	settings.setValue(QStringLiteral("frontend/waved"), waved);
	qDebug() << "Updated pacman frontend to" << cli.first()
			 << (waved ? "(waved)" : "(sorted)");
}

int PacmanRunner::run(const QList<QStringList> &waves)
{
	if(waves.isEmpty()) {
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
	for(const auto& pkgs : waves)
		pacArgs.append(pkgs);
	proc.setArguments(pacArgs);
	proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
	proc.setStandardOutputFile(QProcess::nullDevice());

	qDebug() << "Checking if all packages are still installed...";
	proc.start();
	proc.waitForFinished(-1);
	if(proc.exitCode() != EXIT_SUCCESS)
		throw QStringLiteral("Please remove repkg files of uninstalled packages and mark the unchanged via `repkg clear <pkg>`");

	// run the frontend to reinstall packages
	QStringList cliArgs;
	bool waved;
	std::tie(cliArgs, waved) = frontend();
	auto bin = QStandardPaths::findExecutable(cliArgs.takeFirst());
	if(bin.isNull())
		throw QStringLiteral("Unable to find binary \"%1\" in PATH").arg(bin);
	cliArgs.append(QStringLiteral("-S"));
	if(waved) {
		for(const auto& pkgs : waves) {
			auto args = cliArgs;
			args.append(pkgs);
			auto res = QProcess::execute(bin, args);
			if(res != EXIT_SUCCESS)
				return res;
		}
		return EXIT_SUCCESS;
	} else {
		for(const auto& pkgs : waves)
			cliArgs.append(pkgs);
		return QProcess::execute(bin, cliArgs);
	}
}
