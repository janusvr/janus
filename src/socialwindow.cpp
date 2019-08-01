#include "socialwindow.h"

SocialWindow::SocialWindow(Game * g) :
    game(g),
    cur_room(NULL)
{
    QWidget * w = new QWidget(this);
    QVBoxLayout * layout = new QVBoxLayout();

    label_userid = new QLabel();
    label_globalusers = new QLabel("GLOBAL USERS (PARTY MODE)");
    label_globalusers->setStyleSheet("color:#62BD6C;");
    label_localusers = new QLabel("LOCAL USERS (CURRENT URL)");
    label_localusers->setStyleSheet("color:#62BD6C;");

    QStringList s;
    s.push_back("userid");
    s.push_back("url");

    table_partymode = new QTableWidget();
    table_partymode->setColumnCount(s.size());
    table_partymode->setHorizontalHeaderLabels(s);
    table_partymode->verticalHeader()->setVisible(false);
    table_partymode->horizontalHeader()->setStretchLastSection(true);
    table_partymode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table_partymode->setSortingEnabled(true);
    table_partymode->sortItems(0, Qt::AscendingOrder);
    table_partymode->setSelectionBehavior(QAbstractItemView::SelectItems);
    connect(table_partymode, SIGNAL(clicked(QModelIndex)), this, SLOT(PartyModeListSelection()));

    QStringList s2;
    s2.push_back("userid");

    table_roomusers = new QTableWidget();
    table_roomusers->setColumnCount(s2.size());
    table_roomusers->setHorizontalHeaderLabels(s2);
    table_roomusers->verticalHeader()->setVisible(false);
    table_roomusers->horizontalHeader()->setStretchLastSection(true);
    table_roomusers->setSortingEnabled(true);
    table_roomusers->sortItems(0, Qt::AscendingOrder);

    textbrowser_chatlog = new MyTextBrowser();
    textbrowser_chatlog->setReadOnly(true);
    connect(textbrowser_chatlog, SIGNAL(LinkClicked(QString)), this, SLOT(OpenLinkClicked(QString)));

    lineedit_chatentry = new QLineEdit();
    connect(lineedit_chatentry, SIGNAL(returnPressed()), this, SLOT(SendChatMessage()));

    QWidget * w2 = new QWidget();
    QHBoxLayout * l2 = new QHBoxLayout();
    l2->addWidget(label_userid);
    l2->addWidget(lineedit_chatentry);
    w2->setLayout(l2);

    layout->addWidget(label_globalusers);
    layout->addWidget(table_partymode);
    layout->addWidget(label_localusers);
    layout->addWidget(table_roomusers);
    QLabel * local_chat = new QLabel("LOCAL CHAT");
    local_chat->setStyleSheet("color:#62BD6C;");
    layout->addWidget(local_chat);
    layout->addWidget(textbrowser_chatlog);
    layout->addWidget(w2);

    w->setLayout(layout);
    setCentralWidget(w);

    connect(&partymode_request_timer, SIGNAL(timeout()), this, SLOT(UpdatePartyModeList()));
    partymode_request_timer.start(5000);
}

