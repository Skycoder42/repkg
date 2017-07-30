#include "rulecontroller.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

#include <unistd.h>

RuleController::RuleController(QObject *parent) :
	QObject(parent),
	_rules()
{}

void RuleController::createRule(const QString &pkg, const QStringList &deps)
{
	QDir path;
	if(::geteuid() == 0)
		path = rootPath();
	else
		path = userPath();

	QFile ruleFile(path.absoluteFilePath(pkg + QStringLiteral(".rule")));
	if(!ruleFile.open(QIODevice::WriteOnly | QIODevice::Text))
		throw ruleFile.errorString();

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

QDir RuleController::userPath() const
{
	QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	if(dir.mkpath(QStringLiteral("rules")) &&
	   dir.cd(QStringLiteral("rules")))
		return dir;

	return QDir();
}

QDir RuleController::rootPath() const
{
	QDir dir = QStringLiteral("/etc/%1/rules")
			   .arg(QCoreApplication::applicationName());
	if(::geteuid() == 0)
		dir.mkpath(QStringLiteral("."));
	if(dir.exists())
		return dir;
	else
		return QDir();
}
