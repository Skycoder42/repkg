#include "clicontroller.h"

#include <QCoreApplication>
#include <QDebug>

CliController::CliController(QObject *parent) :
	QObject(parent),
	_rules(new RuleController(this)),
	_resolver(new PkgResolver(this)),
	_runner(new PacmanRunner(this))
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
			rebuild();
		} else if(command == QStringLiteral("update"))
			update(args);
		else if(command == QStringLiteral("create")) {
			if(args.isEmpty())
				throw QStringLiteral("Expected package name as first argument for create");
			create(args.takeFirst(), args);
		} else if(command == QStringLiteral("list")) {
			auto detail = false;
			if(!args.isEmpty() && args[0] == QStringLiteral("detail")) {
				detail = true;
				args.takeFirst();
			}
			testEmpty(args);
			list(detail);
		} else if(command == QStringLiteral("clear")) {
			testEmpty(args);
			clear();
		} else if(command == QStringLiteral("frontend")) {
			if(args.isEmpty())
				frontend();
			else
				setFrontend(args);
		} else if(command == QStringLiteral("help")) {
			printArgs();
			qApp->quit();
		} else
			throw QStringLiteral("Invalid arguments!");
	} catch(QString &e) {
		qCritical() << qUtf8Printable(e) << "\n\n";
		printArgs();
		qApp->exit(EXIT_FAILURE);
	}
}

void CliController::rebuild()
{

}

void CliController::update(const QStringList &pks)
{

}

void CliController::create(const QString &pkg, const QStringList &rules)
{
	try {
		_rules->createRule(pkg, rules);
		qApp->quit();
	} catch (QString &s) {
		qCritical() << "Failed to create rule file with error:"
					<< qUtf8Printable(s);
		qApp->exit(EXIT_FAILURE);
	}
}

void CliController::list(bool detail)
{
	if(detail)
		qInfo() << qUtf8Printable(_resolver->listDetailPkgs().join(QStringLiteral("\n"))) << "\n";
	else
		qInfo() << qUtf8Printable(_resolver->listPkgs().join(QStringLiteral(" ")));
	qApp->quit();
}

void CliController::clear()
{
	if(_resolver->clear())
		qApp->quit();
	else
		qApp->exit(EXIT_FAILURE);
}

void CliController::frontend()
{
	qInfo() << qUtf8Printable(_runner->frontend().join(QStringLiteral(" ")));
	qApp->quit();
}

void CliController::setFrontend(const QStringList &frontend)
{
	_runner->setFrontend(frontend);
	qApp->quit();
}

void CliController::printArgs()
{
	auto usage = QStringLiteral("Usage: %1 [operation] [...]\n"
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

void CliController::testEmpty(const QStringList &args)
{
	if(!args.isEmpty())
		throw QStringLiteral("Unexpected arguments after command!");
}
