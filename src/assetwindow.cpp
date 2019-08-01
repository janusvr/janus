#include "assetwindow.h"

AssetWindow::AssetWindow(Game* g) :
    game(g),
    update(true)
{
    asset_browser_thumbs_processed = true;

//    table_widget.setSelectionMode(QAbstractItemView::SingleSelection);
//    table_widget.setSelectionMode(QAbstractItemView::ExtendedSelection);
//    table_widget.setDragEnabled(true);
    table_widget.viewport()->setAcceptDrops(true);
    table_widget.setDropIndicatorShown(true);
//    table_widget.setDragDropMode(QAbstractItemView::InternalMove);
    table_widget.setSortingEnabled(true);
    table_widget.sortItems(0, Qt::AscendingOrder);
    table_widget.setSelectionBehavior(QAbstractItemView::SelectRows);

    table_widget.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QStringList s;
    s.push_back("id");
    s.push_back("type");
    s.push_back("src");

    table_widget.setColumnCount(s.size());
    table_widget.setHorizontalHeaderLabels(s);
    table_widget.verticalHeader()->setVisible(false);
    table_widget.horizontalHeader()->setStretchLastSection(true);
    table_widget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    load_asset_palette.setText("Load Asset Palette");

    add_asset_pushbutton.setText("Add Asset");
    add_asset_pushbutton.setMaximumHeight(30);

    remove_asset_pushbutton.setText("Remove Asset");
    remove_asset_pushbutton.setMaximumHeight(30);

    asset_palette_slider.setRange(64,256);
    asset_palette_slider.setValue(128);
    asset_palette_slider.setOrientation(Qt::Horizontal);

//    QWidget * w = new QWidget(this);
    QGridLayout * v = new QGridLayout();
    v->addWidget(&load_asset_palette,0,0);
    v->addWidget(&asset_palette_slider,0,1);
    v->addWidget(&asset_browser,1,0,1,2);
    v->addWidget(&add_asset_pushbutton,2,0);
    v->addWidget(&remove_asset_pushbutton,2,1);
    v->addWidget(&table_widget,3,0,1,2);
    v->setSpacing(0);
    v->setMargin(1);

    setLayout(v);

    asset_browser.setAcceptDrops(false);
    asset_browser.setReadOnly(true);
    asset_browser.setOpenLinks(false);
    asset_browser.setAcceptRichText(false);

    connect(&asset_browser, SIGNAL(anchorClicked(QUrl)), this, SLOT(AssetBrowserClick(QUrl)));
    connect(&load_asset_palette, SIGNAL(clicked(bool)), this, SLOT(LoadAssetPalette()));
    connect(&add_asset_pushbutton, SIGNAL(clicked(bool)), this, SLOT(AddAsset()));
    connect(&remove_asset_pushbutton, SIGNAL(clicked(bool)), this, SLOT(RemoveAsset()));
    connect(&asset_palette_slider, SIGNAL(sliderReleased()), this, SLOT(GenerateAssetPaletteView()));
}

void AssetWindow::Update()
{    
    QPointer <Room> r = game->GetEnvironment()->GetCurRoom();
    if (r != cur_room) {
        cur_room = r;
        update = true;
    }

    if (cur_room) {
        QList <QPointer <Asset> > assets = cur_room->GetAllAssets();

        //get number of assets, refresh if it does not match the number of rows in the table                
        if (assets.size() != table_widget.rowCount()) {
            update = true;
        }    

        if (update) {
            update = false;

            table_widget.blockSignals(true);
            table_widget.model()->blockSignals(true);
            table_widget.clearContents();
            table_widget.setSortingEnabled(false);

            table_widget.setRowCount(assets.size());

            int item_count = 0;
            for (QPointer <Asset> & a : assets) {
                if (a) {
                    table_widget.setItem(item_count, 0, new QTableWidgetItem(a->GetProperties()->GetID()));
                    table_widget.setItem(item_count, 1, new QTableWidgetItem(a->GetProperties()->GetTypeAsString()));
                    table_widget.setItem(item_count, 2, new QTableWidgetItem(a->GetProperties()->GetSrc()));
                }
                item_count++;
            }

            table_widget.blockSignals(false);
            table_widget.model()->blockSignals(false);
            table_widget.setSortingEnabled(true);
        }
    }

    //build browser table with base64 encoded images
    if (!asset_browser_thumbs_processed) {
        bool loaded_all = true;
        for (int i=0; i<asset_browser_thumbs.size(); ++i) {
            if (asset_browser_thumbs[i] == nullptr) {
                continue;
            }
            if (!asset_browser_thumbs[i]->GetLoaded() && !asset_browser_thumbs[i]->GetError()) {
                loaded_all = false;
//                qDebug() << "still waiting on" << asset_browser_thumbs[i]->GetURL();
            }
        }

        if (loaded_all) {
            asset_browser_thumbs_processed = true;            
            GenerateAssetPaletteView();
        }
    }
}

