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

QString RuleController::listRules(bool pkgOnly, bool userOnly) const
{
	if(_rules.isEmpty())
		readRules();

	if(pkgOnly) {
		QStringList pkgs;
		pkgs.reserve(_ruleInfos.size());
		for(auto it = _ruleInfos.constBegin(); it != _ruleInfos.constEnd(); it++) {
			if(userOnly && (it->second != isRoot()))
				continue;
			pkgs.append(it.key());
		}
		return pkgs.join(QLatin1Char(' '));
	} else {
		auto baselen = 9;
		for(auto it = _ruleInfos.constBegin(); it != _ruleInfos.constEnd(); it++)
			baselen = std::max(baselen, it.key().size() + 2);

		QStringList pkgs;
		pkgs.reserve(_ruleInfos.size() + 2);
		pkgs.append(QStringLiteral("%1| Origin | Triggers").arg(QStringLiteral(" Package"), -baselen));
		pkgs.append(QStringLiteral("-").repeated(baselen) + QLatin1Char('|') +
					QStringLiteral("-").repeated(8) + QLatin1Char('|') +
					QStringLiteral("-").repeated(70 - baselen));

		for(auto it = _ruleInfos.constBegin(); it != _ruleInfos.constEnd(); it++) {
			if(userOnly && (it->second != isRoot()))
				continue;
			pkgs.append(QStringLiteral(" %1| %2| %3")
						.arg(it.key(), -(baselen - 1))
						.arg(it->second ? QStringLiteral("System") : QStringLiteral("User"), -7)
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
		{userPath(), false},
		{rootPath(), true},
		{systemPath(), true},
	};

	_ruleInfos.clear();
	_rules.clear();
	QMultiHash<QString, RuleInfo> ruleBase;

	for(auto &path : paths) {
		auto &dir = path.first;
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
		dir.setNameFilters({QStringLiteral("*.rule")});
		for(const auto &fileInfo : dir.entryInfoList()) {
			auto name = fileInfo.completeBaseName();
			//DEBUG filter out wildcard rules
			if(name.contains(QLatin1Char('*')) || name.contains(QLatin1Char('?')))
				continue;

			// check for extension rules
			auto extension = false;
			if(name.startsWith(QLatin1Char('+'))) {
				name = name.mid(1);
				extension = true;
			}

			// skip already handeled rules
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
			static const QRegularExpression splitRegex{QStringLiteral("\\s+"), QRegularExpression::DontCaptureOption};
			auto pkgs = str.split(splitRegex, QString::SkipEmptyParts);

			QStringList ruleList;
			ruleList.reserve(pkgs.size());
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
				ruleList.append(match.captured(1));

				RuleInfo rule;
				rule.package = match.captured(1);
				rule.extension = extension;
				if(match.capturedLength(2) > 0)
					parseScope(rule, match.capturedRef(2));
				ruleBase.insert(name, rule);
			}
			_ruleInfos.insert(name, {ruleList, path.second});
		}
	}

	//invert rules for easier evaluation
	for(auto it = ruleBase.begin(); it != ruleBase.end(); it++) {
		auto name = it.key();
		auto &rule = *it;
		std::swap(name, rule.package);
		_rules.insert(name, rule);
	}
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
