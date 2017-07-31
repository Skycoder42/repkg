#include "clicontroller.h"

#include <QCoreApplication>
#include <QDebug>

bool CliController::_verbose = false;

CliController::CliController(QObject *parent) :
	QObject(parent),
	_rules(new RuleController(this)),
	_resolver(new PkgResolver(this)),
	_runner(new PacmanRunner(this)),
	_showHelp(false)
{}

bool CliController::verbose()
{
	return _verbose;
}

void CliController::parseCli()
{
	try {
		auto args = QCoreApplication::arguments();
		args.removeFirst();

		if(!args.isEmpty() &&
		   (args.first() == QStringLiteral("-v") ||
			args.first() == QStringLiteral("--verbose"))) {
			_verbose = true;
			args.removeFirst();
		}

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
			if(args.isEmpty()) {
				_showHelp = true;
				throw QStringLiteral("Expected package name as first argument for create");
			}
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
			clear(args);
		} else if(command == QStringLiteral("frontend")) {
			if(args.isEmpty())
				frontend();
			else
				setFrontend(args);
		} else if(command == QStringLiteral("help")) {
			printArgs();
			qApp->quit();
		} else {
			_showHelp = true;
			throw QStringLiteral("Invalid arguments!");
		}
	} catch(QString &e) {
		qCritical() << qUtf8Printable(e);
		if(_showHelp)
			printArgs();
		qApp->exit(EXIT_FAILURE);
	}
}

void CliController::rebuild()
{
	_runner->run(_resolver->listPkgs());
	Q_UNREACHABLE();
}

void CliController::update(const QStringList &pks)
{
	_resolver->updatePkgs(pks, _rules);
	qApp->quit();
}

void CliController::create(const QString &pkg, const QStringList &rules)
{
	_rules->createRule(pkg, rules);
	qApp->quit();
}

void CliController::list(bool detail)
{
	if(detail)
		qInfo() << qUtf8Printable(_resolver->listDetailPkgs());
	else {
		auto list = _resolver->listPkgs();
		if(!list.isEmpty())
			qInfo() << qUtf8Printable(list.join(QStringLiteral(" ")));
	}
	qApp->quit();
}

void CliController::clear(const QStringList &pkgs)
{
	_resolver->clear(pkgs);
	qApp->quit();
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
	auto usage = QStringLiteral("Usage: %1 [-v|--verbose] [operation] [...]\n"
								"Operations:\n"
								"\t%1 [rebuild]: Build all packages that need a rebuild\n"
								"\t%1 update [packages...]: Mark packages as updated\n"
								"\t%1 create <package> [dependencies...]: Create a rule for a package and it's dependencies\n"
								"\t%1 list [detail]: List all packages that need to be rebuilt\n"
								"\t%1 clear [pkgs...]: Clear all packages that are marked to be rebuilt, or only the ones specified as parameters\n"
								"\t%1 frontend [tool]: Display the current frontend or set a custom one.\n\n"
								"Pass -v as additional parameter to enable verbose output.")
							 .arg(QCoreApplication::applicationName());
	qInfo() << qUtf8Printable(usage);
}

void CliController::testEmpty(const QStringList &args)
{
	if(!args.isEmpty()) {
		_showHelp = true;
		throw QStringLiteral("Unexpected arguments after command!");
	}
}
