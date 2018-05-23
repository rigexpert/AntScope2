#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    QMessageBox::information(NULL, "Test",
//                 QString::number(argc));
//    for(int i = 0; i < argc; ++i )
//    {
//        QMessageBox::information(NULL, "Test",
//                     argv[i]);
//    }
    if(argc > 1)
    {
        QString path = argv[1];
        if(path.indexOf(".asd") >= 0)
        {
            w.openFile(path);
        }
    }
    w.show();

    return a.exec();
}
