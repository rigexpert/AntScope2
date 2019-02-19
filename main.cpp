#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QAbstractNativeEventFilter>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbt.h>

bool g_noRestrictScale = false;

QString logFilePath = "antscope2-debug.log";
bool logToFile = false;
void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    if (logToFile) {
        QString txt = QString("%1 %2: %3 (%4:%5, %6)")
                .arg(formattedTime, logLevelName, msg,  context.file)
                .arg(context.line)
                .arg(context.function);
        QDir dir = QDir::tempPath();
        QFile outFile(dir.absoluteFilePath(logFilePath));
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
        ts.flush();
    } else {
        fprintf(stderr, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
        fflush(stderr);
    }
}


class MyNativeEventFilter : public QAbstractNativeEventFilter {
public :
    virtual bool nativeEventFilter( const QByteArray &eventType, void *message, long *result )
    Q_DECL_OVERRIDE
    {
        if (eventType == "windows_generic_MSG")
        {
          MSG *msg = static_cast<MSG *>(message);
          static int i = 0;

              msg = (MSG*)message;
                  //qDebug() << "message: " << msg->message << " wParam: " << msg->wParam
                    //  << " lParam: " << msg->lParam;
              if (msg->message == WM_DEVICECHANGE)
              {
                  qDebug() << "WM_DEVICECHANGE: " <<
                              (msg->wParam==DBT_DEVICEARRIVAL?"DBT_DEVICEARRIVAL":
                              (msg->wParam==DBT_DEVICEREMOVECOMPLETE?"DBT_DEVICEREMOVECOMPLETE":QString::number(msg->wParam)));
              }
            }
        return false;
    }
};
#endif

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    //{ TODO log to file support
    //qInstallMessageHandler(customMessageOutput);
    //}

#ifdef Q_OS_WIN
    // TODO DEBUG: catch attach/detach device event
    //MyNativeEventFilter myEventfilter;
    //a.eventDispatcher()->installNativeEventFilter(&myEventfilter);
#endif

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
        } else if(path.indexOf("-develop") >= 0)
        {
            g_noRestrictScale = true;
        }
    }
    w.show();

    return a.exec();
}
