#ifndef MENU_H
#define MENU_H
#include <memory>

#include "environment.h"
#include "assetwebsurface.h"
#include "settingsmanager.h"

class MenuOperations : public QObject {
    Q_OBJECT

public:

    Q_PROPERTY(QString version MEMBER version)
    Q_PROPERTY(QString versiononline MEMBER versiononline)
    Q_PROPERTY(QString currentkey MEMBER currentkey)
    Q_PROPERTY(QVariantList chat MEMBER chat)
    Q_PROPERTY(int networkstatus MEMBER networkstatus)    
    Q_PROPERTY(QString networkerror MEMBER networkerror)
    Q_PROPERTY(QString roomserver MEMBER roomserver)
    Q_PROPERTY(int playercount MEMBER playercount)
    Q_PROPERTY(QVariantList bookmarks MEMBER bookmarks)
    Q_PROPERTY(QVariantList workspaces MEMBER workspaces)
    Q_PROPERTY(QVariantList playbackdevices MEMBER playbackdevices)
    Q_PROPERTY(QVariantList capturedevices MEMBER capturedevices)
    Q_PROPERTY(QVariantList playerlist MEMBER playerlist)
    Q_PROPERTY(QString userid READ getuserid WRITE setuserid)
    Q_PROPERTY(bool avatarlighting READ getavatarlighting WRITE setavatarlighting)    
    Q_PROPERTY(QVariantList partymodedata READ getpartymodedata)
    Q_PROPERTY(QVariantList populardata READ getpopulardata)
    Q_PROPERTY(bool hmd MEMBER hmd)

    Q_INVOKABLE void updatepopulardata(QString s="");
    Q_INVOKABLE QVariantList getpopulardata();
    Q_INVOKABLE void updatepartymodedata();
    Q_INVOKABLE QVariantList getpartymodedata();

    Q_INVOKABLE QString currenturl() {
        if (player_curroom) {
            return player_curroom->GetProperties()->GetURL();
        }
        return "";
    }

    Q_INVOKABLE int tricount() {
        if (player_curroom) {
            return player_curroom->GetRoomNumTris();
        }
        return 0;
    }

    Q_INVOKABLE int locked() {
        if (player_curroom) {
            return (player_curroom->GetProperties()->GetLocked() ? 1 : 0);
        }
        return false;
    }

    Q_INVOKABLE bool roompartymode() {
        if (player_curroom) {
            return (player_curroom->GetProperties()->GetPartyMode());
        }
        return false;
    }

    Q_INVOKABLE QVariant getsetting(QString key) {
        return SettingsManager::settings[key];
    }

    Q_INVOKABLE void setsetting(QString key, QVariant value) {
        SettingsManager::settings[key] = value;
        SettingsManager::SaveSettings();
    }

    Q_INVOKABLE float roomprogress() {
        if (player_curroom) {
            return player_curroom->GetProgress();
        }
        return 0.0f;
    }

    Q_INVOKABLE void launchurl(QString url, int usePortal = 1) {
        do_launchurl = true;
        do_launchurl_url = url;
        do_launchurl_useportal = usePortal;
    }

    Q_INVOKABLE void navback() {
        do_navback = true;
    }

    Q_INVOKABLE void navforward() {
        do_navforward = true;
    }

    Q_INVOKABLE void navhome() {
        do_navhome = true;
    }

    Q_INVOKABLE void chatsend(QString msg) {
        do_chatsend = true;
        do_chatsend_msg = msg;
    }

    Q_INVOKABLE void sync() {
        do_sync = true;
    }

    Q_INVOKABLE void quit() {
        do_quit = true;
    }

    Q_INVOKABLE void focus() {
        do_focus = true;
    }

    Q_INVOKABLE void unfocus() {
        do_unfocus = true;
    }

    Q_INVOKABLE QString saveroom() {
        if (player_curroom) {
            do_saveroom = true;
            QDir dir(MathUtil::GetWorkspacePath());
            return dir.relativeFilePath(player_curroom->GetSaveFilename());
        }
        return "";
    }

    Q_INVOKABLE QString roomcode() {
        if (player_curroom.isNull()) {
            return "";
        }
        else {
            QString room_code;
            QTextStream ofs;
            ofs.setString(&room_code);
            player_curroom->SaveXML(ofs);
            return room_code;
        }
    }

    Q_INVOKABLE void setroomcode(QString s) {
        if (player_curroom.isNull() && env != nullptr) {
            env->UpdateRoomCode(s);
        }
    }

    Q_INVOKABLE void setuserid(QString s) {

        userid = s;
        if (player) {
            player->GetProperties()->SetUserID(s);
        }
        if (multi_players != nullptr && env != nullptr) {
            QPointer <RoomObject> user_ghost = multi_players->GetPlayer();
            if (user_ghost) {
                user_ghost->GetProperties()->SetID(s);

                multi_players->DoUpdateAvatar();

                if (multi_players->GetEnabled()) {
                    multi_players->SetEnabled(false);
                    multi_players->SetEnabled(true);
                }
            }
        }
    }

    Q_INVOKABLE QString getuserid() {
        if (player) {
            userid = player->GetProperties()->GetUserID();
        }
        return userid;
    }

