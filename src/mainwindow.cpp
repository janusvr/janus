#include "mainwindow.h"

int MainWindow::use_screen = -1;
bool MainWindow::window_mode = false;
int MainWindow::window_width = 0;
int MainWindow::window_height = 0;
bool MainWindow::display_help = false;
bool MainWindow::display_version = false;
bool MainWindow::output_cubemap = false;
bool MainWindow::output_equi = false;
QString MainWindow::output_cubemap_filename_prefix;

MainWindow::MainWindow()
    : cur_screen(0),
      fullscreened(false)
{
    SettingsManager::LoadSettings();

    //set application-wide font
    const QString font_path = MathUtil::GetApplicationPath() + "assets/fonts/OpenSans-Regular.ttf";
    int opensans_id = QFontDatabase::addApplicationFont(font_path);
    if (!QFontDatabase::applicationFontFamilies(opensans_id).isEmpty()) {
        QString opensans_family = QFontDatabase::applicationFontFamilies(opensans_id).at(0);
        QFont opensans(opensans_family);
        QApplication::setFont(opensans);
    }
    else {
        qDebug() << "Error: Unable to find" << font_path;
    }

    //set mainwindow style
    QString style = "QHeaderView::section {background-color: #59646B;}"
                    "QScrollBar {border: none; background: #3E4D54;}"
                    "QScrollBar::sub-page, QScrollBar::add-page { background: #3E4D54;}"
                    "QScrollBar::sub-line, QScrollBar::add-line {border: none; background: none;}"
                    "QScrollBar::handle {border:none; background: #59646B;}"
                    "QScrollBar::left-arrow, QScrollBar::right-arrow, QScrollBar::up-arrow, QScrollBar::down-arrow {background: none; border: none;}";

    setStyleSheet(style);  //Mainwindow

    setMouseTracking(true);
    setAcceptDrops(true);   

    CloseEventFilter *closeFilter = new CloseEventFilter(this);
    installEventFilter(closeFilter);
    connect(closeFilter, SIGNAL(Closed()), this, SLOT(Closed()));

    //set window title
    QString win_title = QString("Janus VR (");
#ifdef OCULUS_SUBMISSION_BUILD
    win_title+="Oculus build ";
#endif
    win_title += QString(__JANUS_VERSION_COMPLETE)+")";
    setWindowTitle(win_title);

    setMinimumSize(QSize(800, 600));

    //Initialize HMD (if present)
    DisplayMode disp_mode = GLWidget::GetDisplayMode();
#ifdef WIN32        

#ifdef OCULUS_SUBMISSION_BUILD

    hmd_manager = new RiftManager();
    hmd_manager->Initialize();
    if (disp_mode == MODE_AUTO || disp_mode == MODE_RIFT) {
        disp_mode = MODE_RIFT;
        if (!hmd_manager->GetEnabled()) { //could not init Rift, do 2D
            disp_mode = MODE_2D;
        }        
    }

#else

    if (disp_mode == MODE_AUTO) {
        //Attempt Oculus initialization only on mode auto
        hmd_manager = new RiftManager();
        hmd_manager->Initialize();
        disp_mode = MODE_RIFT;

        if (!hmd_manager->GetEnabled()) { //could not init Rift
            delete hmd_manager;
            hmd_manager = new ViveManager();
            hmd_manager->Initialize();
            if (!hmd_manager->GetEnabled()) { //count not init Vive/WMXR
                disp_mode = MODE_2D;
            }
            else {
                disp_mode = MODE_VIVE;
            }
        }
    }
    else if (disp_mode == MODE_VIVE) {
        hmd_manager = new ViveManager();
        hmd_manager->Initialize();

        if (!hmd_manager->GetEnabled()) { //count not init Vive
            disp_mode = MODE_2D;
        }
    }
    else if (disp_mode == MODE_RIFT) {
        hmd_manager = new RiftManager();
        hmd_manager->Initialize();
        disp_mode = MODE_RIFT;

        if (!hmd_manager->GetEnabled()) { //count not init Vive
            disp_mode = MODE_2D;
        }        
    }

#endif

#else
    if (disp_mode == MODE_AUTO || disp_mode == MODE_VIVE) {
        hmd_manager = new ViveManager();
        hmd_manager->Initialize();

        if (!hmd_manager->GetEnabled()) { //count not init Vive
            hmd_manager.clear();
            disp_mode = MODE_2D;
        }        
        else {
            disp_mode = MODE_VIVE;
        }
    }    
#endif   

    GLWidget::SetDisplayMode(disp_mode);

    glwidget = new GLWidget();
    glwidget->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    //set screen size
    QRect screenSize = QApplication::desktop()->screenGeometry();
    int margin_x = int(float(screenSize.width())*0.1f);
    int margin_y = int(float(screenSize.height())*0.1f);
    setGeometry(margin_x, margin_y, screenSize.width()-margin_x*2, screenSize.height()-margin_y*2);

    button_bookmark_state = 0;
    default_window_flags = windowFlags();
}

