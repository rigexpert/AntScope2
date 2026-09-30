                                                                                                                                    #include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QSslSocket>     //20260929_vn
#include <QAbstractNativeEventFilter>
#include "analyzer/customanalyzer.h"
#include "style.h"
#include "settings.h"     //20260930_vn : Settings::migrateLegacyIniFile()

bool g_developerMode = false;
bool g_usbOnly = false;
bool g_raspbian = false;
//20260930_vn : доступність захищених з'єднань. Визначається один раз у main()
// після вибору бекенда; використовується в settings.cpp, щоб пояснити
// користувачу недоступність реєстрації замість тривожного діалогу при старті.
bool g_tlsAvailable = false;
bool g_bAA55modeNewProtocol = false;
MainWindow* g_mainWindow;
int g_maxDots = 2000;

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbt.h>

//#ifndef _DEBUG
//#define LOG_TO_FILE
//#endif

#ifdef LOG_TO_FILE
QString logFilePath = "antscope2";
bool firstLog = true;
void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type != QtInfoMsg)
        return;
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QString sufix = QDateTime::currentDateTime().toString("-yyyyMMdd_hhmmss.log");
    QString logLevelName = "";//msgLevelHash[type];

    QString txt = QString("%1 %2: %3 (%4:%5, %6)")
            .arg(formattedTime, logLevelName, msg,  context.file)
            .arg(context.line)
            .arg(context.function);
    if (firstLog) {
        firstLog = false;
        QDir dir = QDir::tempPath();
        logFilePath = dir.absoluteFilePath(logFilePath + sufix);
    }
    QFile outFile(logFilePath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << "\n";
    ts.flush();
}
#endif

class MyNativeEventFilter : public QAbstractNativeEventFilter {
public :
    virtual bool nativeEventFilter( const QByteArray &eventType, void *message, long * /*result*/ )
    //Q_DECL_OVERRIDE
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


void setAbsoluteFqMaximum()
{
    int fqMax = 0;

    if (CustomAnalyzer::customized() && CustomAnalyzer::getCurrent() != nullptr) {
            fqMax = CustomAnalyzer::getCurrent()->maxFq().toInt();
    } else {
#ifdef NEW_ANALYZER
        foreach (AnalyzerParameters* param, AnalyzerParameters::analyzers()) {
            QString str = param->maxFq();
            int fq = str.toInt();
            fqMax = qMax(fqMax, fq);
        }
#else
        for (int idx=1; idx<QUANTITY; idx++) {
            QString str = maxFq[idx];
            int fq = str.toInt();
            fqMax = qMax(fqMax, fq);
        }
#endif
    }
    ABSOLUTE_MAX_FQ = fqMax;
}

int g_showMessageBox(QWidget* parent, QMessageBox::Icon icon,
                      QString title, QString text,
                      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
{
    QMessageBox msgBox;
    msgBox.setIcon(icon);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    msgBox.setStyleSheet(Style::messageBox());
    return msgBox.exec();
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

// Fix for 4K Display Issues Disabled
#ifdef DUMB_Q_OS_WIN
    char** params;
    params = new char*[argc+2];
    int ip=0;
    for (; ip<argc; ip++) {
        params[ip] = argv[ip];
    }
    params[ip++] = (char*)"--platform";
    params[ip] = (char*)"windows:dpiawareness=0";
    int cntp = argc + 2;
    QApplication a(cntp, params);
#else
    QApplication a(argc, argv);
#endif

    QStringList args = a.arguments();

    //20260930_vn : нативний криптопровайдер на кожній платформі.
    // Windows — Schannel, macOS — Secure Transport, решта — системний OpenSSL.
    // Так застосунок не постачає криптографії взагалі: виправлення приходять
    // через Windows Update, Apple Software Update і менеджер пакетів відповідно.
    //
    // Вибір має відбутись ДО першого QSslSocket чи QNetworkAccessManager —
    // інакше Qt зафіксує бекенд за замовчуванням і перемикання не спрацює.
    // Раніше (20260929_vn) тут був лише примусовий openssl під ANTSCOPE_FORCE_OPENSSL,
    // тож на Windows Schannel виходив не вибором, а відсутністю плагіна openssl
    // у постачанні — і міг мовчки змінитись при зміні складу розгортання.
    {
        QStringList preferred;
#ifdef ANTSCOPE_FORCE_OPENSSL
        // збірка з qmake CONFIG+=use_openssl — явна вимога саме OpenSSL
        preferred << QStringLiteral("openssl");
#elif defined(Q_OS_WIN)
        preferred << QStringLiteral("schannel") << QStringLiteral("openssl");
#elif defined(Q_OS_MACOS)
        preferred << QStringLiteral("securetransport") << QStringLiteral("openssl");
#else
        preferred << QStringLiteral("openssl");
#endif
        const QStringList available = QSslSocket::availableBackends();
        for (const QString &backend : preferred) {
            if (!available.contains(backend))
                continue;
            if (QSslSocket::setActiveBackend(backend))
                break;
            qCritical() << "*** Failed to activate TLS backend" << backend;
        }
    }

    //20260930_vn : стан фіксуємо після вибору бекенда й до створення вікна
    g_tlsAvailable = QSslSocket::supportsSsl();

    //20260930_vn : перенос старого файлу налаштувань має відбутись ДО створення
    // MainWindow — саме його конструктор першим відкриває QSettings через
    // Settings::setIniFile(), і після цього перейменовувати файл вже пізно.
    Settings::migrateLegacyIniFile();

#ifdef LOG_TO_FILE
    qInstallMessageHandler(customMessageOutput);
    qInfo() << "                                                         ";
    qInfo() << "*********************************************************";
    qInfo() << "  AntScope2 " << QString(ANTSCOPE2VER) << " STARTED " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qInfo() << "                                                         ";
#endif

#ifdef Q_OS_WIN
    // TODO DEBUG: catch attach/detach device event
    //MyNativeEventFilter myEventfilter;
    //a.eventDispatcher()->installNativeEventFilter(&myEventfilter);
#endif

    if (args.contains("-developer")) {
        g_developerMode = true;
        g_maxDots = 1000000;
    }
    if (args.contains("-usb-only")) {
        g_usbOnly = true;
    }

    g_raspbian = QSysInfo::productType().contains("raspbian", Qt::CaseInsensitive);

    QString style;
    style = Style::messageBox();
    a.setStyleSheet(style);

    style = Style::dialog();
    style += Style::pushButton();
    style += Style::label();
    style += Style::lineEdit();
    a.setStyleSheet(style);

    MainWindow w;
    g_mainWindow = w.m_mainWindow;

    foreach (QString path, args) {
        if (path.contains(".asd")) {
            w.openFile(path);
            break;
        }
    }
    w.show();

    return a.exec();
}
