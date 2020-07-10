#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qopengl.h>
#include <qopenglext.h>

#include <QKeyEvent>

#include "renderer.h"

#include "soundmanager.h"
#include "abstracthmdmanager.h"
#include "performancelogger.h"

#include "controllermanager.h"

#include "socialwindow.h"
#include "settingswindow.h"
#include "hierarchywindow.h"
#include "propertieswindow.h"
#include "assetwindow.h"
#include "codeeditorwindow.h"
#include "navigationwindow.h"
#include "glwidget.h"

#ifdef WIN32
#include "riftmanager.h"

#ifndef OCULUS_SUBMISSION_BUILD
#include "vivemanager.h"
#endif

#elif defined __linux__    
#include "vivemanager.h"
#endif

class CloseEventFilter : public QObject
{
     Q_OBJECT
public:
     CloseEventFilter(QObject *parent) : QObject(parent) {}

signals:
    void Closed();

protected:
     bool eventFilter(QObject *obj, QEvent *event)
     {
          if (event->type() == QEvent::Close)
          {
              // Do something interesting, emit a signal for instance.
              emit Closed();
          }

          return QObject::eventFilter(obj, event);
     }

};

//class MainWindow : public QWindow
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow();
    ~MainWindow();

    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent* event);

    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);

    bool focusNextPrevChild(bool next);

    void Initialize();

    static int use_screen;
    static bool window_mode;
    static int window_width;
    static int window_height;
    static bool display_help;        
    static bool display_version;
    static bool output_cubemap;
    static bool output_equi;
    static QString output_cubemap_filename_prefix;

public slots:

    void Closed();
    void TimeOut();    

    void ActionNew();
    void ActionOpen();
    void ActionOpenURL();
    void ActionOpenURL(QAction * a);
    void ActionSave();
    void ActionSaveAs();
    void ActionImportLocal();
    void ActionImportRemote();
    void ActionExportAFrame();
    void ActionSaveThumb();
    void ActionSaveScreenshot();
    void ActionSaveEqui();
    void ActionExit();

    void ActionBack();
    void ActionForward();
    void ActionReload();
    void ActionHome();
    void ActionBookmark();

    void ActionOpenEdit();
    void ActionStartRecording();
    void ActionStartRecordingEveryone();
    void ActionStopRecording();
    void ActionSyncTo();

    void ActionOpenWindow();
    void ActionToggleFullscreen();
    void ActionVirtualMenu();
    void ActionSocial();
    void ActionCodeEditor();
    void ActionNavigation();
    void ActionAssets();
    void ActionHierarchy();
    void ActionProperties();

    void ActionOpenBookmarks();
    void ActionAddBookmark();
    void ActionRemoveBookmark();

    void ActionSettings();
    void ActionEllipsisMenu();

    
private:

    void DoSFXDisplayType( QString dm );
    void DoOpenURL(const QString url);
    void SetupMenuWidgets();
    void SetupWidgets();

    void Update();    
    void UpdateHands();

    QString GetNewWorkspaceDirectory();

    QTimer timer;

    QPointer <Game> game;
    QPointer <AbstractHMDManager> hmd_manager;
    QPointer <QOpenGLDebugLogger> opengl_debug_logger;

    int cur_screen;
    bool fullscreened;

    QPointer <SocialWindow> social_window;
    QPointer <SettingsWindow> settings_window;
    QPointer <CodeEditorWindow> codeeditor_window;
    QPointer <HierarchyWindow> hierarchy_window;
    QPointer <PropertiesWindow> properties_window;
    QPointer <AssetWindow> asset_window;
    QPointer <NavigationWindow> navigation_window;

    QMenu * ellipsisMenu;
    QPushButton * button_ellipsis;

    QWidget * topbarwidget;
    QPushButton * button_back;    
    QPushButton * button_forward;
    QPushButton * button_reload;    
    QPushButton * button_home;
    QLineEdit * urlbar;
    QProgressBar * progressbar;
    QPushButton * button_bookmark;
    int button_bookmark_state;
    QPointer <GLWidget> glwidget;
    QMenu *fileMenu;
    QMenu *usersMenu;
    QMenu *windowMenu;
    QMenu *bookmarkMenu;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *importLocalAct;
    QAction *importRemoteAct;
    QAction *exportAFrameAct;
    QAction *saveThumbAct;
    QAction *saveScreenshotAct;
    QAction *saveEquiAct;
    QAction *exitAct;
    QAction *socialAct;
    QAction *startRecordingAct;
    QAction *startRecordingEveryoneAct;
    QAction *stopRecordingAct;
    QAction *syncToAct;
    QAction *settingsAct;
    QAction *virtualMenuAct;
    QAction *codeEditorAct;
    QAction *navigationAct;
    QAction *hierarchyAct;
    QAction *propertiesAct;
    QAction *assetAct;
    QAction *addBookmarkAct;
    QAction *removeBookmarkAct;
    QAction *toggleFullscreenAct;

    QSplitter * splitter;

    bool repaint_queued;
    Qt::WindowFlags default_window_flags;
};

#endif // MAINWINDOW_H