MainWindow::~MainWindow()
{
    CookieJar::cookie_jar->SaveToDisk();
    SoundManager::Unload();    

    GamepadShutdown();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* )
{

}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* )
{

}

void MainWindow::dropEvent(QDropEvent* event)
{
    asset_window->AddAssetGivenPath(event->mimeData()->text());
    activateWindow();
    glwidget->SetGrab(true);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    //qDebug() << "KEYPRESSEVENT" << e->key();
    if (e->key()==Qt::Key_T && !game->GetPlayerEnteringText() && !urlbar->hasFocus()
            && !codeeditor_window->GetHasFocus() && !properties_window->GetHasFocus()) {
        e->accept();
    }
    else if (e->key() == Qt::Key_F11) {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* e)
{
//    qDebug() << "MainWindow::keyReleaseEvent()";
    if (e->key()==Qt::Key_T && !game->GetPlayerEnteringText() && !urlbar->hasFocus()
            && !codeeditor_window->GetHasFocus() && !properties_window->GetHasFocus()) {
        if (e->modifiers().testFlag(Qt::ControlModifier)) {
            glwidget->SetGrab(true);
            social_window->setVisible(false);
        }
        else {
            glwidget->SetGrab(false);
            social_window->SetFocusOnChatEntry(true);
        }
        e->accept();
    }
    else if ((e->key()==Qt::Key_L && (e->modifiers().testFlag(Qt::ControlModifier))) ||
             e->key()==Qt::Key_F6) {
        glwidget->SetGrab(false);
        urlbar->setFocus();
        urlbar->selectAll();
        e->accept();
    }
    else if (e->key() == Qt::Key_F11) {
        ActionToggleFullscreen();
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

bool MainWindow::focusNextPrevChild(bool )
{
    return false;
}

void MainWindow::Update()
{
    if (game == NULL || game->GetEnvironment() == NULL || game->GetEnvironment()->GetCurRoom() == NULL) {
        return;
    }

    if (!urlbar->hasFocus() && urlbar->text() != game->GetPlayer()->GetProperties()->GetURL()) {
        urlbar->setText(game->GetPlayer()->GetProperties()->GetURL());
    }

    //60.0 - top bar widget visibility
    const bool vis = !(fullscreened && glwidget->GetGrab());
    if (topbarwidget && topbarwidget->isVisible() != vis) {
        topbarwidget->setVisible(vis);
    }

    const int value = game->GetEnvironment()->GetCurRoom()->GetProgress()*100.0f;
    if (progressbar->value() != value) {
        progressbar->setValue(value);
    }

    const QString bookmarked_path = MathUtil::GetApplicationPath() + "assets/icons/bookmarked.png";
    const QString bookmark_path = MathUtil::GetApplicationPath() + "assets/icons/bookmark.png";

    if (game->GetBookmarkManager()->GetBookmarked(game->GetPlayer()->GetProperties()->GetURL())) {
        if (button_bookmark_state != 1) {
            button_bookmark_state = 1;
            button_bookmark->setIcon(QIcon(bookmarked_path));
            button_bookmark->setIconSize(QSize(32,32));
            button_bookmark->setToolTip("Remove bookmark");
        }
    }
    else {
        if (button_bookmark_state != 2) {
            button_bookmark_state = 2;
            button_bookmark->setIcon(QIcon(bookmark_path));
            button_bookmark->setIconSize(QSize(32,32));
            button_bookmark->setToolTip("Add bookmark");
        }
    }

    //grabbing subject to window focus
    const bool active_window = isActiveWindow();
    if (!active_window && glwidget->GetGrab()) {
        glwidget->SetGrab(false);
    }

    //player text entry (URL bar or chat text entry)
    if (social_window && urlbar) {
        game->GetPlayer()->SetTyping(social_window->GetFocusOnChatEntry() || urlbar->hasFocus());
    }

    //Should quit? or not entitled?
    if (hmd_manager && hmd_manager->GetEnabled()) {
        hmd_manager->Platform_ProcessMessages();
        if ((hmd_manager->Platform_GetShouldQuit() || !hmd_manager->Platform_GetEntitled())) {
            game->SetDoExit(true);
        }
    }

    if (social_window->isVisible()) {
        social_window->Update();
    }

    if (codeeditor_window->isVisible()) {
        codeeditor_window->Update();
    }
    if (navigation_window->isVisible()) {
        navigation_window->Update();
    }
    if (hierarchy_window->isVisible()) {
        hierarchy_window->Update();
    }
    if (properties_window->isVisible()) {
        properties_window->Update();
    }
    if (asset_window->isVisible()) {
        asset_window->Update();
    }

    //Controllers (Leap Motion)
    UpdateHands();
}

void MainWindow::TimeOut()
{    
    if (game && game->GetDoExit()) {       
       Closed();
       return;
    }

    //59.3 - makeCurrent call not needed/costly
//    glwidget->makeCurrent();

    //Check for changes to antialiasing setting
    uint32_t sampleCount = (SettingsManager::GetAntialiasingEnabled()) ? 4 : 0;
    Renderer::m_pimpl->ConfigureSamples(sampleCount);

    //Check for changes to enhanced depth precision setting
    Renderer::m_pimpl->SetIsUsingEnhancedDepthPrecision(SettingsManager::GetEnhancedDepthPrecisionEnabled());

    Update();    
    glwidget->update();      
}

void MainWindow::Initialize()
{    
//    qDebug() << "MainWindow::Initialize()";
    glwidget->makeCurrent();

    SetupWidgets();
    SetupMenuWidgets();

    game = new Game();

    glwidget->SetGame(game);    

    social_window = new SocialWindow(game);
    social_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"); //Hover: #3E4D54; Click: #1F2227

    splitter->addWidget(social_window);
    social_window->setVisible(false);

    codeeditor_window = new CodeEditorWindow(game);
    splitter->addWidget(codeeditor_window);
    codeeditor_window->setVisible(false);
    codeeditor_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"
                                     "QCheckBox::indicator:unchecked {color: #FFFFFF; background: #24292D;}"); //Hover: #3E4D54; Click: #1F2227

    navigation_window = new NavigationWindow(game);
    splitter->addWidget(navigation_window);
    navigation_window->setVisible(false);
    navigation_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"); //Hover: #3E4D54; Click: #1F2227

    asset_window = new AssetWindow(game);
    splitter->addWidget(asset_window);
    asset_window->setVisible(false);
    asset_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"
                                "QTableWidget {color: #FFFFFF; background: #24292D;}"); //Hover: #3E4D54; Click: #1F2227

    hierarchy_window = new HierarchyWindow(game);
    splitter->addWidget(hierarchy_window);
    hierarchy_window->setVisible(false);
    hierarchy_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"
                                    "QTableWidget {color: #FFFFFF; background: #24292D;}"); //Hover: #3E4D54; Click: #1F2227

    properties_window = new PropertiesWindow(game);
    splitter->addWidget(properties_window);
    properties_window->setVisible(false);
    properties_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"
                                     "QLineEdit {color: #FFFFFF; background: #24292D;}"
                                     "QCheckBox::indicator:unchecked {color: #FFFFFF; background: #24292D;}"
                                     "QGroupBox::title {color:#62BD6C;}"); //Hover: #3E4D54; Click: #1F2227

    settings_window = new SettingsWindow(game);
    settings_window->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"
                                   "QCheckBox::indicator:unchecked {color: #FFFFFF; background: #24292D;}"
                                   "QLineEdit {color: #FFFFFF; background: #24292D;}"
                                   "QGroupBox::title {color:#62BD6C;}"); //Hover: #3E4D54; Click: #1F2227

    this->centralWidget()->setAcceptDrops(true);

    const DisplayMode disp_mode = GLWidget::GetDisplayMode();

    //60.0 - We keep the hmd_manager pointer in MainWindow for Oculus SDK entitlement check, but only pass it to glwidget and others if an HMD is actually in use
    if (disp_mode == MODE_RIFT || disp_mode == MODE_VIVE) {
        glwidget->SetHMDManager(hmd_manager);
        game->GetControllerManager()->SetHMDManager(hmd_manager);
    }

    if (disp_mode == MODE_VIVE || disp_mode == MODE_RIFT || disp_mode == MODE_CUBE || disp_mode == MODE_EQUI) {
         game->SetMouseDoPitch(false);
    }
    else if (disp_mode == MODE_2D || disp_mode == MODE_SBS || disp_mode == MODE_SBS_REVERSE || disp_mode == MODE_OU3D) {
        game->SetMouseDoPitch(true);
    }      

    switch (disp_mode) {    
    case MODE_VIVE:
        game->GetPlayer()->SetHMDType(hmd_manager->GetHMDString());
        game->GetPlayer()->SetHMDEnabled(true);   
        break;
    case MODE_RIFT:
        game->GetPlayer()->SetHMDType("rift");
        game->GetPlayer()->SetHMDEnabled(true);        
        break;   
    case MODE_SBS:
        game->GetPlayer()->SetHMDType("sbs");
        break;
    case MODE_SBS_REVERSE:
        game->GetPlayer()->SetHMDType("sbs_reverse");
        break;
    case MODE_OU3D:
        game->GetPlayer()->SetHMDType("ou3d");
        break;
    case MODE_CUBE:
        game->GetPlayer()->SetHMDType("cube");
        break;
    default:
        game->GetPlayer()->SetHMDType("2d");
        break;
    }

    game->GetPlayer()->SetDeviceType("desktop");

    qDebug() << "MainWindow::InitializeGame() - HMD/render type:" << game->GetPlayer()->GetHMDType();

    SoundManager::Load(SettingsManager::GetPlaybackDevice(), SettingsManager::GetCaptureDevice());

    //initialize controllers
    GamepadInit();

    if (hmd_manager) {
        glwidget->makeCurrent();
    }

    connect(&timer, SIGNAL(timeout()), this, SLOT(TimeOut()));
    timer.start( 0 );
}

