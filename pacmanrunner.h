#ifndef PACMANRUNNER_H
#define PACMANRUNNER_H

#include <tuple>
#include <QObject>
#include <QProcess>

class PacmanRunner : public QObject
{
	Q_OBJECT

public:
	explicit PacmanRunner(QObject *parent = nullptr);

	std::tuple<QStringList, bool> frontend() const; //(frontend, waved)
	QString frontendDescription() const;
	void setFrontend(const QStringList &cli, bool waved);
	void resetFrontend();
	bool isWaved() const;

	int run(const QList<QStringList> &pkgs);

	QString readPackageVersion(const QString &pkg);
	QStringList readForeignPackages();

private:
	void initPacman(QProcess &proc, bool asVercmp = false) const;
};

#endif // PACMANRUNNER_H
