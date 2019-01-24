#ifndef RULECONTROLLER_H
#define RULECONTROLLER_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <QMap>
#include <QFileInfo>
#include <variant>
#include <optional>

#include "pacmanrunner.h"

class RuleController : public QObject
{
	Q_OBJECT

public:
	enum class RuleScope {
		Any,
		Epoche,
		Version,
		Suffix,
		Revision
	};
	Q_ENUM(RuleScope)

	struct RuleInfo {
		using RangeContent = std::pair<int, std::optional<int>>;
		using Range = std::optional<RangeContent>; // (offset, limit)

		QString package;
		RuleScope scope = RuleScope::Any;
		Range range;
		std::optional<int> count;
	};

	struct RuleSource {
		bool extension = false;
		bool isRoot = false;
		QStringList targets;
	};

	explicit RuleController(PacmanRunner *runner, QObject *parent = nullptr);

	void createRule(const QString &pkg, const QStringList &deps);
	void removeRule(const QString &pkg);

	QString listRules(bool pkgOnly, bool userOnly);

	QList<RuleInfo> findRules(const QString &pkg);

private:
	PacmanRunner *_runner;
	QMap<QString, RuleSource> _ruleSources;
	QMultiHash<QString, RuleInfo> _rules;

	void readRules();
	QList<RuleInfo> readRuleDefinitions(const QFileInfo &fileInfo, RuleSource &srcBase);
	static void parseScope(RuleInfo &ruleInfo, const QStringRef &scopeStr);
	static void addRules(QList<RuleInfo> &target, const QList<RuleInfo> &newRules);
};

#endif // RULECONTROLLER_H
