#include <QApplication>
#include <QtWebEngine>

#include "mainwindow.h"
#include "mathutil.h"
#include "multiplayermanager.h"

#ifdef WIN32
#include <conio.h>
#include <Windows.h>
#endif

void DisplayVersion()
{
    std::cout << QString(__JANUS_VERSION_COMPLETE).toLatin1().data() << "\n";
}

void DisplayHelp()
{
    std::cout << "Janus VR - Help\n\n";
    std::cout << "Command line usage:\n\n";
    std::cout << "  janusvr [-server SERVER] [-port PORT]\n";
    std::cout << "          [-adapter X] [-render MODE] [-window] [-width X] [-height X]\n";
    std::cout << "          [-help] [-pos X Y Z] [-output_cubemap X] [<url>]\n\n";
    std::cout << "    -server     - SERVER specifies the server to create an initial portal\n";
    std::cout << "    -port       - PORT is a number that specifies the port of the server to connect to\n";
    std::cout << "    -adapter    - X specifies the screen that the Janus window will be positioned on\n";
    std::cout << "    -render     - MODE specifies render mode\n";
    std::cout << "                  (can be: 2d, sbs, sbs_reverse, ou3d, cube, equi, rift, vive)\n";    
    std::cout << "    -window     - launch as a window instead of fullscreen\n";
    std::cout << "    -width      - set width of JanusVR window in windowed mode\n";
    std::cout << "    -height     - set height of JanusVR window in windowed mode\n";
    std::cout << "    -novsync    - run JanusVR without V-sync\n";    
    std::cout << "    -output_cubemap     - save out 2k x 2k per face cubemap face images with filename\n";
    std::cout << "                          prefix X from [pos] at  [url], then exit\n";
    std::cout << "    -output_equi        - same as output_cubemap but also saves out an 8k x 4k\n";
    std::cout << "                          equirectangular image with filename prefix X from [pos] at \n";
    std::cout << "                          [url], then exit\n";
    std::cout << "    -help       - prints out this help information, then exits\n";
    std::cout << "    -version    - prints version, then exits\n";
    std::cout << "    <url>       - location to start in\n";
}

bool isCommandArg2(QString arg)
{
    const bool isServer = (QString::compare(arg, "-server", Qt::CaseInsensitive) == 0);
    const bool isPort   = (QString::compare(arg, "-port", Qt::CaseInsensitive) == 0);
    const bool isCubemap = (QString::compare(arg, "-output_cubemap", Qt::CaseInsensitive) == 0);
    const bool isEqui = (QString::compare(arg, "-output_equi", Qt::CaseInsensitive) == 0);

    return (isServer || isPort || isCubemap || isEqui);
}

void ProcessCmdLineArgs1(int argc, char *argv[])
{
    //49.45 - Set command line parameters *after* constructors are called, so we can change defaults/override saved settings
    if (argc > 1) {
        for (int i=1; i<argc; ++i) {
            const QString eacharg(argv[i]);
            if (isCommandArg2(eacharg)) {
                ++i;
                continue;
            }
            else if (QString::compare(eacharg, "-adapter", Qt::CaseInsensitive) == 0 && i < argc-1) {
                QString eacharg2(argv[i+1]);
                MainWindow::use_screen = eacharg2.toInt();
                ++i;
                continue;
            }
            else if (QString::compare(eacharg, "-window", Qt::CaseInsensitive) == 0) {
                MainWindow::window_mode = true;
                continue;
            }
            else if (QString::compare(eacharg, "-width", Qt::CaseInsensitive) == 0 && i < argc-1) {
                QString eacharg2(argv[i+1]);
                MainWindow::window_width = eacharg2.toInt();
                ++i;
                continue;
            }
            else if (QString::compare(eacharg, "-height", Qt::CaseInsensitive) == 0 && i < argc-1) {
                QString eacharg2(argv[i+1]);
                MainWindow::window_height = eacharg2.toInt();
                ++i;
                continue;
            }
            else if (QString::compare(eacharg, "-novsync", Qt::CaseInsensitive) == 0) {
                GLWidget::SetNoVSync(true);
                continue;
            }
            else if ((QString::compare(eacharg, "-render", Qt::CaseInsensitive) == 0 ||
                      QString::compare(eacharg, "-mode", Qt::CaseInsensitive) == 0) && i < argc-1) {
                QString eacharg2(argv[i+1]);
                DisplayMode d = MODE_AUTO;
                if (QString::compare(eacharg2, "sbs", Qt::CaseInsensitive) == 0) {
                    d = MODE_SBS;
                }
                else if (QString::compare(eacharg2, "sbs_reverse", Qt::CaseInsensitive) == 0) {
                    d = MODE_SBS_REVERSE;
                }
                else if (QString::compare(eacharg2, "ou3d", Qt::CaseInsensitive) == 0) {
                    d = MODE_OU3D;
                }
                else if (QString::compare(eacharg2, "cube", Qt::CaseInsensitive) == 0) {
                    d = MODE_CUBE;
                }
                else if (QString::compare(eacharg2, "equi", Qt::CaseInsensitive) == 0) {
                    d = MODE_EQUI;
                    MathUtil::m_do_equi = true;
                }
                else if (QString::compare(eacharg2, "rift", Qt::CaseInsensitive) == 0) {
                    d = MODE_RIFT;
                }
                else if (QString::compare(eacharg2, "vive", Qt::CaseInsensitive) == 0) {
                    d = MODE_VIVE;
                }                
                else {
                    d = MODE_2D;
                }
                GLWidget::SetDisplayMode(d);
                ++i;
                continue;
            }           
            else if (QString::compare(eacharg, "-help", Qt::CaseInsensitive) == 0 ||
                     QString::compare(eacharg, "-h", Qt::CaseInsensitive) == 0) {
                MainWindow::display_help = true;
                continue;
            }
            else if (QString::compare(eacharg, "-version", Qt::CaseInsensitive) == 0 ||
                     QString::compare(eacharg, "-v", Qt::CaseInsensitive) == 0) {
                MainWindow::display_version = true;
                continue;
            }
            else {
                Environment::SetLaunchURLIsCustom(true);
                Environment::SetLaunchURL(QString(argv[i]));
                continue;
            }
        }
    }
}

