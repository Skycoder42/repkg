#ifndef CLICONTROLLER_H
#define CLICONTROLLER_H

#include <QObject>

class CliController : public QObject
{
	Q_OBJECT

public:
	explicit CliController(QObject *parent = nullptr);

public slots:
	void parseCli();

signals:
	void rebuild();
	void update(const QStringList &pks);
	void create(const QString &pkg, const QStringList &rules);
	void list();
	void clear();
	void frontend(const QString &frontend);

private:
	void testEmpty(const QStringList &args);
	void printArgs();
};

#endif // CLICONTROLLER_H