void MainWindow::UpdateHands()
{
//    qDebug() << "MainWindow::UpdateHands()" << leap_controller.isConnected() << hmd_manager << hmd_manager->GetEnabled();
    QPointer <Player> player = game->GetPlayer();
    player->GetHand(0).is_active = false;
    player->GetHand(1).is_active = false;

    if (game->GetControllerManager()->GetUsingSpatiallyTrackedControllers()) { //spatially tracked controller path
        for (int i=0; i<2; ++i) {
            LeapHand & hand = player->GetHand(i);
            hand.is_active = false;
            if (hmd_manager->GetControllerTracked(i)) {                                                
                hand.is_active = true;          
                hand.basis = hmd_manager->GetControllerTransform(i);
            }
        }
    }
}

void MainWindow::Closed()
{        
    qDebug() << "MainWindow::Closed()";      

    disconnect(&timer, 0, 0, 0);

    if (social_window) {
        social_window->Shutdown();
    }

    if (game) {
        delete game;
    }

    QCoreApplication::quit(); //60.0 - important!  Calling QCoreApplication::quit clears the event loop
}

void MainWindow::SetupWidgets()
{
    const unsigned int btn_size = 32 * this->devicePixelRatio();
//    qDebug() << "MainWindow::SetupWidgets()" << btn_size << this->devicePixelRatio();

    button_back = new QPushButton();
    button_back->setIcon(QIcon(MathUtil::GetApplicationPath() + "assets/icons/back.png"));
    button_back->setIconSize(QSize(btn_size,btn_size));
    button_back->setMaximumWidth(btn_size);
    button_back->setMaximumHeight(btn_size);
    button_back->setToolTip("Back");
    connect(button_back, SIGNAL(clicked(bool)), this, SLOT(ActionBack()));

    button_forward = new QPushButton();
    button_forward->setIcon(QIcon(MathUtil::GetApplicationPath() + "assets/icons/forward.png"));
    button_forward->setIconSize(QSize(btn_size,btn_size));
    button_forward->setMaximumWidth(btn_size);
    button_forward->setMaximumHeight(btn_size);
    button_forward->setToolTip("Forward");
    connect(button_forward, SIGNAL(clicked(bool)), this, SLOT(ActionForward()));

    button_reload = new QPushButton();
    button_reload->setIcon(QIcon(MathUtil::GetApplicationPath() + "assets/icons/reload.png"));
    button_reload->setIconSize(QSize(btn_size,btn_size));
    button_reload->setMaximumWidth(btn_size);
    button_reload->setMaximumHeight(btn_size);
    button_reload->setToolTip("Reload");
    connect(button_reload, SIGNAL(clicked(bool)), this, SLOT(ActionReload()));

    button_home = new QPushButton();
    button_home->setIcon(QIcon(MathUtil::GetApplicationPath() + "assets/icons/home.png"));
    button_home->setIconSize(QSize(btn_size,btn_size));
    button_home->setMaximumWidth(btn_size);
    button_home->setMaximumHeight(btn_size);
    button_home->setToolTip("Go to home URL and reset environment.");
    connect(button_home, SIGNAL(clicked(bool)), this, SLOT(ActionHome()));

    urlbar = new QLineEdit();
    urlbar->setStyleSheet("color:#62BD6C; padding-left:16px;");
    connect(urlbar, SIGNAL(returnPressed()), this, SLOT(ActionOpenURL()));

    progressbar = new QProgressBar();
    progressbar->setRange(0, 100);
    progressbar->setValue(0);
    progressbar->setMaximumHeight(btn_size/8);
    progressbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    progressbar->setTextVisible(false);
    progressbar->setStyleSheet("QProgressBar::chunk {background-color: #62BD6C;}");

    button_bookmark = new QPushButton();
    button_bookmark->setIcon(QIcon(MathUtil::GetApplicationPath() + "assets/icons/bookmark.png"));
    button_bookmark->setIconSize(QSize(btn_size,btn_size));
    button_bookmark->setMinimumWidth(btn_size);
    button_bookmark->setMinimumHeight(btn_size);
    button_bookmark->setMaximumWidth(btn_size);
    button_bookmark->setMaximumHeight(btn_size);
    button_bookmark->setToolTip("Bookmark");
    connect(button_bookmark, SIGNAL(clicked(bool)), this, SLOT(ActionBookmark()));

    button_ellipsis = new QPushButton();
    button_ellipsis->setIcon(QIcon(MathUtil::GetApplicationPath() + "assets/icons/menu.png"));
    button_ellipsis->setIconSize(QSize(btn_size,btn_size));
    button_ellipsis->setMaximumWidth(btn_size);
    button_ellipsis->setMaximumHeight(btn_size);
    button_ellipsis->setToolTip("Customize and control Janus VR");
    connect(button_ellipsis, SIGNAL(clicked(bool)), this, SLOT(ActionEllipsisMenu()));

    QWidget * w4 = new QWidget();
    QVBoxLayout * l4 = new QVBoxLayout();
    l4->addWidget(urlbar, 1);
    l4->addWidget(progressbar);
    l4->setSpacing(0);
    l4->setMargin(0);
    w4->setLayout(l4);
    w4->setMaximumHeight(btn_size);

    topbarwidget = new QWidget();
    QHBoxLayout * l3 = new QHBoxLayout();
    l3->addWidget(button_back);
    l3->addWidget(button_forward);
    l3->addWidget(button_reload);
    l3->addWidget(button_home);
    l3->addWidget(w4);
    l3->addWidget(button_bookmark);
    l3->addWidget(button_ellipsis);
    l3->setSpacing(0);
    l3->setMargin(0);
    topbarwidget->setLayout(l3);
    topbarwidget->setMaximumHeight(btn_size);
    topbarwidget->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"
                        "QPushButton:hover, QPushButton:selected {background: #3E4D54;}" //Hover: #3E4D54; Click: #1F2227;
                        "QPushButton::menu-indicator {image: url("");}"); //Hover: #3E4D54; Click: #1F2227;


    QVBoxLayout * l2 = new QVBoxLayout();
    l2->setSpacing(0);
    l2->setMargin(0);    
    l2->addWidget(topbarwidget);
    l2->addWidget(glwidget);

    splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);

    QWidget * w = new QWidget();
    w->setLayout(l2);
    splitter->addWidget(w);

    splitter->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}");
    this->setCentralWidget(splitter);
}

