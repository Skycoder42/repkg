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
	QObject{parent}
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
				.arg(pkg, ruleFile.errorString());
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
		auto baselen = 9;
		for(auto it = _ruleInfos.constBegin(); it != _ruleInfos.constEnd(); it++)
			baselen = std::max(baselen, it.key().size() + 2);

		QStringList pkgs;
		pkgs.append(QStringLiteral("%1| Origin | Triggers").arg(QStringLiteral(" Package"), -baselen));
		pkgs.append(QStringLiteral("-").repeated(baselen) + QLatin1Char('|') +
					QStringLiteral("-").repeated(8) + QLatin1Char('|') +
					QStringLiteral("-").repeated(70 - baselen));

		for(auto it = _ruleInfos.constBegin(); it != _ruleInfos.constEnd(); it++) {
			pkgs.append(QStringLiteral(" %1| %2| %3")
						.arg(it.key(), -(baselen - 1))
						.arg(it->second ? QStringLiteral("User") : QStringLiteral("System"), -7)
						.arg(it->first.join(QLatin1Char(' '))));
		}
		return pkgs.join(QLatin1Char('\n'));
	}
}

QList<RuleController::RuleInfo> RuleController::findRules(const QString &pkg) const
{
	if(_rules.isEmpty())
		readRules();
	return _rules.values(pkg);
}

void RuleController::readRules() const
{
	QList<std::pair<QDir, bool>> paths {
		{userPath(), true},
		{rootPath(), false}
	};

	_ruleInfos.clear();
	_rules.clear();
	QMultiHash<QString, std::pair<QString, QRegularExpression>> ruleBase;

	for(auto &path : paths) {
		auto &dir = path.first;
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
		dir.setNameFilters({QStringLiteral("*.rule")});
		for(const auto &fileInfo : dir.entryInfoList()) {
			auto name = fileInfo.completeBaseName();
			if(ruleBase.contains(name))
				continue;

			QFile file{fileInfo.absoluteFilePath()};
			if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				qWarning() << "Failed to read file" << file.fileName()
						   << "with error" << file.errorString();
				continue;
			}

			auto str = QString::fromUtf8(file.readAll());
			file.close();
			static const QRegularExpression splitRegex{
				QStringLiteral("\\s"),
				QRegularExpression::DontCaptureOption |
				QRegularExpression::OptimizeOnFirstUsageOption
			};
			auto pkgs = str.split(splitRegex);

			QStringList ruleList;
			ruleList.reserve(pkgs.size());
			for(const auto& pkgInfo : pkgs) {
				static const QRegularExpression pkgInfoRegex{
					QStringLiteral(R"__(^(.*?)(?:{{(.*)}})?$)__"),
					QRegularExpression::OptimizeOnFirstUsageOption
				};
				auto match = pkgInfoRegex.match(pkgInfo);
				Q_ASSERT(match.hasMatch());
				ruleList.append(match.captured(1));
				if(match.capturedView(2).isEmpty())
					ruleBase.insert(name, {match.captured(1), {}});
				else
					ruleBase.insert(name, {match.captured(1), QRegularExpression{match.captured(2)}});
			}
			_ruleInfos.insert(name, {ruleList, path.second});
		}
	}

	//invert rules for easier evaluation
	for(auto it = ruleBase.begin(); it != ruleBase.end(); it++)
		_rules.insert(it.value().first, {it.key(), it.value().second});
}
