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
	return readPkgs().keys();
}

QString PkgResolver::listDetailPkgs() const
{
	QStringList pkgs;
	pkgs.append(QStringLiteral("%1| Triggered by").arg(QStringLiteral(" Package Update"), -30));
	pkgs.append(QStringLiteral("-").repeated(30) + QLatin1Char('|') + QStringLiteral("-").repeated(49));

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

QList<QStringList> PkgResolver::listPkgWaves() const
{
	auto pkgs = readPkgs();

	QList<QStringList> waves;
	while(!pkgs.isEmpty()) {
		//find all packages that dont have a trigger that needs to be rebuild as well
		const auto keys = QSet<QString>::fromList(pkgs.keys());
		QStringList wave;
		for(auto it = pkgs.begin(); it != pkgs.end();) {
			if(keys.intersects(it.value()))
				it++; //has a trigger dep, postpone for later
			else {
				wave.append(it.key());
				it = pkgs.erase(it);
			}
		}
		if(wave.isEmpty()) {
			throw QStringLiteral("Cyclic dependencies detected! Is within packages: %1")
					.arg(keys.toList().join(QLatin1Char(' ')));
		}
		waves.append(wave);
		qDebug() << "Calculated wave:" << wave.join(QLatin1Char(' '));
	}

	return waves;
}

void PkgResolver::updatePkgs(const QStringList &pkgs, RuleController *rules, PacmanRunner *runner)
{
	if(!isRoot())
		throw QStringLiteral("Must be run as root to update packages!");

	QQueue<QString> pkgQueue;
	for(const auto& pkg : pkgs)
		pkgQueue.enqueue(pkg);

	auto pkgInfos = readPkgs();
	QSet<QString> skipPkgs;

	while (!pkgQueue.isEmpty()) {
		//handle each package only once
		auto pkg = pkgQueue.dequeue();
		if(skipPkgs.contains(pkg))
			continue;

		//check if packages need updates
		auto matches = rules->findRules(pkg);
		//add those to the "needs updates" list
		//and check if they themselves will trigger rebuilds by adding them to the queue
		for(const auto& match : matches) {
			if(checkVersionUpdate(match, pkg, runner)) {
				pkgInfos[match.first].insert(pkg);
				pkgQueue.enqueue(match.first);
				qDebug() << "Rule triggered. Marked"
						 << match.first
						 << "for updates because of"
						 << pkg;
			} else {
				qDebug() << "Rule skipped. Did not mark "
						 << match.first
						 << "for updates because version of"
						 << pkg
						 << "did not change significantly";
			}
		}

		//each package only once -> skip next time
		skipPkgs.insert(pkg);
	}

	//remove all "original" packages from the rebuild list as they have just been built
	for(const auto& pkg : pkgs)
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
		for(const auto& pkg : pkgs)
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
		_settings->setValue(QStringLiteral("reason"), static_cast<QStringList>(pkgInfos[keys[i]].toList()));
	}
	_settings->endArray();
}

bool PkgResolver::checkVersionUpdate(const RuleController::RuleInfo &pkgInfo, const QString &target, PacmanRunner *runner)
{
	auto newVersion = runner->readPackageVersion(target);

	_settings->beginGroup(QStringLiteral("versions"));
	_settings->beginGroup(pkgInfo.first);
	auto oldVersion = _settings->value(target).toString();
	_settings->setValue(target, newVersion);
	_settings->endGroup();
	_settings->endGroup();

	if(!pkgInfo.second.isValid() || oldVersion.isEmpty())
		return true;

	// filter both versions
	oldVersion = pkgInfo.second.match(oldVersion).captured(1);
	newVersion = pkgInfo.second.match(newVersion).captured(1);
	if(newVersion.isEmpty() || oldVersion.isEmpty()) { // in case of a failed match -> is update
		qWarning() << "Failed to filter package versions of" << pkgInfo.first
				   << "for rule of" << target << "- this indicates a broken regex!";
		return true;
	}

	qDebug() << "Check versions of" << target << ", comparing"
			 << oldVersion << "with" << newVersion;
	return runner->comparePackageVersion(oldVersion, newVersion);
}
