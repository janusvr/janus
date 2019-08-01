#ifndef NAVIGATIONWINDOW_H
#define NAVIGATIONWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"

class NavigationWindow : public QWidget
{
    Q_OBJECT

public:

    NavigationWindow(Game * g);

    void Update();

public slots:

    void DoUpdate();
    void ItemSelectionChanged();

private:

    void Update_Helper(QTreeWidgetItem * parent_item, QPointer <Room> r);

    QTreeWidget tree_widget;
    Game * game;
    QPointer <Room> cur_room;
    int itemcount;
    bool update;

    QMap <QTreeWidgetItem *, QPointer <Room> > item_node_map;
};


#endif // NAVIGATIONWINDOW_H