    Q_INVOKABLE void setavatarlighting(bool b) {
        if (multi_players != nullptr && env != nullptr) {
            QPointer <RoomObject> user_ghost = multi_players->GetPlayer();
            if (user_ghost) {
                user_ghost->GetProperties()->SetLighting(b);
                multi_players->DoUpdateAvatar();
            }
        }
    }

    Q_INVOKABLE bool getavatarlighting() {
        if (multi_players) {
            QPointer <RoomObject> user_ghost = multi_players->GetPlayer();
            if (user_ghost) {
                return user_ghost->GetProperties()->GetLighting();
            }
        }
        return false;
    }

    Q_INVOKABLE void setavatar(QString s) {
        if (multi_players) {
            multi_players->SetAvatarData(s);
            multi_players->SaveAvatarData();
            multi_players->LoadAvatarData(true);

            if (multi_players->GetEnabled()) {
                multi_players->SetEnabled(false);
                multi_players->SetEnabled(true);
            }
        }
    }

    Q_INVOKABLE QString getavatar() {
        if (multi_players) {
            return multi_players->GetAvatarData();
        }
        else {
            return "";
        }
    }

    Q_INVOKABLE void resetavatar() {
        if (multi_players) {
            multi_players->LoadAvatarData(false);

            if (multi_players->GetEnabled()) {
                multi_players->SetEnabled(false);
                multi_players->SetEnabled(true);
            }
        }
    }

    Q_INVOKABLE void createasset(QString asset_type, QVariantMap property_list, const bool do_sync=true) {
//        qDebug() << "createasset" << asset_type << property_list;
        if (player_curroom && (!player_curroom->GetProperties()->GetLocked() || !do_sync)) {
            player_curroom->AddAsset(asset_type, property_list, do_sync);
        }
    }

    Q_INVOKABLE void createobject(QString object_type, QVariantMap property_list, const bool do_sync=true) {
//        qDebug() << "createobject" << object_type << property_list;
        if (player_curroom && (!player_curroom->GetProperties()->GetLocked() || !do_sync)) {

            //different way of adding to room if it's a link, not a RoomObject
            if (object_type.toLower() == "link") {
                QPointer <RoomObject> new_portal = new RoomObject();
                new_portal->SetType(TYPE_LINK);
                new_portal->SetProperties(property_list);
                new_portal->SetURL(player_curroom->GetProperties()->GetURL(), property_list["url"].toString());
                new_portal->SetTitle(property_list["title"].toString());

                player_curroom->AddRoomObject(new_portal);
            }
            else {                                
                QPointer <RoomObject> o = new RoomObject();
                o->SetType(DOMNode::StringToElementType(object_type.toLower()));
                o->SetProperties(property_list);
                o->GetProperties()->SetSync(false);
                o->PlayCreateObject();
                player_curroom->AddRoomObject(o);
            }
        }
    }   

    Q_INVOKABLE void removeobject(QString js_id, const bool do_sync=true) {
//        qDebug() << "removeobject" << js_id << do_sync;
        if (player_curroom && (!player_curroom->GetProperties()->GetLocked() || !do_sync)) {
            QPointer <RoomObject> o = player_curroom->GetRoomObject(js_id);
            if (o) {
                const QString delete_obj_code = o->GetXMLCode();
                if (player_curroom->DeleteSelected(js_id) && do_sync) {
                    multi_players->SetRoomDeleteCode(delete_obj_code);
                }
            }
        }
    }

    Q_INVOKABLE void saveworkspace(const QString workspace_dir) {
        do_saveworkspace = true;
        do_saveworkspacename = workspace_dir;
    }

    void UpdatePageWithJanusObject(QPointer <AssetWebSurface> webpage);

    MenuOperations():        
        version(__JANUS_VERSION),
        versiononline(__JANUS_VERSION),
        hmd(false),
        do_navback(false),
        do_navforward(false),
        do_navhome(false),
        do_launchurl(false),
        do_chatsend(false),
        do_sync(false),        
        do_quit(false),
        do_saveroom(false),
        do_focus(false),
        do_unfocus(false),
        do_saveworkspace(false),
        near_dist(1.0f)
    {
    timer.start();
    }

    QString version;
    QString versiononline;
    QString currentkey;
    QVariantList chat;
    int networkstatus;
    QString networkerror;
    QString roomserver;
    int playercount;
    QVariantList bookmarks;
    QVariantList workspaces;
    QVariantList playbackdevices;
    QVariantList capturedevices;
    QVariantList playerlist;
//    QVariantMap settings;
    QString userid;    
    bool hmd;

    bool do_navback;
    bool do_navforward;
    bool do_navhome;
    bool do_launchurl;
    QString do_launchurl_url;
    int do_launchurl_useportal;
    bool do_chatsend;
    QString do_chatsend_msg;
    bool do_sync;    
    bool do_quit;
    bool do_saveroom;    
    bool do_focus;
    bool do_unfocus;
    bool do_saveworkspace;
    QString do_saveworkspacename;

    QPointer <Room> player_curroom;
    QString selected;
    Environment * env;
    QPointer <MultiPlayerManager> multi_players;
    QPointer <Player> player;

    WebAsset popular_data_request;
    QVariantList popular_data;

    WebAsset partymode_data_request;
    QVariantList partymode_data;

    float near_dist;
    QTime timer;
};

#endif // MENU_H
