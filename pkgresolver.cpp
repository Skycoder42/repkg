#include "pkgresolver.h"
#include "global.h"

#include <QCoreApplication>
#include <QDebug>
#include <QQueue>
#include <QStandardPaths>
using namespace global;

PkgResolver::PkgResolver(QObject *parent) :
	QObject(parent),
	_settings(new QSettings(rootPath().absoluteFilePath(QStringLiteral("../state.conf")),
							QSettings::IniFormat,
							this))
{}

QStringList PkgResolver::listPkgs() const
{
	QStringList pkgs;
	foreach (auto pkg, readPkgs().keys())
		pkgs.append(pkg);
	return pkgs;
}

QString PkgResolver::listDetailPkgs() const
{
	QStringList pkgs;
	pkgs.append(QStringLiteral("%1| Triggered by").arg(QStringLiteral(" Package Update"), -30));
	pkgs.append(QStringLiteral("-").repeated(30) + QLatin1Char('|') + QStringLiteral("-").repeated(49)); //TODO get console width

	auto pkgInfos = readPkgs();
	for(auto it = pkgInfos.constBegin(); it != pkgInfos.constEnd(); it++) {
		auto lst = it.value().toList();
		std::sort(lst.begin(), lst.end());
		pkgs.append(QStringLiteral("%1| %2")
					.arg(it.key(), -30)
					.arg(lst.join(QStringLiteral(", "))));
	}
	return pkgs.join(QLatin1Char('\n'));
}

void PkgResolver::updatePkgs(const QStringList &pkgs, RuleController *rules)
{
	if(!isRoot())
		throw QStringLiteral("Must be run as root to update packages!");

	QQueue<QString> pkgQueue;
	foreach (auto pkg, pkgs)
		pkgQueue.enqueue(pkg);

	auto pkgInfos = readPkgs();
	QSet<QString> skipPkgs;

	while (!pkgQueue.isEmpty()) {
		//handle each package only once
		auto pkg = pkgQueue.dequeue();
		if(skipPkgs.contains(pkg))
			continue;

		//check if packages need updates
		auto matches = rules->analyze(pkg);
		//add those to the "needs updates" list
		//and check if they themselves will trigger rebuilds by adding them to the queue
		foreach (auto match, matches) {
			pkgInfos[match].insert(pkg);
			pkgQueue.enqueue(match);
			qDebug() << "Rule triggered. Marked"
					 << match
					 << "for updates because of"
					 << pkg;
		}

		//each package only once -> skip next time
		skipPkgs.insert(pkg);
	}

	//remove all "original" packages from the rebuild list as they have just been built
	foreach(auto pkg, pkgs)
		pkgInfos.remove(pkg);

	//save the infos
	writePkgs(pkgInfos);
}

void PkgResolver::clear(const QStringList &pkgs)
{
	if(!isRoot())
		throw QStringLiteral("Must be run as root to clear packages!");

	if(pkgs.isEmpty()) {
		_settings->remove(QStringLiteral("pkgstate"));
		qDebug() << "Cleared all pending package rebuilds";
	} else {
		auto pkgInfos = readPkgs();
		auto save = false;
		foreach(auto pkg, pkgs)
			save = pkgInfos.remove(pkg) || save;
		if(save)
			writePkgs(pkgInfos);
		qDebug() << "Cleared specified pending package rebuilds";
	}
}

PkgResolver::PkgInfos PkgResolver::readPkgs() const
{
	auto count = _settings->beginReadArray(QStringLiteral("pkgstate"));
	PkgInfos pkgs;
	for(auto i = 0; i < count; i++) {
		_settings->setArrayIndex(i);
		pkgs.insert(_settings->value(QStringLiteral("name")).toString(),
					QSet<QString>::fromList(_settings->value(QStringLiteral("reason")).toStringList()));
	}
	_settings->endArray();
	return pkgs;
}

void PkgResolver::writePkgs(const PkgInfos &pkgInfos)
{
	auto keys = pkgInfos.keys();
	_settings->remove(QStringLiteral("pkgstate"));
	_settings->beginWriteArray(QStringLiteral("pkgstate"), pkgInfos.size());
	for(auto i = 0; i < pkgInfos.size(); i++) {
		_settings->setArrayIndex(i);
		_settings->setValue(QStringLiteral("name"), keys[i]);
		_settings->setValue(QStringLiteral("reason"), (QStringList)pkgInfos[keys[i]].toList());
	}
	_settings->endArray();
}
