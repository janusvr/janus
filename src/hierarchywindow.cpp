#include "hierarchywindow.h"

HierarchyWindow::HierarchyWindow(Game * g) :
    game(g),
    cur_room_objects(-1),
    cur_room_envobjects(-1)
{

    tree_widget.setSelectionMode(QAbstractItemView::SingleSelection);
    tree_widget.setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_widget.setDragEnabled(true);
    tree_widget.viewport()->setAcceptDrops(true);
    tree_widget.setDropIndicatorShown(true);
    tree_widget.setDragDropMode(QAbstractItemView::InternalMove);
    tree_widget.setSortingEnabled(true);
    tree_widget.sortItems(0, Qt::AscendingOrder);

    tree_widget.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QStringList s;
    s.push_back("js_id");
    s.push_back("type");
    s.push_back("id");

    tree_widget.setColumnCount(s.size());
    tree_widget.setHeaderLabels(s);
    tree_widget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    create_object_pushbutton.setText("Object");
    create_object_pushbutton.setMaximumHeight(30);

    create_text_pushbutton.setText("Text");
    create_text_pushbutton.setMaximumHeight(30);

    create_paragraph_pushbutton.setText("Paragraph");
    create_paragraph_pushbutton.setMaximumHeight(30);

    create_link_pushbutton.setText("Link");
    create_link_pushbutton.setMaximumHeight(30);

    create_image_pushbutton.setText("Image");
    create_image_pushbutton.setMaximumHeight(30);

    create_sound_pushbutton.setText("Sound");
    create_sound_pushbutton.setMaximumHeight(30);

    create_video_pushbutton.setText("Video");
    create_video_pushbutton.setMaximumHeight(30);

    create_ghost_pushbutton.setText("Ghost");
    create_ghost_pushbutton.setMaximumHeight(30);

    create_particle_pushbutton.setText("Particle");
    create_particle_pushbutton.setMaximumHeight(30);

    create_light_pushbutton.setText("Light");
    create_light_pushbutton.setMaximumHeight(30);

    delete_object_pushbutton.setText("Remove Selected");
    delete_object_pushbutton.setMaximumHeight(30);

//    QWidget * w = new QWidget(this);
    QGridLayout * v = new QGridLayout();
    v->addWidget(&create_object_pushbutton,0,0);
    v->addWidget(&create_text_pushbutton,0,1);
    v->addWidget(&create_paragraph_pushbutton,1,0);
    v->addWidget(&create_link_pushbutton,1,1);
    v->addWidget(&create_image_pushbutton,2,0);
    v->addWidget(&create_sound_pushbutton,2,1);
    v->addWidget(&create_video_pushbutton,3,0);
    v->addWidget(&create_ghost_pushbutton,3,1);
    v->addWidget(&create_particle_pushbutton,4,0);
    v->addWidget(&create_light_pushbutton,4,1);
    v->addWidget(&delete_object_pushbutton,5,0,1,2);

    v->addWidget(&tree_widget,6,0,1,2);
//    w->setLayout(v);
    v->setSpacing(0);
    v->setMargin(1);

//    setWindowTitle("Hierarchy");
//    setMinimumSize(300, 400);
    //setCentralWidget(w);
    this->setLayout(v);

    connect(&tree_widget, SIGNAL(itemSelectionChanged()), this, SLOT(ItemSelectionChanged()));
    connect(tree_widget.model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(RowsInserted(const QModelIndex &, int, int)));
    connect(&create_object_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateObject()));
    connect(&create_text_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateText()));
    connect(&create_paragraph_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateParagraph()));
    connect(&create_link_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateLink()));
    connect(&create_image_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateImage()));
    connect(&create_sound_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateSound()));
    connect(&create_video_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateVideo()));
    connect(&create_ghost_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateGhost()));
    connect(&create_particle_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateParticle()));
    connect(&create_light_pushbutton, SIGNAL(clicked(bool)), this, SLOT(CreateLight()));
    connect(&delete_object_pushbutton, SIGNAL(clicked(bool)), this, SLOT(DeleteSelected()));
}