void MainWindow::SetupMenuWidgets()
{
    newAct = new QAction(tr("&New..."), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new workspace"));
    connect(newAct, &QAction::triggered, this, &MainWindow::ActionNew);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open file or workspace"));
    connect(openAct, &QAction::triggered, this, &MainWindow::ActionOpen);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save current workspace"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::ActionSave);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save current workspace as"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::ActionSaveAs);

    importLocalAct = new QAction(tr("&Import (Local file)..."), this);
    importLocalAct->setStatusTip(tr("Import JML code from a local HTML file"));
    connect(importLocalAct, &QAction::triggered, this, &MainWindow::ActionImportLocal);

    importRemoteAct = new QAction(tr("Import (&Remote URL)..."), this);
    importRemoteAct->setStatusTip(tr("Import JML code from a remote HTML file"));
    connect(importRemoteAct, &QAction::triggered, this, &MainWindow::ActionImportRemote);

    saveThumbAct = new QAction(tr("Save Thumbnail"), this);
    saveThumbAct->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F7));
    saveThumbAct->setStatusTip(tr("Save thumbnail image"));
    connect(saveThumbAct, &QAction::triggered, this, &MainWindow::ActionSaveThumb);

    saveScreenshotAct = new QAction(tr("Save Screenshot"), this);
    saveScreenshotAct->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F8));
    saveScreenshotAct->setStatusTip(tr("Save screenshot"));
    connect(saveScreenshotAct, &QAction::triggered, this, &MainWindow::ActionSaveScreenshot);

    saveEquiAct = new QAction(tr("Save Equirectangular"), this);
    saveEquiAct->setShortcut(QKeySequence(Qt::SHIFT+Qt::CTRL + Qt::Key_F8));
    saveEquiAct->setStatusTip(tr("Save equirectangular image"));
    connect(saveEquiAct, &QAction::triggered, this, &MainWindow::ActionSaveEqui);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit Janus VR"));
    connect(exitAct, &QAction::triggered, this, &MainWindow::ActionExit);

    startRecordingAct = new QAction(tr("Start Recording"), this);
    startRecordingAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    startRecordingAct->setStatusTip(tr("Start Recording"));
    connect(startRecordingAct, &QAction::triggered, this, &MainWindow::ActionStartRecording);

    startRecordingEveryoneAct = new QAction(tr("Start Recording (All users)"), this);
    startRecordingEveryoneAct->setStatusTip(tr("Start Recording (All users)"));
    connect(startRecordingEveryoneAct, &QAction::triggered, this, &MainWindow::ActionStartRecordingEveryone);

    stopRecordingAct = new QAction(tr("Stop Recording"), this);
    stopRecordingAct->setStatusTip(tr("Stop Recording"));
    connect(stopRecordingAct, &QAction::triggered, this, &MainWindow::ActionStopRecording);

    syncToAct = new QAction(tr("Sync to"), this);
    syncToAct->setStatusTip(tr("Sync to"));
    connect(syncToAct, &QAction::triggered, this, &MainWindow::ActionSyncTo);

    settingsAct = new QAction(tr("&Settings..."), this);    
    settingsAct->setStatusTip(tr("Settings..."));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::ActionSettings);

    virtualMenuAct = new QAction(tr("Virtual Menu"), this);
