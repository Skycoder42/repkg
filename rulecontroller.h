#ifndef RULECONTROLLER_H
#define RULECONTROLLER_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <QMap>
#include <QRegularExpression>

class RuleController : public QObject
{
	Q_OBJECT

public:
	using RuleInfo = std::pair<QString, QRegularExpression>;

	explicit RuleController(QObject *parent = nullptr);

	void createRule(const QString &pkg, const QStringList &deps);
	void removeRule(const QString &pkg);

	QString listRules(bool pkgOnly, bool userOnly) const;

	QList<RuleInfo> findRules(const QString &pkg) const;

private:
	mutable QMap<QString, std::pair<QStringList, bool>> _ruleInfos;
	mutable QMultiHash<QString, RuleInfo> _rules;


	void readRules() const;
};

#endif // RULECONTROLLER_H
