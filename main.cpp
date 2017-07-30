#include <QCoreApplication>
#include "clicontroller.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	auto ctr = new CliController(qApp);
	QMetaObject::invokeMethod(ctr, "parseCli", Qt::QueuedConnection);

	return a.exec();
}
