#ifndef GLOBAL_H
#define GLOBAL_H

#include <QDir>

namespace global
{

bool isRoot();

QDir userPath();
QDir rootPath();
QDir systemPath();
}

#endif // GLOBAL_H