//    virtualMenuAct->setShortcut(QKeySequence(Qt::Key_Tab)); //62.0 - enabling this causes TAB not to work in edit mode
    virtualMenuAct->setStatusTip(tr("Show/hide virtual menu"));
    connect(virtualMenuAct, &QAction::triggered, this, &MainWindow::ActionVirtualMenu);

    toggleFullscreenAct = new QAction(tr("Fullscreen"), this);
    toggleFullscreenAct->setStatusTip(tr("Toggle Fullscreen Mode"));
    connect(toggleFullscreenAct, &QAction::triggered, this, &MainWindow::ActionToggleFullscreen);

    socialAct = new QAction(tr("Social"), this);
    socialAct->setCheckable(true);
    socialAct->setChecked(false);
    socialAct->setStatusTip(tr("Show social pane"));
    connect(socialAct, &QAction::triggered, this, &MainWindow::ActionSocial);

    codeEditorAct = new QAction(tr("Code Editor"), this);
    codeEditorAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    codeEditorAct->setCheckable(true);
    codeEditorAct->setChecked(false);
    codeEditorAct->setStatusTip(tr("Show code editor"));
    connect(codeEditorAct, &QAction::triggered, this, &MainWindow::ActionCodeEditor);

    navigationAct = new QAction(tr("Navigation"), this);
    navigationAct->setCheckable(true);
    navigationAct->setChecked(false);
    navigationAct->setStatusTip(tr("Show navigation pane"));
    connect(navigationAct, &QAction::triggered, this, &MainWindow::ActionNavigation);

    assetAct = new QAction(tr("Assets"), this);
    assetAct->setCheckable(true);
    assetAct->setChecked(false);
    assetAct->setStatusTip(tr("Show/hide assets"));
    connect(assetAct, &QAction::triggered, this, &MainWindow::ActionAssets);

    hierarchyAct = new QAction(tr("Room Objects"), this);
    hierarchyAct->setCheckable(true);
    hierarchyAct->setChecked(false);
    hierarchyAct->setStatusTip(tr("Show/hide hierarchy"));
    connect(hierarchyAct, &QAction::triggered, this, &MainWindow::ActionHierarchy);

    propertiesAct = new QAction(tr("Properties"), this);
    propertiesAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    propertiesAct->setCheckable(true);
    propertiesAct->setChecked(false);
    propertiesAct->setStatusTip(tr("Show/hide properties"));
    connect(propertiesAct, &QAction::triggered, this, &MainWindow::ActionProperties);

    addBookmarkAct = new QAction("Add Bookmark", this);
    connect(addBookmarkAct, SIGNAL(triggered(bool)), this, SLOT(ActionAddBookmark()));

    removeBookmarkAct = new QAction("Remove Bookmark", this);
    connect(removeBookmarkAct, SIGNAL(triggered(bool)), this, SLOT(ActionRemoveBookmark()));

    bookmarkMenu = new QMenu("Bookmarks", this);
    connect(bookmarkMenu, SIGNAL(aboutToShow()), this, SLOT(ActionOpenBookmarks()));
    connect(bookmarkMenu, SIGNAL(triggered(QAction*)), this, SLOT(ActionOpenURL(QAction *)));

    fileMenu = new QMenu("File", this);
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(importLocalAct);
    fileMenu->addAction(importRemoteAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveThumbAct);

    windowMenu = new QMenu("Window", this);
    windowMenu->addAction(codeEditorAct);
    windowMenu->addSeparator();
    windowMenu->addAction(socialAct);
    windowMenu->addAction(navigationAct);
    windowMenu->addSeparator();
    windowMenu->addAction(assetAct);
    windowMenu->addAction(hierarchyAct);
    windowMenu->addAction(propertiesAct);
    windowMenu->addSeparator();
    windowMenu->addAction(codeEditorAct);
    windowMenu->addSeparator();
    windowMenu->addSeparator();
    windowMenu->addAction(toggleFullscreenAct);
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(ActionOpenWindow()));

    usersMenu = new QMenu("Users", this);
    usersMenu->addAction(startRecordingAct);
    usersMenu->addAction(startRecordingEveryoneAct);
    usersMenu->addAction(stopRecordingAct);
    usersMenu->addSeparator();
    usersMenu->addAction(syncToAct);
    connect(usersMenu, SIGNAL(aboutToShow()), this, SLOT(ActionOpenEdit()));

    ellipsisMenu = new QMenu(this);
    ellipsisMenu->addMenu(fileMenu);

    ellipsisMenu->addMenu(windowMenu);
    ellipsisMenu->addMenu(usersMenu);

    ellipsisMenu->addMenu(bookmarkMenu);    
    ellipsisMenu->addSeparator();
    ellipsisMenu->addAction(settingsAct);
    ellipsisMenu->addAction(virtualMenuAct);
    ellipsisMenu->addAction(exitAct);

    button_ellipsis->setMenu(ellipsisMenu);
}

