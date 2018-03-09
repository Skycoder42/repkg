#ifndef PKGRESOLVER_H
#define PKGRESOLVER_H

#include "rulecontroller.h"

#include <QObject>
#include <QSettings>

class PkgResolver : public QObject
{
	Q_OBJECT

public:
	explicit PkgResolver(QObject *parent = nullptr);

	QStringList listPkgs() const;
	QString listDetailPkgs() const;
	QList<QStringList> listPkgWaves() const;

	void updatePkgs(const QStringList &pkgs, RuleController *rules);
	void clear(const QStringList &pkgs);

private:
	typedef QMap<QString, QSet<QString>> PkgInfos; //package -> triggered by

	QSettings *_settings;

	PkgInfos readPkgs() const;
	void writePkgs(const PkgInfos &pkgInfos);
};

#endif // PKGRESOLVER_H
