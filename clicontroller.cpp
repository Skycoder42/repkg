#include "clicontroller.h"

#include <QCoreApplication>
#include <QDebug>

bool CliController::_verbose = false;

CliController::CliController(QObject *parent) :
	QObject(parent),
	_parser(new QCliParser()),
	_rules(new RuleController(this)),
	_resolver(new PkgResolver(this)),
	_runner(new PacmanRunner(this))
{}

void CliController::parseArguments(const QCoreApplication &app)
{
	setup();
	_parser->process(app, true);

	_verbose = _parser->isSet(QStringLiteral("verbose"));
	QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
}

bool CliController::verbose()
{
	return _verbose;
}

void CliController::run()
{
	try {
		auto args = _parser->positionalArguments();
		if(_parser->enterContext(QStringLiteral("rebuild"))) {
			testEmpty(args);
			rebuild();
		} else if(_parser->enterContext(QStringLiteral("update")))
			update(args);
		else if(_parser->enterContext(QStringLiteral("create"))) {
			if(args.isEmpty())
				throw tr("You must specify a package to create a rule for");
			create(args.takeFirst(), args);
		} else if(_parser->enterContext(QStringLiteral("remove")))
			remove(args);
		else if(_parser->enterContext(QStringLiteral("list"))) {
			testEmpty(args);
			list(_parser->isSet(QStringLiteral("detail")));
		} else if(_parser->enterContext(QStringLiteral("rules"))) {
			testEmpty(args);
			listRules(_parser->isSet(QStringLiteral("short")));
		} else if(_parser->enterContext(QStringLiteral("clear")))
			clear(args);
		else if(_parser->enterContext(QStringLiteral("frontend"))) {
			testEmpty(args);
			if(_parser->isSet(QStringLiteral("set")))
				setFrontend(_parser->value(QStringLiteral("set")).split(QLatin1Char(' ')));
			else
				frontend();
		} else
			throw QStringLiteral("Invalid arguments");
		_parser->leaveContext();
	} catch(QString &e) {
		qCritical().noquote() << e;
		qApp->exit(EXIT_FAILURE);
	}
}

void CliController::setup()
{
	_parser->setApplicationDescription(QStringLiteral("A tool to manage rebuilding of AUR packages based on their dependencies"));
	_parser->addVersionOption();
	_parser->addHelpOption();

	_parser->addOption({
						   QStringLiteral("verbose"),
						   QStringLiteral("Run in verbose mode to output more information.")
					   });

	_parser->addLeafNode(QStringLiteral("rebuild"), QStringLiteral("Build all packages that need a rebuild."));
	_parser->setDefaultNode(QStringLiteral("rebuild"));

	auto updateNode = _parser->addLeafNode(QStringLiteral("update"), QStringLiteral("Mark packages as updated."));
	updateNode->addPositionalArgument(QStringLiteral("packages"),
									  QStringLiteral("The packages to be marked as updated."),
									  QStringLiteral("[<package> ...]"));

	auto createNode = _parser->addLeafNode(QStringLiteral("create"), QStringLiteral("Create a rule for a package and it's dependencies."));
	createNode->addPositionalArgument(QStringLiteral("package"), QStringLiteral("The package to create a rule for."));
	createNode->addPositionalArgument(QStringLiteral("dependencies"),
									  QStringLiteral("The packages this one depends on and requires a rebuild for."),
									  QStringLiteral("[<dependency> ...]"));

	auto removeNode = _parser->addLeafNode(QStringLiteral("remove"), QStringLiteral("Remove previously created package rules"));
	removeNode->addPositionalArgument(QStringLiteral("packages"),
									  QStringLiteral("The packages to remove the rules for."),
									  QStringLiteral("[<package> ...]"));

	auto listNode = _parser->addLeafNode(QStringLiteral("list"), QStringLiteral("List all packages that need to be rebuilt."));
	listNode->addOption({
							{QStringLiteral("d"), QStringLiteral("detail")},
							QStringLiteral("Display a detailed table with all packages and the dependencies that triggered them.")
						});

	auto rulesNode = _parser->addLeafNode(QStringLiteral("rules"), QStringLiteral("List all rules known to repkg"));
	rulesNode->addOption({
							 {QStringLiteral("s"), QStringLiteral("short")},
							 QStringLiteral("Only display the package names, not the path to the rule file.")
						 });


	auto clearNode = _parser->addLeafNode(QStringLiteral("clear"),
										  QStringLiteral("Clear all packages that are marked to be rebuilt, or only the ones specified as parameters."));
	clearNode->addPositionalArgument(QStringLiteral("packages"),
									 QStringLiteral("The packages to be cleared. If no packages are specified, all packages are cleared."),
									 QStringLiteral("[<package> ...]"));

	auto frontendNode = _parser->addLeafNode(QStringLiteral("frontend"), QStringLiteral("Display the current frontend or set a custom one."));
	frontendNode->addOption({
								{QStringLiteral("s"), QStringLiteral("set")},
								QStringLiteral("Instead of displaying the tool, set a new <tool> as the one to be used by repkg."),
								QStringLiteral("tool")
							});
}

void CliController::rebuild()
{
	qApp->exit(_runner->run(_resolver->listPkgs()));
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

void CliController::remove(const QStringList &pkgs)
{
	for(auto pkg : pkgs)
		_rules->removeRule(pkg);
	qInfo() << "Remember to run `sudo repkg update <pkgs>` to remove any already scheduled rebuilds";
	qApp->quit();
}

void CliController::list(bool detail)
{
	if(detail)
		qInfo().noquote() << _resolver->listDetailPkgs();
	else {
		auto list = _resolver->listPkgs();
		if(!list.isEmpty())
			qInfo().noquote() << list.join(QStringLiteral(" "));
	}
	qApp->quit();
}

void CliController::listRules(bool listShort)
{
	qInfo().noquote() << _rules->listRules(listShort);
	qApp->quit();
}

void CliController::clear(const QStringList &pkgs)
{
	_resolver->clear(pkgs);
	qApp->quit();
}

void CliController::frontend()
{
	qInfo().noquote() << _runner->frontend().join(QStringLiteral(" "));
	qApp->quit();
}

void CliController::setFrontend(const QStringList &frontend)
{
	_runner->setFrontend(frontend);
	qApp->quit();
}

void CliController::testEmpty(const QStringList &args)
{
	if(!args.isEmpty())
		throw QStringLiteral("Unexpected arguments after command!");
}
