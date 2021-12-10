#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    w.showFullScreen();
    w.setWindowFlag(Qt::FramelessWindowHint);
    w.setAttribute(Qt::WA_TranslucentBackground);
//    w.showMaximized();//w.setWindowState(Qt::WindowMaximized);
    w.show();
    return a.exec();

}
