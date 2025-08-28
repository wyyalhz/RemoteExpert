#include <QtWidgets>
#include <QtCore>
#include <QtNetwork>
#include "mainwindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QCommandLineParser parser; parser.addHelpOption();
    QCommandLineOption portOpt(QStringList() << "p" << "port", "Listen port", "port", "9000");
    parser.addOption(portOpt);
    parser.process(app);

    ServerWindow w;
    // 如果命令行给了端口，就预填
    QString portStr = parser.value(portOpt);
    if (!portStr.isEmpty()) {
        w.findChild<QLineEdit*>("edPort")->setText(portStr);
    }
    w.show();
    return app.exec();
}