void HierarchyWindow::Update()
{
    bool do_update = false;
    QPointer <Room> r = game->GetEnvironment()->GetCurRoom();
    if (r != cur_room) {
        cur_room = r;
        do_update = true;
    }
    if (cur_room) {
        if (cur_room_objects != cur_room->GetProperties()->GetChildren().size()) {
            do_update = true;
        }
        cur_room_objects = cur_room->GetProperties()->GetChildren().size();

        const int num = cur_room->GetRoomObjects().size();
        if (cur_room_envobjects != num) {
            do_update = true;
        }
        cur_room_envobjects = num;

        //makes update after renaming a jsid
        if (tree_widget.selectedItems().size() == 1 && tree_widget.selectedItems().first()->text(0) != game->GetSelected(0)) {
            do_update = true;
        }

        //makes update after deletion of a child
        if (tree_widget.selectedItems().size() == 1) {
            const QString s = tree_widget.selectedItems().first()->text(0);
            if (cur_room->GetRoomObject(s) && cur_room->GetRoomObject(s)->GetChildObjects().size() != tree_widget.selectedItems().first()->childCount()) {
                do_update = true;
            }
        }
    }

    if (do_update) {

        tree_widget.blockSignals(true);
        tree_widget.model()->blockSignals(true);

        tree_widget.clear();

        if (cur_room) {
            Update_Helper(NULL, cur_room->GetProperties());
        }

        tree_widget.blockSignals(false);
        tree_widget.model()->blockSignals(false);
    }

    const QString sel = game->GetSelected(0);
    if (sel.length() > 0) {
        if (tree_widget.selectedItems().isEmpty() || tree_widget.selectedItems().first()->text(0) != sel) {
            QList <QTreeWidgetItem *> items = tree_widget.findItems(sel, Qt::MatchExactly | Qt::MatchRecursive, 0);
//            qDebug() << "find it?" << sel << items.size();
            if (!items.isEmpty()) {
                tree_widget.clearSelection();
                items.first()->setSelected(true);
                tree_widget.scrollToItem(items.first());
            }

        }
    }
}

void HierarchyWindow::Update_Helper(QTreeWidgetItem * p, DOMNode * d)
{
    //for each child, call update_helper
    QList <DOMNode *> c = d->GetChildren();
    for (int i=0; i<c.size(); ++i) {
        QTreeWidgetItem * item;
        if (p == NULL) {
            item = new QTreeWidgetItem(&tree_widget);
        }
        else {
            item = new QTreeWidgetItem(p);
        }
        item->setText(0, c[i]->GetJSID());
        item->setText(1, c[i]->GetTypeAsString());
        item->setText(2, c[i]->GetID());

        Update_Helper(item, c[i]);
    }
}

void HierarchyWindow::CreateObject()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y());

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_OBJECT);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetID("cube");
    o->GetProperties()->SetCollisionID("cube");
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetInterpolate(true);
    o->SetInterpolation();

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);


//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
//    game->SetState(JVR_STATE_)
}

void HierarchyWindow::CreateText()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y() + 1.0f);

    QVector3D d = -game->GetPlayer()->GetProperties()->GetDir()->toQVector3D();
    d.setY(0.0f);
    d.normalize();

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_TEXT);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetDir(d);
    o->GetProperties()->SetText("T");

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
    game->SetState(JVR_STATE_EDIT_TEXT);
}