void ProcessCmdLineArgs2(int argc, char *argv[])
{
    //49.45 - Set command line parameters *after* constructors are called, so we can change defaults/override saved settings
    if (argc > 1) {
        for (int i=1; i<argc; ++i) {
            const QString eacharg(argv[i]);
            if (QString::compare(eacharg, "-server", Qt::CaseInsensitive) == 0 && i < argc-1) {
                QString eacharg2(argv[i+1]);
                SettingsManager::SetServer(eacharg2);
                qDebug() << "Setting server" << eacharg2;
                ++i;
            }
            else if (QString::compare(eacharg, "-port", Qt::CaseInsensitive) == 0&& i < argc-1) {
                QString eacharg2(argv[i+1]);
                SettingsManager::SetPort(eacharg2.toInt());
                ++i;
            }  
            else if (QString::compare(eacharg, "-output_cubemap", Qt::CaseInsensitive) == 0 && i < argc-1) {
                MainWindow::output_cubemap = true;
                MainWindow::output_cubemap_filename_prefix = QString(argv[i+1]);
                ++i;
            }
            else if (QString::compare(eacharg, "-output_equi", Qt::CaseInsensitive) == 0 && i < argc-1) {
                MainWindow::output_equi = true;
                MainWindow::output_cubemap_filename_prefix = QString(argv[i+1]);
                ++i;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    ProcessCmdLineArgs1(argc, argv);

    //If a new default QSurfaceFormat with a modified OpenGL profile has to be set,
    //it should be set before the application instance is declared, to make sure
    //that all created OpenGL contexts use the same OpenGL profile.
    QSurfaceFormat format;    
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setMajorVersion(3); //66.0 - ensure that default vertex/fragment shaders are supported (GLSL 330)
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    if (GLWidget::GetNoVSync() || GLWidget::GetDisplayMode() == MODE_AUTO) {
        format.setSwapInterval(0); //this disables vsync
    }
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);

    //Sets up an OpenGL Context that can be shared between threads. This has to be
    //done after QGuiApplication is created, but before a Qt Quick window is created.
    QtWebEngine::initialize();

    a.setStyle(QStyleFactory::create("fusion"));
    a.setStyleSheet("QMenu {background: #2F363B; color: #FFFFFF; border: 2px solid #FFFFFF;}"                
                    //59.9 - note: use default (padding: 0px) for non-Android builds
                    "QMenu::item:selected { background: #414A51; }"
                    "QMenu::item:disabled { color: #4A5B68; }");

    if (MainWindow::display_help) {                
        DisplayHelp();
        return 0;
    }

    if (MainWindow::display_version) {
        DisplayVersion();
        return 0;
    }

    qDebug() << "CookieJar initialize";
    CookieJar::Initialize();
    qDebug() << "WebAsset initialize";
    WebAsset::Initialize();
    qDebug() << "MathUtil initialize";
    MathUtil::Initialize();      

    MainWindow w;

    //move the window to the screen if needed
    if (MainWindow::use_screen != -1 && MainWindow::use_screen < QApplication::desktop()->screenCount() ) {
        QRect screenres = QApplication::desktop()->screenGeometry(MainWindow::use_screen);
        w.move(QPoint(screenres.x(), screenres.y()));
    }

    if ((MainWindow::use_screen != -1 &&
         MainWindow::use_screen < QApplication::desktop()->screenCount()) ||
            MainWindow::window_width > 0 ||
            MainWindow::window_height > 0) {
        QRect screenres = QApplication::desktop()->screenGeometry(MainWindow::use_screen);
        w.resize((MainWindow::window_width > 0) ? MainWindow::window_width : screenres.width(),
                 (MainWindow::window_height > 0) ? MainWindow::window_height : screenres.height());
    }

    if (SettingsManager::GetDemoModeEnabled() && SettingsManager::GetDemoModeWindowMaximize()) {
        w.showMaximized();
    }
    else {
        w.show();
    }

    qDebug() << "main(): application path " << MathUtil::GetApplicationPath();

    w.Initialize();
    ProcessCmdLineArgs2(argc, argv);

    qDebug() << "main(): writing settings to path" << MathUtil::GetAppDataPath();
    const int ret_val = a.exec();
    return ret_val;
}
