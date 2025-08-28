#include <QtCore>
#include <QtNetwork>
#include "roomhub.h"

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCommandLineParser parser; parser.addHelpOption();
    QCommandLineOption portOpt(QStringList() << "p" << "port", "Listen port", "port", "9000");
    parser.addOption(portOpt);
    parser.process(app);

    quint16 port = parser.value(portOpt).toUShort();
    RoomHub hub;
    if (!hub.start(port)) return 1;

    qInfo() << "Usage: clients connect to server_ip:" << port;
    return app.exec();
}