void HierarchyWindow::CreateParagraph()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y() + 1.0f);

    QVector3D d = -game->GetPlayer()->GetProperties()->GetDir()->toQVector3D();
    d.setY(0.0f);
    d.normalize();

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_PARAGRAPH);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetDir(d);
    o->GetProperties()->SetText("Paragraph");

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateLink()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y());

    QVector3D d = -game->GetPlayer()->GetProperties()->GetDir()->toQVector3D();
    d.setY(0.0f);
    d.normalize();

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_LINK);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetDir(d);
    o->GetProperties()->SetURL("https://janusxr.org");
    o->GetProperties()->SetTitle("Janus VR");

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateImage()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y() + 1.0f);

    QVector3D d = -game->GetPlayer()->GetProperties()->GetDir()->toQVector3D();
    d.setY(0.0f);
    d.normalize();

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_IMAGE);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetDir(d);

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateSound()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y() + 1.0f);

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_SOUND);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateVideo()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y() + 1.0f);

    QVector3D d = -game->GetPlayer()->GetProperties()->GetDir()->toQVector3D();
    d.setY(0.0f);
    d.normalize();

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_VIDEO);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetDir(d);

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateGhost()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y());

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_GHOST);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateParticle()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y());

    //set defaults to show it operating
    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_PARTICLE);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetID("cube");
    o->GetProperties()->SetScale(QVector3D(0.25f,0.25f,0.25f));
    o->GetProperties()->SetVel(QVector3D(-0.5f,1,-0.5f));
    o->GetProperties()->SetRandVel(QVector3D(1.0f, 0.0f, 1.0f));
    o->GetProperties()->SetDuration(5.0f);
    o->GetProperties()->SetRate(2.0f);
    o->GetProperties()->SetCount(100);
    o->GetProperties()->SetLoop(true);

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::CreateLight()
{
    QVector3D p = game->GetPlayer()->GetProperties()->GetPos()->toQVector3D()
            + game->GetPlayer()->GetProperties()->GetDir()->toQVector3D() * 2.0f;
    p.setY(game->GetPlayer()->GetProperties()->GetPos()->toQVector3D().y() + 1.0f);

    QPointer <RoomObject> o = new RoomObject();
    o->SetType(TYPE_LIGHT);
    o->GetProperties()->SetJSID(game->GetGlobalUUID());
    o->GetProperties()->SetPos(p);
    o->GetProperties()->SetLightIntensity(10.0f);
    o->GetProperties()->SetLightRange(5.0f);

    const QString new_jsid = game->GetEnvironment()->GetCurRoom()->AddRoomObject(o);
//    game->SetSelected(game->GetEnvironment()->GetCurRoom(), new_jsid, true);
}

void HierarchyWindow::RowsInserted(const QModelIndex & parent, int start, int end)
{
//    qDebug() << "HierarchyWindow::RowsInserted" << start << end;
    if (cur_room) {
//        QHash <QString, QPointer <RoomObject> > & envobjects = cur_room->GetRoomObjects();
        QMap <int, QVariant> item_data = tree_widget.model()->itemData(parent);
        if (item_data.isEmpty()) {
            return;
        }
//
        QList<QTreeWidgetItem *> items = tree_widget.findItems(item_data.first().toString(), Qt::MatchExactly, 0);
        if (items.isEmpty()) {
            return;
        }

        QTreeWidgetItem * item = items.first();
        if (item == NULL) {
            return;
        }

        QPointer <RoomObject> p = cur_room->GetRoomObject(item->text(0));
        if (p) {
            for (int i=start; i<=end; ++i) {
                //use the js_id's of each to do grouping
                QTreeWidgetItem * child_item = item->child(i);
                if (child_item) {
                    QPointer <RoomObject> c = cur_room->GetRoomObject(child_item->text(0));
                    if (c) {
                        //we need to invert the parent's transform onto the child before adding
                        c->SetAttributeModelMatrix(p->GetAttributeModelMatrix().inverted() * c->GetAttributeModelMatrix());

                        p->GetProperties()->AppendChild(c->GetProperties());
                        p->AppendChild(c);
                    }
                }
            }
        }
    }
}

void HierarchyWindow::ItemSelectionChanged()
{
//    qDebug() << "HierarchyWindow::ItemSelectionChanged";
    if (cur_room) {        
        QHash <QString, QPointer <RoomObject> > envobjects = cur_room->GetRoomObjects();        
        for (QPointer <RoomObject> & o : envobjects) {
            if (o) {
                o->SetSelected(false);
            }
        }

        QList <QTreeWidgetItem *> items = tree_widget.selectedItems();
        for (int i=0; i<items.size(); ++i) {
            const QString js_id = items[i]->text(0);
//            qDebug() << "testing" << js_id << envobjects.contains(js_id) << envobjects[js_id];
            if (envobjects.contains(js_id) && envobjects[js_id]) {
                envobjects[js_id]->SetSelected(true);
                game->SetSelected(cur_room, js_id, true); //set's game's selected val, to trigger propertieswindow
            }
        }
    }
}

void HierarchyWindow::keyReleaseEvent(QKeyEvent * e)
{
    switch (e->key()) {
    case Qt::Key_Delete:
    {
        DeleteSelected();
    }
        break;
    }
}

void HierarchyWindow::DeleteSelected()
{
    QList <QTreeWidgetItem *> items = tree_widget.selectedItems();    
    for (int i=0; i<items.size(); ++i) {
        if (items[i]) {
            game->SetRoomDeleteCode(cur_room->GetSelectedCode(items[i]->text(0)));
            if (cur_room->DeleteSelected(items[i]->text(0))) {
                delete items[i];
            }
        }
    }
}
