#include <QCoreApplication>

#include <nvrmanager.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	NvrManager n;
//	n.getNvrInfo(QStringList() << "50.23.2.231");
//	n.getEncInfo(QStringList() << "50.23.2.231");
	qDebug() << n.getRecordInfo("50.23.2.231", "50.23.20.244", 6);
	return a.exec();
}

