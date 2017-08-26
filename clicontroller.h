#ifndef CLICONTROLLER_H
#define CLICONTROLLER_H

#include "pkgresolver.h"
#include "rulecontroller.h"
#include "pacmanrunner.h"

#include <QObject>

class CliController : public QObject
{
	Q_OBJECT

public:
	explicit CliController(QObject *parent = nullptr);

	static bool verbose();

public slots:
	void parseCli();

private:
	void rebuild();
	void update(const QStringList &pks);
	void create(const QString &pkg, const QStringList &rules);
	void list(bool detail);
	void listRules();
	void clear(const QStringList &pkgs);
	void frontend();
	void setFrontend(const QStringList &frontend);
	void printArgs();

	void testEmpty(const QStringList &args);

	RuleController *_rules;
	PkgResolver *_resolver;
	PacmanRunner *_runner;

	bool _showHelp;
	static bool _verbose;
};

#endif // CLICONTROLLER_H
