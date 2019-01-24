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

RuleController::RuleController(PacmanRunner *runner, QObject *parent) :
	QObject{parent},
	_runner{runner}
{}

void RuleController::createRule(const QString &pkg, bool autoDepends, QStringList deps)
{
	QDir path;
	if(isRoot())
		path = rootPath();
	else
		path = userPath();

	if(autoDepends)
		deps.append(_runner->listDependencies(pkg));

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

QString RuleController::listRules(bool pkgOnly, bool userOnly)
{
	if(_rules.isEmpty())
		readRules();

	if(pkgOnly) {
		QStringList pkgs;
		pkgs.reserve(_ruleSources.size());
		for(auto it = _ruleSources.constBegin(); it != _ruleSources.constEnd(); it++) {
			if(userOnly && (it->isRoot != isRoot()))
				continue;
			pkgs.append(it.key());
		}
		return pkgs.join(QLatin1Char(' '));
	} else {
		auto baselen = 9;
		for(auto it = _ruleSources.constBegin(); it != _ruleSources.constEnd(); it++)
			baselen = std::max(baselen, it.key().size() + 2);

		QStringList pkgs;
		pkgs.reserve(_ruleSources.size() + 2);
		pkgs.append(QStringLiteral("%1| Origin | Ext. | Triggers").arg(QStringLiteral(" Package"), -baselen));
		pkgs.append(QStringLiteral("-").repeated(baselen) + QLatin1Char('|') +
					QStringLiteral("-").repeated(8) + QLatin1Char('|') +
					QStringLiteral("-").repeated(6) + QLatin1Char('|') +
					QStringLiteral("-").repeated(67 - baselen));

		for(auto it = _ruleSources.constBegin(); it != _ruleSources.constEnd(); it++) {
			if(userOnly && (it->isRoot != isRoot()))
				continue;
			pkgs.append(QStringLiteral(" %1| %2| %3| %4")
						.arg(it.key(), -(baselen - 1))
						.arg(it->isRoot ? QStringLiteral("System") : QStringLiteral("User"), -7)
						.arg(it->extension ? QStringLiteral("Yes") : QStringLiteral("No"), -5)
						.arg(it->targets.join(QLatin1Char(' '))));
		}
		return pkgs.join(QLatin1Char('\n'));
	}
}

QList<RuleController::RuleInfo> RuleController::findRules(const QString &pkg)
{
	if(_rules.isEmpty())
		readRules();
	return _rules.values(pkg);
}

void RuleController::readRules()
{
	QList<std::pair<QDir, bool>> paths {
		{userPath(), false},
		{rootPath(), true},
		{systemPath(), true},
	};

	_ruleSources.clear();
	_rules.clear();
	QHash<QString, std::pair<QList<RuleInfo>, bool>> ruleBase;  // (rules, extension)
	QHash<QString, std::tuple<QRegularExpression, QList<RuleInfo>, bool>> wildcardRules;  // (pattern, rules, extension)

	for(auto &path : paths) {
		auto &dir = path.first;
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
		dir.setNameFilters({QStringLiteral("*.rule")});
		for(const auto &fileInfo : dir.entryInfoList()) {
			RuleSource ruleSrc;
			auto name = fileInfo.completeBaseName();
			ruleSrc.isRoot = path.second;
			// check for extension rules
			if(name.startsWith(QLatin1Char('+'))) {
				name = name.mid(1);
				ruleSrc.extension = true;
			}

			// special handling for wildcard rules
			if(name.contains(QLatin1Char('*')) ||
			   name.contains(QLatin1Char('?')) ||
			   (name.contains(QLatin1Char('[')) && name.contains(QLatin1Char(']')))) {
				auto definitions = readRuleDefinitions(fileInfo, ruleSrc);
				if(wildcardRules.contains(name)) {
					auto &entry = wildcardRules[name];
					if(std::get<2>(entry)) {
						addRules(std::get<1>(entry), definitions);
						std::get<2>(entry) = ruleSrc.extension;
					}
				} else {
					QRegularExpression ruleRegex {
						QRegularExpression::wildcardToRegularExpression(name),
						QRegularExpression::DontCaptureOption
					};
					wildcardRules.insert(name, std::make_tuple(std::move(ruleRegex), std::move(definitions), ruleSrc.extension));
				}
			} else { // normal rules are treated normall
				// skip already handeled rules
				if(ruleBase.contains(name))
					continue;
				// read rule definitions and add to mapping and rule list
				ruleBase.insert(name, {readRuleDefinitions(fileInfo, ruleSrc), ruleSrc.extension});
			}
			_ruleSources.insert(name, ruleSrc);
		}
	}

	// find ALL foreign packages and match them against the wildcards to add them if neccessary
	if(!wildcardRules.isEmpty()) {
		for(const auto &pkg : _runner->readForeignPackages()) {
			// skip already existing rules
			if(ruleBase.contains(pkg))
				continue;
			// match againts wildcards
			for(const auto &wTpl : qAsConst(wildcardRules)) {
				if(std::get<0>(wTpl).match(pkg).hasMatch())
					ruleBase.insert(pkg, {std::get<1>(wTpl), std::get<2>(wTpl)});
			}
		}
	}

	for(auto it = ruleBase.begin(); it != ruleBase.end(); it++) {
		// add regex rules to extensible normal rules
		if(it->second) {
			for(const auto &wTpl : qAsConst(wildcardRules)) {
				if(std::get<0>(wTpl).match(it.key()).hasMatch())
					addRules(it->first, std::get<1>(wTpl));
			}
		}

		//invert rules for easier evaluation
		for(auto &rule : it->first) {
			auto name = it.key();
			std::swap(name, rule.package);
			_rules.insert(name, rule);
		}
	}
}

QList<RuleController::RuleInfo> RuleController::readRuleDefinitions(const QFileInfo &fileInfo, RuleSource &srcBase)
{
	QFile file{fileInfo.absoluteFilePath()};
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Failed to read file" << file.fileName()
				   << "with error" << file.errorString();
		return {};
	}

	auto str = QString::fromUtf8(file.readAll());
	file.close();
	static const QRegularExpression splitRegex{QStringLiteral("\\s+"), QRegularExpression::DontCaptureOption};
	auto pkgs = str.split(splitRegex, QString::SkipEmptyParts);

	QList<RuleInfo> rules;
	for(auto pkgInfo : pkgs) { //MAJOR make const & again
		{
			//MAJOR compat workaround to filter out old regex syntax
			static const QRegularExpression pkgCompatRegex{QStringLiteral(R"__(^(.*?)(?:{{(.*)}})?$)__")};
			const auto compatMatch = pkgCompatRegex.match(pkgInfo);
			if(compatMatch.hasMatch())
				pkgInfo = compatMatch.captured(1);
		}

		static const QRegularExpression pkgInfoRegex{QStringLiteral(R"__(^(.+?)(?:=([\dvsr:]+))?$)__")};
		auto match = pkgInfoRegex.match(pkgInfo);
		Q_ASSERT(match.hasMatch());

		RuleInfo rule;
		rule.package = match.captured(1);
		if(match.capturedLength(2) > 0)
			parseScope(rule, match.capturedRef(2));
		rules.append(rule);
		srcBase.targets.append(rule.package);
	}

	return rules;
}

