#include "rulecontroller.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>

RuleController::RuleController(QObject *parent) :
	QObject(parent),
	_rules()
{}

void RuleController::readRules()
{
	auto paths = {
		QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
		QStringLiteral("/etc/%1").arg(QCoreApplication::applicationName())
	};

	foreach(auto path, paths) {
		QDir dir(path);
		if(!dir.cd(QStringLiteral("rules")))
			continue;

		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
		dir.setNameFilters({QStringLiteral("*.rule")});
		foreach(auto file, dir.entryInfoList()) {
			auto name = file.completeBaseName();
			if(_rules.contains(name))
				continue;

			qDebug(qPrintable(name));
			//TODO here
		}
	}
}