void SocialWindow::Update()
{
    //update userid
    if (label_userid && game->GetPlayer()->GetProperties()->GetUserID() != label_userid->text()) {
        label_userid->setText(game->GetPlayer()->GetProperties()->GetUserID());
    }

    //update current URL playerlist
    QList <QPointer <RoomObject> > players = game->GetMultiPlayerManager()->GetPlayersInRoom(game->GetPlayer()->GetProperties()->GetURL());
    if (players_list != players) {
        players_list = players;

//        qDebug() << "SocialWindow::Update()" << players_list;
        //repopulate the party mode stuff
        table_roomusers->setSortingEnabled(false);
        table_roomusers->clearContents();
        table_roomusers->setRowCount(players_list.size());
        table_roomusers->setEditTriggers(QAbstractItemView::NoEditTriggers);
        for (int i=0; i<players_list.size(); ++i) {
            if (players_list[i]) {
                table_roomusers->setItem(i, 0, new QTableWidgetItem(players_list[i]->GetProperties()->GetID()));
            }
        }
        table_roomusers->setSortingEnabled(true);

        label_localusers->setText("LOCAL USERS (CURRENT URL) "+QString::number(players_list.size()));
    }

    //update party mode
    if (partymode_data_request.GetLoaded() && !partymode_data_request.GetProcessed()) {
        const QByteArray & ba = partymode_data_request.GetData();
        MathUtil::GetPartyModeData() = QJsonDocument::fromJson(ba).toVariant().toMap()["data"].toList();
        partymode_data_request.SetProcessed(true);

        //repopulate the party mode stuff
        UpdatePartyModeTable();
    }

    //update text chat messages
    QList <QPair <QString, QColor> > msgs = game->GetMultiPlayerManager()->GetNewChatMessages();
    for (int i = 0; i < msgs.size(); ++i) {
        textbrowser_chatlog->append(ProcessChatMessage(msgs[i].first));
    }
    if (!msgs.isEmpty()) {
        QScrollBar *sb = textbrowser_chatlog->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void SocialWindow::Shutdown()
{
    disconnect(&partymode_request_timer, 0, 0, 0);
}

void SocialWindow::UpdatePartyModeList()
{
    //if not visible and we're not following
    if (!isVisible() && game->GetPlayer() && !game->GetPlayer()->GetFollowMode()) {
        return;
    }

    if (!partymode_data_request.GetStarted() || partymode_data_request.GetProcessed()) {
        partymode_data_request.Load(QUrl("https://vesta.janusvr.com/api/party_mode"));
    }
}

void SocialWindow::PartyModeListSelection()
{
    QModelIndexList sel = table_partymode->selectionModel()->selectedIndexes();

    if (!sel.isEmpty() && table_partymode->item(sel.first().row(), sel.first().column())) {

        const QString id = table_partymode->item(sel.first().row(), 0)->text();
        const QString url = table_partymode->item(sel.first().row(), 1)->text();

        if (sel.first().column() == 0) { //clicked player (follow directly)
            QPointer <Player> player = game->GetPlayer();

            if (player->GetFollowMode() && player->GetFollowModeUserID() == id) {
                player->SetFollowModeUserID("");
                player->SetFollowMode(false);
            }
            else {
                //if unfollowing, follow
                if (!url.isEmpty()) {
                    player->SetFollowMode(true);
                    player->SetFollowModeUserID(id);
                }
            }
//            qDebug() << "  " << player->GetS("follow_mode_userid");
            UpdatePartyModeTable();
        }
        else {
            //clicked URL - create a portal to it
            game->CreatePortal(url, true);
        }
    }
}

void SocialWindow::SendChatMessage()
{
    if (!lineedit_chatentry->text().isEmpty()) {
        game->SendChatMessage(lineedit_chatentry->text());
        lineedit_chatentry->clear();
    }
}

bool SocialWindow::GetFocusOnChatEntry()
{
    if (isVisible()) {
        return lineedit_chatentry->hasFocus();
    }
    return false;
}

void SocialWindow::SetFocusOnChatEntry(const bool b)
{
    if (b) {
        if (!isVisible()) {
            setVisible(true);
        }
        lineedit_chatentry->setFocus();
    }
    else {
        lineedit_chatentry->clearFocus();
    }
}

void SocialWindow::OpenLinkClicked(QString s)
{
//    qDebug() << "SocialWindow::LinkClicked" << s;
    game->CreatePortal(s, true);
}

QString SocialWindow::ProcessChatMessage(const QString s)
{
    QStringList l = s.split(" ");
    if (l.isEmpty()) {
        return QString("");
    }

    if (l.size() == 3 && l[1] == "is" && l[2] == "nearby.") {
        return "<span style='color:#62BD6C;'>" + l[0] + " " +l[1] + " " + l[2] + "</span>";
    }

    QString s2 = QString("<span style='color:#62BD6C;'>") + l.first() + ": </span>";

    for (int i=1; i<l.size(); ++i) {
        if (l[i].contains("http://") || l[i].contains("https://")) {
            l[i] = QString("<a href=\"") + l[i] + QString("\">") + l[i] + QString("</a>");
        }
        s2 += " " + l[i];
    }
    return s2;
}

void SocialWindow::UpdatePartyModeTable()
{
    QVariantList & partymode_data = MathUtil::GetPartyModeData();
    const bool follow_mode = game->GetPlayer()->GetFollowMode();
    const QString follow_mode_userid = game->GetPlayer()->GetFollowModeUserID();

    table_partymode->setSortingEnabled(false);
    table_partymode->clearContents();
    table_partymode->setRowCount(partymode_data.size());
    for (int i=0; i<partymode_data.size(); ++i) {
        //party mode attribs: userId, url (optional), roomId, name
        //qDebug() << partymode_data[i].toMap()["userId"];
        QMap<QString, QVariant> m = partymode_data[i].toMap();
        const QString userid = m["userId"].toString();
        const QString url = m.contains("url") ? m["url"].toString() : QString();
        table_partymode->setItem(i, 0, new QTableWidgetItem(userid));
        table_partymode->setItem(i, 1, new QTableWidgetItem(url));
    }
    table_partymode->setSortingEnabled(true);

    label_globalusers->setText("GLOBAL USERS (PARTY MODE) "+QString::number(partymode_data.size()));
}
