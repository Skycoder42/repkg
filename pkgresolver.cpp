#include "pkgresolver.h"
#include "global.h"

#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
using namespace global;

PkgResolver::PkgResolver(QObject *parent) :
	QObject(parent),
	_settings(new QSettings(rootPath().absoluteFilePath(QStringLiteral("%1/state.conf")),
							QSettings::IniFormat,
							this))
{}

QStringList PkgResolver::listPkgs() const
{
	auto count = _settings->beginReadArray(QStringLiteral("pkgstate"));
	QStringList pkgs;
	for(auto i = 0; i < count; i++) {
		_settings->setArrayIndex(i);
		pkgs.append(_settings->value(QStringLiteral("name")).toString());
	}
	_settings->endArray();
	return pkgs;
}

QStringList PkgResolver::listDetailPkgs() const
{
	auto count = _settings->beginReadArray(QStringLiteral("pkgstate"));
	QStringList pkgs;
	for(auto i = 0; i < count; i++) {
		_settings->setArrayIndex(i);
		pkgs.append(QStringLiteral("%1: Triggered by %2")
					.arg(_settings->value(QStringLiteral("name")).toString())
					.arg(_settings->value(QStringLiteral("reason")).toString()));
	}
	_settings->endArray();
	return pkgs;
}

void PkgResolver::updatePkgs(const QStringList &pkgs, RuleController *rules)
{
	if(!isRoot())
		throw QStringLiteral("Must be run as root to update packages!");

}

void PkgResolver::clear()
{
	if(!isRoot())
		throw QStringLiteral("Must be run as root to clear packages!");

	_settings->remove(QStringLiteral("pkgstate"));
	qDebug() << "Cleared all pending package rebuilds";
}
