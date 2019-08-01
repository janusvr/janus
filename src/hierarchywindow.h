#ifndef HIERARCHYWINDOW_H
#define HIERARCHYWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"

class HierarchyWindow : public QWidget
{
    Q_OBJECT

public:

    HierarchyWindow(Game * g);

    void Update();
    void keyReleaseEvent(QKeyEvent * e);

public slots:

    void ItemSelectionChanged();
    void RowsInserted(const QModelIndex & parent, int start, int end);

    void CreateObject();
    void CreateText();
    void CreateParagraph();
    void CreateLink();
    void CreateImage();
    void CreateSound();
    void CreateVideo();
    void CreateGhost();
    void CreateParticle();
    void CreateLight();

    void DeleteSelected();

private:

    void Update_Helper(QTreeWidgetItem * parent_item, DOMNode * d);    

    QPushButton create_object_pushbutton;
    QPushButton create_text_pushbutton;
    QPushButton create_paragraph_pushbutton;
    QPushButton create_link_pushbutton;
    QPushButton create_image_pushbutton;
    QPushButton create_sound_pushbutton;
    QPushButton create_video_pushbutton;
    QPushButton create_ghost_pushbutton;
    QPushButton create_particle_pushbutton;
    QPushButton create_light_pushbutton;

    QPushButton delete_object_pushbutton;
    QTreeWidget tree_widget;
    Game * game;
    QPointer <Room> cur_room;
    int cur_room_objects;
    int cur_room_envobjects;
};

#endif // HIERARCHYWINDOW_H
