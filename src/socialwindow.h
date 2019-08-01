#ifndef SOCIALWINDOW_H
#define SOCIALWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"
#include "webasset.h"

class MyTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    MyTextBrowser()
    {

    }

    void setSource(const QUrl &name)
    {
//        qDebug() << "ss" << name;
        emit LinkClicked(name.toString());
    }

signals:

    void LinkClicked(QString);

};

class SocialWindow : public QMainWindow
{
    Q_OBJECT
public:

    SocialWindow(Game * g);

    void SetFocusOnChatEntry(const bool b);
    bool GetFocusOnChatEntry();

    void Shutdown();

public slots:

    void Update();
    void UpdatePartyModeList();
    void PartyModeListSelection();
    void SendChatMessage();
    void OpenLinkClicked(QString s);

private:

    QString ProcessChatMessage(const QString s);
    void UpdatePartyModeTable();

    QLabel * label_globalusers;
    QLabel * label_localusers;
    QLabel * label_userid;
    QTableWidget * table_partymode;
    QTableWidget * table_roomusers;
    MyTextBrowser * textbrowser_chatlog;
    QLineEdit * lineedit_chatentry;

    Game * game;
    QPointer <Room> cur_room;

    QTimer partymode_request_timer;
    WebAsset partymode_data_request;
    QList <QPointer <RoomObject> > players_list;
};

#endif // SOCIALWINDOW_H
