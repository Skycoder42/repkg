#ifndef PACMANRUNNER_H
#define PACMANRUNNER_H

#include <QObject>

class PacmanRunner : public QObject
{
	Q_OBJECT
public:
	explicit PacmanRunner(QObject *parent = nullptr);

	QStringList frontend() const;
	void setFrontend(const QStringList &cli);

	int run(const QStringList &pkgs);

private:
	void checkInstalled(const QString &pkg);
};

#endif // PACMANRUNNER_H
