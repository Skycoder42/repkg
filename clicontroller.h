#ifndef CLICONTROLLER_H
#define CLICONTROLLER_H

#include "pkgresolver.h"
#include "rulecontroller.h"
#include "pacmanrunner.h"

#include <QCoreApplication>
#include <QObject>
#include <qcliparser.h>

class CliController : public QObject
{
	Q_OBJECT

public:
	explicit CliController(QObject *parent = nullptr);

	void parseArguments(const QCoreApplication &app);
	static bool verbose();

private slots:
	void run();

private:
	void setup();

	void rebuild();
	void update(const QStringList &pks);
	void create(const QString &pkg, const QStringList &rules);
	void remove(const QStringList &pkgs);
	void list(bool detail);
	void listRules(bool listShort);
	void clear(const QStringList &pkgs);
	void frontend();
	void setFrontend(const QStringList &frontend);

	void testEmpty(const QStringList &args);

	QScopedPointer<QCliParser> _parser;

	RuleController *_rules;
	PkgResolver *_resolver;
	PacmanRunner *_runner;

	static bool _verbose;
};

#endif // CLICONTROLLER_H
