#ifndef PKGRESOLVER_H
#define PKGRESOLVER_H

#include <QObject>
#include <QSettings>

class PkgResolver : public QObject
{
	Q_OBJECT

public:
	explicit PkgResolver(QObject *parent = nullptr);

	QStringList listPkgs() const;
	QStringList listDetailPkgs() const;

	QStringList frontend() const;
	void setFrontend(const QStringList &cli);

	void clear();

private:
	QSettings *_settings;
};

#endif // PKGRESOLVER_H
