#ifndef PACMANRUNNER_H
#define PACMANRUNNER_H

#include <tuple>
#include <QObject>

class PacmanRunner : public QObject
{
	Q_OBJECT

public:
	explicit PacmanRunner(QObject *parent = nullptr);

	std::tuple<QStringList, bool> frontend() const; //(frontend, waved)
	QString frontendDescription() const;
	void setFrontend(const QStringList &cli, bool waved);
	bool isWaved() const;

	int run(const QList<QStringList> &pkgs);

private:
	void checkInstalled(const QString &pkg);
};

#endif // PACMANRUNNER_H
