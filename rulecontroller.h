#ifndef RULECONTROLLER_H
#define RULECONTROLLER_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <QMap>
#include <variant>
#include <optional>

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
		bool extension = false;
		Range range;
		std::optional<int> count;
	};

	explicit RuleController(QObject *parent = nullptr);

	void createRule(const QString &pkg, const QStringList &deps);
	void removeRule(const QString &pkg);

	QString listRules(bool pkgOnly, bool userOnly) const;

	QList<RuleInfo> findRules(const QString &pkg) const;

private:
	mutable QMap<QString, std::pair<QStringList, bool>> _ruleInfos;
	mutable QMultiHash<QString, RuleInfo> _rules;

	void readRules() const;
	static void parseScope(RuleInfo &ruleInfo, const QStringRef &scopeStr);
};

#endif // RULECONTROLLER_H