QString MainWindow::GetNewWorkspaceDirectory()
{
    QDir d;
    int val = 0;
    while (true) {
        ++val;
        //check that workspace directory does not exist
        d.setPath(MathUtil::GetWorkspacePath() + "workspace" +
               QString("%1").arg(val, 3, 10, QChar('0')));
        if (!d.exists()) {
            break;
        }
    }
    return d.path();
}

void MainWindow::ActionNew()
{
    game->CreateNewWorkspace(GetNewWorkspaceDirectory());
}

void MainWindow::ActionOpen()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open...", MathUtil::GetWorkspacePath(), tr("HTML (*.html)"));
    if (!filename.isNull()) {
        QString local_file = QUrl::fromLocalFile(filename).toString();
//        qDebug() << QUrl(filename).isLocalFile() << filename << local_file;
        game->CreatePortal(local_file, false);
    }
}

void MainWindow::ActionSave()
{
    //game->SaveRoom(game->getEnvironment()->GetPlayerRoom()->GetSaveFilename());
    codeeditor_window->SlotSaveChanges();
}

void MainWindow::ActionSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save As...", MathUtil::GetWorkspacePath(), tr("HTML (*.html);; JSON (*.json)"));
    if (!filename.isNull()) {
        game->SaveRoom(filename);
    }
}

