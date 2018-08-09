#include "pacmanrunner.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QProcess>
#include <QCoreApplication>

#include <unistd.h>
#include <cerrno>

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
			std::make_tuple(QStringList{QStringLiteral("yay"), QStringLiteral("--rebuild"), QStringLiteral("-S")}, false),
			std::make_tuple(QStringList{QStringLiteral("trizen"), QStringLiteral("-S")}, false),
			std::make_tuple(QStringList{QStringLiteral("pacaur"), QStringLiteral("--rebuild"), QStringLiteral("-S")}, false),
			std::make_tuple(QStringList{QStringLiteral("yaourt"), QStringLiteral("-S")}, true)
		};
		for(const auto &fn : defaultFn) {
			if(!QStandardPaths::findExecutable(std::get<0>(fn).first()).isNull())
				return fn;
		}
		return std::make_tuple(QStringList{QStringLiteral("pacman")}, true);
	}
}

QString PacmanRunner::frontendDescription() const
{
	auto fn = frontend();
	return QStringLiteral("%1 %2")
			.arg(std::get<0>(fn).join(QLatin1Char(' ')),
				 std::get<1>(fn) ? QStringLiteral("<package wave> ...") : QStringLiteral("<all packages...>"));
}

void PacmanRunner::setFrontend(const QStringList &cli, bool waved)
{
	QSettings settings;
	settings.setValue(QStringLiteral("frontend"), cli);
	settings.setValue(QStringLiteral("frontend/waved"), waved);
	qDebug() << "Updated pacman frontend to" << cli.first()
			 << (waved ? "(waved)" : "(sorted)");
}

void PacmanRunner::resetFrontend()
{
	QSettings settings;
	settings.remove(QStringLiteral("frontend"));
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

		// prepare arguments
		auto argSize = cliArgs.size() + 1;
		QByteArrayList rawArgs;
		rawArgs.reserve(argSize);
		QVector<char*> execArgs{argSize + 1, nullptr};
		// bin
		rawArgs.append(bin.toUtf8());
		execArgs[0] = rawArgs[0].data();
		// args
		for(auto i = 1; i < argSize; i++) {
			rawArgs.append(cliArgs[i - 1].toUtf8());
			execArgs[i] = rawArgs[i].data();
		}

		::execv(execArgs[0], execArgs.data());
		//unexcepted error
		throw QStringLiteral("execv failed with error: %1").arg(qt_error_string(errno));
	}
}
