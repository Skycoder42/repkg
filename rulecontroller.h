#ifndef RULECONTROLLER_H
#define RULECONTROLLER_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <QMap>

class RuleController : public QObject
{
	Q_OBJECT

public:
	explicit RuleController(QObject *parent = nullptr);

	void createRule(const QString &pkg, const QStringList &deps);

	QString listRules() const;

	QStringList analyze(const QString &pkg) const;

private:
	mutable QMap<QString, QString> _ruleInfos;
	mutable QMultiHash<QString, QString> _rules;


	void readRules() const;
};

#endif // RULECONTROLLER_H
