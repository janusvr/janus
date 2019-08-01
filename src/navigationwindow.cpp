#include "navigationwindow.h"

NavigationWindow::NavigationWindow(Game * g) :
    game(g),
    itemcount(0),
    update(true)
{

    tree_widget.setSelectionMode(QAbstractItemView::SingleSelection);
    tree_widget.setDropIndicatorShown(true);
    tree_widget.setSortingEnabled(false);

    tree_widget.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QStringList s;
    s.push_back("url");

    tree_widget.setColumnCount(s.size());
    tree_widget.setHeaderLabels(s);
    tree_widget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&tree_widget, SIGNAL(itemSelectionChanged()), this, SLOT(ItemSelectionChanged()));
    connect(game->GetEnvironment(), SIGNAL(RoomsChanged()), this, SLOT(DoUpdate()));

//    QWidget * w = new QWidget(this);
    QGridLayout * v = new QGridLayout();
    v->addWidget(&tree_widget,0,0,1,1);
    v->setSpacing(0);
    v->setMargin(1);

    setLayout(v);
}

void NavigationWindow::DoUpdate()
{
//    qDebug() << "NavigationWindow::DoUpdate()";
    update = true;
}

void NavigationWindow::Update()
{    
    QPointer <Room> r = game->GetEnvironment()->GetCurRoom();
    if (r != cur_room) {
        cur_room = r;
//        update = true;
    }

    QPointer <Room> root = game->GetEnvironment()->GetRootRoom();

    if (update && root) {
        update = false;

        tree_widget.setUpdatesEnabled(false);

        tree_widget.clear();
        item_node_map.clear();

        itemcount = 0;
        Update_Helper(NULL, root);

        tree_widget.setUpdatesEnabled(true);
    }
}

void NavigationWindow::Update_Helper(QTreeWidgetItem * p, QPointer <Room> r)
{
    //do not add mirrors    
    QTreeWidgetItem * item;
    if (p == NULL) {
        item = new QTreeWidgetItem(&tree_widget);
    }
    else {
        item = new QTreeWidgetItem(p);
    }

    const QString u = r->GetProperties()->GetURL();
    item->setText(0, (r->GetStarted() ? QString("[X] ") : QString("[  ] ")) + u);
    item->setExpanded(true);
    if (game->GetEnvironment()->GetCurRoom() == r) {
        item->setSelected(true);
    }
    if (r->GetParentObject() && r->GetParentObject()->GetProperties()->GetMirror()) {
        item->setDisabled(true);
        item->setText(0, item->text(0) + QString(" (mirror)"));
    }
    ++itemcount;

    item_node_map[item] = r;

    //for each child, call update_helper
    QList <QPointer <Room> > c = r->GetChildren();
    for (int i=0; i<c.size(); ++i) {
        if (c[i]) {
            Update_Helper(item, c[i]);
        }
    }
}

void NavigationWindow::ItemSelectionChanged()
{
//    qDebug() << "NavigationWindow::ItemSelectionChanged";
    QList <QTreeWidgetItem *> items = tree_widget.selectedItems();
    if (!items.isEmpty() && item_node_map.contains(items.first())) {
        QPointer <Room> r = item_node_map[items.first()];
        if (r && r != game->GetEnvironment()->GetCurRoom()) {
            game->GetEnvironment()->NavigateToRoom(game->GetPlayer(), r);
        }
    }
}
