#ifndef PKGRESOLVER_H
#define PKGRESOLVER_H

#include "pacmanrunner.h"
#include "rulecontroller.h"

#include <QObject>
#include <QSettings>
#include <QVersionNumber>

class PkgResolver : public QObject
{
	Q_OBJECT

public:
	explicit PkgResolver(PacmanRunner *runner, RuleController *controller, QObject *parent = nullptr);

	QStringList listPkgs() const;
	QString listDetailPkgs() const;
	QList<QStringList> listPkgWaves() const;

	void updatePkgs(const QStringList &pkgs);
	void clear(const QStringList &pkgs);

private:
	struct VersionTuple {
		int epoche = 0;
		QVersionNumber version;
		QString suffix;
		QVersionNumber revision;
	};
	using PkgInfos = QMap<QString, QSet<QString>>; //package -> triggered by

	QSettings *_settings;
	PacmanRunner *_runner;
	RuleController *_controller;

	PkgInfos readPkgs() const;
	void writePkgs(const PkgInfos &pkgInfos);

	QString readVersion();

	bool checkVersionUpdate(const RuleController::RuleInfo &pkgInfo, const QString &target);
	static VersionTuple splitVersion(const QString &version, bool &ok);
};

#endif // PKGRESOLVER_H
