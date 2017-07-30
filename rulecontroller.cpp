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
	if(!ruleFile.open(QIODevice::WriteOnly | QIODevice::Text))
		throw QStringLiteral("Failed to create rule file with error: %1").arg(ruleFile.errorString());

	ruleFile.write(deps.join(QStringLiteral(" ")).toUtf8());
	ruleFile.close();
	qDebug() << "Created rule for" << qUtf8Printable(pkg);
}

void RuleController::readRules()
{
	QList<QDir> paths = {userPath(), rootPath()};

	foreach(auto path, paths) {
		QDir dir(path);
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
		dir.setNameFilters({QStringLiteral("*.rule")});
		foreach(auto fileInfo, dir.entryInfoList()) {
			auto name = fileInfo.completeBaseName();
			if(_rules.contains(name))
				continue;

			QFile file(fileInfo.absoluteFilePath());
			if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				qWarning() << "Failed to read file" << file.fileName()
						   << "with error" << file.errorString();
				continue;
			}

			auto str = QString::fromUtf8(file.readAll());
			auto pkgs = str.split(QRegularExpression(QStringLiteral("\\s")));
			foreach (auto pkg, pkgs)
				_rules.insert(name, pkg);
			file.close();
		}
	}
}
