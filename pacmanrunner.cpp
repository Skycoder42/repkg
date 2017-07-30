#include "pacmanrunner.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

PacmanRunner::PacmanRunner(QObject *parent) : QObject(parent)
{

}

QStringList PacmanRunner::frontend() const
{
	QSettings settings;
	if(settings.contains(QStringLiteral("frontend")))
	   return settings.value(QStringLiteral("frontend")).toStringList();
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

void PacmanRunner::setFrontend(const QStringList &cli)
{
	QSettings settings;
	settings.setValue(QStringLiteral("frontend"), cli);
	qDebug() << "Updated pacman frontend to" << cli.first();
}
