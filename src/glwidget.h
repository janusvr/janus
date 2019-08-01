#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qopengl.h>
#include <qopenglext.h>
#include <QOpenGLWidget>
#include <QtGui>

#include "game.h"
#include "assetobject.h"
#include "renderer.h"

enum DisplayMode {
    MODE_2D,
    MODE_SBS,
    MODE_SBS_REVERSE,
    MODE_OU3D,
    MODE_CUBE,
    MODE_EQUI,
    MODE_RIFT,
    MODE_VIVE,    
    MODE_AUTO
};

class GLWidget : public QOpenGLWidget
{

public:

    GLWidget();
    ~GLWidget();

    void SetGame(Game *  g);
    void SetHMDManager(AbstractHMDManager *h);

    static void SetDisplayMode(const DisplayMode d);
    static DisplayMode GetDisplayMode();

    static void SetNoVSync(const bool b);
    static bool GetNoVSync();

    //these are from GLWidget
    void SetDefaultProjectionPersp(const float fov, const float aspect, const float near_dist, const float far_dist);
    void SetDefaultProjectionOrtho();

    void SetGrab(const bool b);
    bool GetGrab() const;
    QPoint GetWinCentre();
    QPoint GetLocalWinCentre();

    void DoSaveThumb(const QString path);
    void DoSaveScreenshot(const QString path);
    void DoSaveEqui(const QString path);

    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);
    void wheelEvent(QWheelEvent * e);
    void keyPressEvent(QKeyEvent * e);
    void keyReleaseEvent(QKeyEvent * e);
    void leaveEvent(QEvent *);

    void DoBookmark();

    void SetupFramebuffer();

protected:

    void initializeGL();
    void resizeGL(int, int);
    void paintGL();

private:      

    QPoint last_mouse_pos;
    QPoint mouse_pos;
    bool snap_mouse;
    bool grabbed;

    //frametime related
    QTime framerate_time;
    unsigned int counted_frames;

    unsigned int fps;

    QString take_screenshot_path;
    bool take_screenshot;
    bool take_screenshot_cubemap;
    bool take_screenthumb;
    bool take_bookmark;

    GLsizei m_cube_face_width;
    GLsizei m_cube_face_height;

    QPointer <Game> game;
    QPointer <AbstractHMDManager> hmd_manager;   

    static bool novsync;
    static DisplayMode disp_mode;

    QPointer<TextureHandle> m_equi_cubemap_handle;

    QVector<VirtualCamera> cameras;
};

#endif // GLWIDGET_H
