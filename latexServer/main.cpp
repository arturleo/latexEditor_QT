
#include <QCoreApplication>
#include "syncserver.h"
//void testServer(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	syncServer server(3435);
	//qDebug() << "pulse";
	return a.exec();
}

//void testServer(int argc, char* argv[])
//{
//
//}