void MainWindow::ActionImportLocal()
{
    QString filename = QFileDialog::getOpenFileName(this, "Import (local file)...", MathUtil::GetWorkspacePath(), tr("HTML (*.html)"));
    if (!filename.isNull()) {
        QString local_file = QUrl::fromLocalFile(filename).toString();
        game->DoImport(local_file);
    }
}

void MainWindow::ActionImportRemote()
{
    bool ok;
    QString url = QInputDialog::getText(this, tr("Import (Remote URL)"),
                                         tr("Specify URL of HTML file (local or remote) containing JML code:"),
                                         QLineEdit::Normal,
                                         "http://www.janusvr.com", &ok);
    if (ok && !url.isEmpty()) {
        game->DoImport(url);
    }
}

void MainWindow::ActionSaveThumb()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Thumbnail As...", MathUtil::GetScreenshotPath(), tr("JPEG (*.jpg)"));
    if (!filename.isNull()) {
        glwidget->DoSaveThumb(filename);
    }
}

void MainWindow::ActionSaveScreenshot()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Screenshot As...", MathUtil::GetScreenshotPath(), tr("JPEG (*.jpg)"));

    if (!filename.isNull()) {
        glwidget->DoSaveScreenshot(filename);
    }
}

void MainWindow::ActionSaveEqui()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Equirectangular As...", MathUtil::GetScreenshotPath(), tr("JPEG (*.jpg)"));

    if (!filename.isNull()) {
        glwidget->DoSaveEqui(filename);
    }
}

void MainWindow::ActionExit()
{
    game->SetDoExit(true);
}

void MainWindow::ActionBack()
{
    game->StartResetPlayer();
}

void MainWindow::ActionForward()
{
    game->StartResetPlayerForward();
}

void MainWindow::ActionReload()
{
    game->GetEnvironment()->ReloadRoom();
}

void MainWindow::ActionHome()
{
    game->GetEnvironment()->Reset();
}

void MainWindow::ActionBookmark()
{
    const QString url = game->GetPlayer()->GetProperties()->GetURL();
    const bool url_bookmarked = game->GetBookmarkManager()->GetBookmarked(url);
    url_bookmarked ? ActionRemoveBookmark() : ActionAddBookmark();
}

void MainWindow::ActionOpenURL()
{
    DoOpenURL(urlbar->text());
}

void MainWindow::ActionOpenURL(QAction * a)
{
    if (a && a != addBookmarkAct && a != removeBookmarkAct) {        
        DoOpenURL(a->text());
    }
}

void MainWindow::DoOpenURL(const QString url)
{    
    //59.0 - path might be copy-pasted from e.g. windows explorer
    //as a local (non URL) path, correct accordingly
    QString s = url;
    QFileInfo f(QUrl(url).toLocalFile());
    const bool local_file = f.exists();
    if (local_file) {
        //s = QUrl::fromLocalFile(s).toString();
    }
    else if (s.toLower() == "home") {
        s = SettingsManager::GetLaunchURL();
    }
    else if (s.toLower() == "bookmarks" || s.toLower() == "workspaces") {

    }
    else if (s.left(4).toLower() != "http") {
        s = "http://" + s;
    }

//    qDebug() << "MainWindow::DoOpenURL" << url << s;
    game->CreatePortal(s, !local_file);
    glwidget->SetGrab(true);
}

void MainWindow::ActionSocial()
{
    social_window->setVisible(socialAct->isChecked());
}

void MainWindow::ActionVirtualMenu()
{
    //62.0 - do not show menu when in edit mode or doing something else
    if (game->GetState() == JVR_STATE_DEFAULT) {
        game->GetVirtualMenu()->MenuButtonPressed();
        glwidget->SetGrab(true);
    }
}

