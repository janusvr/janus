#ifndef ASSETWINDOW_H
#define ASSETWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"
#include "webasset.h"

class AssetWindow : public QWidget
{
    Q_OBJECT

public:

    AssetWindow(Game *g);

    void Update();
    void keyReleaseEvent(QKeyEvent * e);

public slots:

    void AssetBrowserClick(QUrl u);
    void LoadAssetPalette();
    void AddAsset();
    void AddAssetGivenPath(QString path);
    void RemoveAsset();
    void GenerateAssetPaletteView();

private:

    QSlider asset_palette_slider;
    QPushButton load_asset_palette;
    QVariantMap asset_palette;
    QTextBrowser asset_browser;
    bool asset_browser_thumbs_processed;
    QList <QString> asset_browser_urls;
    QList <WebAsset *> asset_browser_thumbs;

    QPushButton add_asset_pushbutton;
    QPushButton remove_asset_pushbutton;
    QTableWidget table_widget;
    Game * game;
    QPointer <Room> cur_room;
    bool update;
};

#endif // ASSETWINDOW_H
