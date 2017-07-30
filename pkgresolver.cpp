#include "pkgresolver.h"

#include <QDebug>
#include <QStandardPaths>

PkgResolver::PkgResolver(QObject *parent) :
	QObject(parent),
	_settings(new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/state.conf"),
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

QStringList PkgResolver::frontend() const
{
	if(_settings->contains(QStringLiteral("frontend")))
	   return _settings->value(QStringLiteral("frontend")).toStringList();
	else {
		QList<QStringList> defaultFn = {
			{QStringLiteral("pacaur"), QStringLiteral("--rebuild")},
			{QStringLiteral("yaourt")}
		};
		foreach(auto fn, defaultFn) {
			if(!QStandardPaths::findExecutable(fn.first()).isNull())
				return fn;
		}
		return {QStringLiteral("pacman")};
	}
}

void PkgResolver::setFrontend(const QStringList &cli)
{
	_settings->setValue(QStringLiteral("frontend"), cli);
	qDebug() << "Updated pacman frontend to" << cli.first();
}

void PkgResolver::clear()
{
	_settings->remove(QStringLiteral("pkgstate"));
	qDebug() << "Cleared all pending package rebuilds";
}
