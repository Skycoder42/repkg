#ifndef RULECONTROLLER_H
#define RULECONTROLLER_H

#include <QObject>
#include <QHash>

class RuleController : public QObject
{
	Q_OBJECT

public:
	explicit RuleController(QObject *parent = nullptr);

public slots:
	void readRules();

signals:

private:
	QMultiHash<QString, QString> _rules;
};

#endif // RULECONTROLLER_H
