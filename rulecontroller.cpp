#include "rulecontroller.h"
#include "global.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
using namespace global;

RuleController::RuleController(QObject *parent) :
	QObject(parent),
	_rules()
{}

void RuleController::createRule(const QString &pkg, const QStringList &deps)
{
	QDir path;
	if(isRoot())
		path = rootPath();
	else
		path = userPath();

	QFile ruleFile(path.absoluteFilePath(pkg + QStringLiteral(".rule")));
	if(!ruleFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		throw QStringLiteral("Failed to create rule file for %1 with error: %2")
				.arg(pkg)
				.arg(ruleFile.errorString());
	}

	ruleFile.write(deps.join(QStringLiteral(" ")).toUtf8());
	ruleFile.close();
	qDebug() << "Created rule for" << qUtf8Printable(pkg) << "as:" << ruleFile.fileName();
}

void RuleController::removeRule(const QString &pkg)
{
	QDir path;
	if(isRoot())
		path = rootPath();
	else
		path = userPath();

	QFile ruleFile(path.absoluteFilePath(pkg + QStringLiteral(".rule")));
	if(!ruleFile.exists())
		qWarning() << "Rule for" << qUtf8Printable(pkg) << "does not exist";
	else if(!ruleFile.remove())
		throw QStringLiteral("Failed to remove rule file for %1").arg(pkg);
}

QString RuleController::listRules(bool pkgOnly) const
{
	if(_rules.isEmpty())
		readRules();

	if(pkgOnly)
		return _ruleInfos.keys().join(QLatin1Char(' '));
	else {
		QStringList pkgs;
		pkgs.append(QStringLiteral("%1| Rule file").arg(QStringLiteral(" Package"), -30));
		pkgs.append(QStringLiteral("-").repeated(30) + QLatin1Char('|') + QStringLiteral("-").repeated(49));

		for(auto it = _ruleInfos.constBegin(); it != _ruleInfos.constEnd(); it++) {
			pkgs.append(QStringLiteral("%1| %2")
						.arg(it.key(), -30)
						.arg(it.value()));
		}
		return pkgs.join(QLatin1Char('\n'));
	}
}

QStringList RuleController::analyze(const QString &pkg) const
{
	if(_rules.isEmpty())
		readRules();
	return _rules.values(pkg);
}

void RuleController::readRules() const
{
	QList<QDir> paths = {userPath(), rootPath()};

	_ruleInfos.clear();
	_rules.clear();
	QMultiHash<QString, QString> ruleBase;

	for(auto path : paths) {
		QDir dir(path);
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
		dir.setNameFilters({QStringLiteral("*.rule")});
		for(auto fileInfo : dir.entryInfoList()) {
			auto name = fileInfo.completeBaseName();
			if(ruleBase.contains(name))
				continue;

			QFile file(fileInfo.absoluteFilePath());
			if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				qWarning() << "Failed to read file" << file.fileName()
						   << "with error" << file.errorString();
				continue;
			}

			auto str = QString::fromUtf8(file.readAll());
			file.close();
			auto pkgs = str.split(QRegularExpression(QStringLiteral("\\s")));

			_ruleInfos.insert(name, fileInfo.absoluteFilePath());
			for(auto pkg : pkgs)
				ruleBase.insert(name, pkg);
		}
	}

	//invert rules for easier evaluation
	for(auto it = ruleBase.begin(); it != ruleBase.end(); it++)
		_rules.insert(it.value(), it.key());
}