void RuleController::parseScope(RuleInfo &ruleInfo, const QStringRef &scopeStr)
{
	// first: extract the substring range if applicable
	QStringRef parseStr;
	static const QRegularExpression rangeRegex{QStringLiteral(R"__(^:(\d+)(?::(\d+))?(?:::(.*)$|$))__")};
	const auto rangeMatch = rangeRegex.match(scopeStr);
	if(rangeMatch.hasMatch()) {
		ruleInfo.range = RuleInfo::RangeContent{};
		ruleInfo.range->first = rangeMatch.capturedRef(1).toInt();
		if(rangeMatch.capturedLength(2) > 0)
			ruleInfo.range->second = rangeMatch.capturedRef(2).toInt();
		if(rangeMatch.capturedLength(3) > 0)
			parseStr = rangeMatch.capturedRef(3);
	} else
		parseStr = scopeStr;

	if(!parseStr.isNull()) {
		static const QRegularExpression scopeRegex{QStringLiteral(R"__(^(?:\d+|v|s|r)$)__")};
		if(!scopeRegex.match(parseStr).hasMatch()) {
			//TODO print warning
		} else {
			if(parseStr == QStringLiteral("r"))
				ruleInfo.scope = RuleScope::Revision;
			else if(parseStr == QStringLiteral("s"))
				ruleInfo.scope = RuleScope::Suffix;
			else if(parseStr == QStringLiteral("v"))
				ruleInfo.scope = RuleScope::Version;
			else if(parseStr == QStringLiteral("0"))
				ruleInfo.scope = RuleScope::Epoche;
			else {
				ruleInfo.scope = RuleScope::Version;
				ruleInfo.count = parseStr.toInt();
			}
		}
	}
}

void RuleController::addRules(QList<RuleController::RuleInfo> &target, const QList<RuleController::RuleInfo> &newRules)
{
	const auto begin = target.begin();
	const auto end = target.end();
	for(const auto &rule : newRules) {
		auto fIndex = std::find_if(begin, end, [rule](const RuleInfo &tRule){
			return rule.package == tRule.package;
		});
		if(fIndex == target.end())
			target.append(rule);
	}
}
