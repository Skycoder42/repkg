#include "clicontroller.h"

#include <QCoreApplication>
#include <QDebug>

CliController::CliController(QObject *parent) :
	QObject(parent)
{}

void CliController::parseCli()
{
	try {
		auto args = QCoreApplication::arguments();
		args.removeFirst();

		QString command;
		if(!args.isEmpty())
			command = args.takeFirst();
		if(command.isEmpty())
			command = QStringLiteral("rebuild");

		if(command == QStringLiteral("rebuild")) {
			testEmpty(args);
			emit rebuild();
		} else if(command == QStringLiteral("update"))
			emit update(args);
		else if(command == QStringLiteral("create")) {
			if(args.isEmpty())
				throw tr("Expected package name as first argument for create");
			emit create(args.takeFirst(), args);
		} else if(command == QStringLiteral("list")) {
			testEmpty(args);
			emit list();
		} else if(command == QStringLiteral("clear")) {
			testEmpty(args);
			emit clear();
		} else if(command == QStringLiteral("frontend")) {
			if(args.isEmpty())
				emit frontend(QString());
			else {
				auto name = args.takeFirst();
				testEmpty(args);
				emit frontend(name);
			}
		} else if(command == QStringLiteral("help")) {
			printArgs();
			qApp->quit();
		} else
			throw tr("Invalid arguments!");
	} catch(QString &e) {
		qCritical() << qUtf8Printable(e) << "\n\n";
		printArgs();
		qApp->exit(EXIT_FAILURE);
	}
}

void CliController::testEmpty(const QStringList &args)
{
	if(!args.isEmpty())
		emit tr("Unexpected arguments after command!");
}

void CliController::printArgs()
{
	auto usage = tr("Usage: %1 [operation] [...]\n"
					"Operations:\n"
					"\t%1 [rebuild]: Build all packages that need a rebuild\n"
					"\t%1 update [packages...]: Mark packages as updated\n"
					"\t%1 create <package> [dependencies...]: Create a rule for a package and it's dependencies\n"
					"\t%1 list: List all packages that need to be rebuilt\n"
					"\t%1 clear: Clear all packages that are marked to be rebuilt\n"
					"\t%1 frontend [tool]: Display the current frontend or set a custom one.\n")
				 .arg(QCoreApplication::applicationName());
	qInfo() << qUtf8Printable(usage);
}