void AssetWindow::GenerateAssetPaletteView()
{
    if (asset_browser_thumbs_processed) {
        QVariantMap data = asset_palette["data"].toMap();
        QVariantList assets = data["assets"].toList();
        QString code = "<html><head></head><body>";
        for (int i=0; i<assets.size(); ++i) {
            QVariantMap m = assets[i].toMap();
            //code += "<table><tr><td>" + m["name"].toString() + "</td></tr><tr><td><img width=128 src=\"data:image/png;base64," + asset_browser_thumbs[i]->GetData().toBase64() + "\" /></td></tr></table>";
            code += "<a href=\"" + asset_browser_urls[i] + "\">";
            code += "<img width=" + QString::number(asset_palette_slider.value()) + " src=\"data:image/png;base64," + asset_browser_thumbs[i]->GetData().toBase64() + "\" />";
            code += "</a>";
        }
        asset_browser.setHtml(code);
    }
}

void AssetWindow::keyReleaseEvent(QKeyEvent * e)
{
    switch (e->key()) {
    case Qt::Key_Delete:
    {
        RemoveAsset();
    }
        break;
    }
}

void AssetWindow::AssetBrowserClick(QUrl u)
{
    //drag and drop a known asset
    game->DragAndDropAssetObject(u.toString(), 0);
}

void AssetWindow::LoadAssetPalette()
{
    const QString filename = QFileDialog::getOpenFileName(this, "Load Asset Palette", MathUtil::GetWorkspacePath(), "*.json");
    if (!filename.isNull()) {
        QFile f(filename);
        if (!f.open(QIODevice::ReadOnly)) {
            return;
        }

        QByteArray ba = f.readAll();
        f.close();

        asset_palette = QJsonDocument::fromJson(ba).toVariant().toMap();
        QVariantMap data = asset_palette["data"].toMap();
        QVariantList assets = data["assets"].toList();

        for (int i=0; i<asset_browser_thumbs.size(); ++i) {
            delete asset_browser_thumbs[i];
        }
        asset_browser_thumbs.clear();
        asset_browser_urls.clear();

        asset_browser_thumbs_processed = false;

        for (int i=0; i<assets.size(); ++i) {
            QVariantMap m = assets[i].toMap();
            QVariantMap c = m["contents"].toMap();

            const QString u = m["url"].toString();

            QPointer <AssetObject> ao = new AssetObject();

            //set the texture
            for (QVariantMap::const_iterator iter = c.begin(); iter != c.end(); ++iter) {
                if (iter.key().right(4) == ".png" && iter.key().right(13) != "thumbnail.png") {
//                    qDebug() << "SETTING TEXTURE FILE!" << "https://content.decentraland.today/contents/" + iter.value().toString();
                    ao->SetTextureFile("https://content.decentraland.today/contents/" + iter.value().toString(), 0);
                }
            }

            //set the ID to URL
            ao->GetProperties()->SetID(u);

            //set the geometry
            for (QVariantMap::const_iterator iter = c.begin(); iter != c.end(); ++iter) {
//                qDebug() << iter.key() << iter.value();
                if (iter.key() == u) {
                    ao->SetSrc("https://content.decentraland.today/contents/" + iter.value().toString(),
                               "https://content.decentraland.today/contents/" + iter.value().toString());
                }
            }

//            ao->Load();

            //add this asset data to the room (the .glb, and the other texture)
            if (cur_room) {
                cur_room->AddAssetObject(ao);
            }

            //get the thumbnail
            WebAsset * w = new WebAsset();
            w->Load(m["thumbnail"].toString());
            asset_browser_thumbs.push_back(w);
            asset_browser_urls.push_back(u);
        }

        asset_browser.setText("Loading asset palette...");

    }
}

void AssetWindow::AddAsset()
{
    const QString filename = QFileDialog::getOpenFileName(this, "Add Asset", MathUtil::GetWorkspacePath());
    if (!filename.isNull()) {
        AddAssetGivenPath(QUrl::fromLocalFile(filename).toString());
    }
}

void AssetWindow::AddAssetGivenPath(QString url)
{
    //59.0 - don't bother with dialog, just use the filename as assetid for now
    game->DragAndDrop(url.trimmed(), "Drag+Pin", 0);

    //force an update
    cur_room = NULL;
    Update();
}

void AssetWindow::RemoveAsset()
{
    QModelIndexList sel = table_widget.selectionModel()->selectedRows();

    if (cur_room && !sel.isEmpty()) {
        const int index = sel[0].row();
        QList <QPointer <Asset> > assets = cur_room->GetAllAssets();
        if (index >= 0 && index < assets.size()) {
            cur_room->RemoveAsset(assets[index]);
            cur_room = NULL;
            Update();
        }
    }
}