void MainWindow::ActionCodeEditor()
{
    codeeditor_window->setVisible(codeEditorAct->isChecked());
}

void MainWindow::ActionNavigation()
{
    navigation_window->setVisible(navigationAct->isChecked());
}

void MainWindow::ActionAssets()
{
    asset_window->setVisible(assetAct->isChecked());
}

void MainWindow::ActionHierarchy()
{
    hierarchy_window->setVisible(hierarchyAct->isChecked());
}

void MainWindow::ActionProperties()
{
    properties_window->setVisible(propertiesAct->isChecked());
}

void MainWindow::ActionOpenEdit()
{
    const bool rec = game->GetRecording();
    if (rec) {
        startRecordingAct->setShortcut(QKeySequence());
        stopRecordingAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_G));
    }
    else {
        startRecordingAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_G));
        stopRecordingAct->setShortcut(QKeySequence());
    }
    startRecordingAct->setEnabled(!rec);
    startRecordingEveryoneAct->setEnabled(!rec);
    stopRecordingAct->setEnabled(rec);
}

void MainWindow::ActionStartRecording()
{
    game->StartRecording(false);
    ActionOpenEdit();
}

void MainWindow::ActionStartRecordingEveryone()
{
    game->StartRecording(true);
    ActionOpenEdit();
}

void MainWindow::ActionStopRecording()
{
    game->StopRecording();
    ActionOpenEdit();
}

void MainWindow::ActionSyncTo()
{
    game->GetEnvironment()->GetCurRoom()->SyncAll();
}

void MainWindow::ActionOpenWindow()
{
    socialAct->setChecked(social_window->isVisible());
    codeEditorAct->setChecked(codeeditor_window->isVisible());
    hierarchyAct->setChecked(hierarchy_window->isVisible());
    propertiesAct->setChecked(properties_window->isVisible());
    assetAct->setChecked(asset_window->isVisible());
    navigationAct->setChecked(navigation_window->isVisible());
}

void MainWindow::ActionAddBookmark()
{
    glwidget->DoBookmark();
}

void MainWindow::ActionRemoveBookmark()
{
    glwidget->DoBookmark();
}

void MainWindow::ActionOpenBookmarks()
{
    const QString url = game->GetPlayer()->GetProperties()->GetURL();
    const bool url_bookmarked = game->GetBookmarkManager()->GetBookmarked(url);
    if (url_bookmarked) {
        addBookmarkAct->setShortcut(QKeySequence());
        removeBookmarkAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_B));
    }
    else {
        addBookmarkAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_B));
        removeBookmarkAct->setShortcut(QKeySequence());
    }
    addBookmarkAct->setEnabled(!url_bookmarked);
    removeBookmarkAct->setEnabled(url_bookmarked);

    bookmarkMenu->clear(); //existing actions are deleted!
    bookmarkMenu->addAction(addBookmarkAct);
    bookmarkMenu->addAction(removeBookmarkAct);
    bookmarkMenu->addSeparator();

    QVariantList list = game->GetBookmarkManager()->GetBookmarks();
    for (int i=0; i<list.size(); ++i) {
        QMap <QString, QVariant> o = list[i].toMap();
        QString url = o["url"].toString();
        QString thumbnail = o["thumbnail"].toString();

        QAction * a = new QAction(QIcon(QUrl(thumbnail).toLocalFile()), url, bookmarkMenu);
        bookmarkMenu->addAction(a);
    }
    bookmarkMenu->addSeparator();
    list = game->GetBookmarkManager()->GetWorkspaces();
    for (int i=0; i<list.size(); ++i) {
        QMap <QString, QVariant> o = list[i].toMap();
        QString url = o["url"].toString();
        QString thumbnail = o["thumbnail"].toString();

        QAction * a = new QAction(QIcon(QUrl(thumbnail).toLocalFile()), QUrl::fromLocalFile(url).toString(), bookmarkMenu);
        bookmarkMenu->addAction(a);
    }

}

void MainWindow::ActionSettings()
{
    if (fullscreened) {
        ActionToggleFullscreen();
    }
    settings_window->setVisible(true);
    settings_window->Update();
}

void MainWindow::ActionToggleFullscreen()
{
    QScreen * s = QApplication::primaryScreen();
    if (s == NULL) {
        return;
    }

    const QRect r = s->geometry();
    if (fullscreened) {
        setWindowFlags( default_window_flags );
        show();
        setGeometry(r.left()+100, r.top()+100, r.width()-200, r.height()-200);
    }
    else {
        default_window_flags = windowFlags();
        setWindowFlags( Qt::CustomizeWindowHint);
        show();
        setGeometry(s->availableGeometry());
    }
    fullscreened = !fullscreened;
}

void MainWindow::ActionEllipsisMenu()
{
    ellipsisMenu->show();
}
