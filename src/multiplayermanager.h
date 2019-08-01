#ifndef MULTIPLAYERMANAGER_H
#define MULTIPLAYERMANAGER_H

#include <QtCore>
#include <QtNetwork>
#include <QtWebSockets/QtWebSockets>
#include <QUdpSocket>

#include "mathutil.h"
#include "htmlpage.h"
#include "assetghost.h"
#include "assetrecording.h"
#include "soundmanager.h"
#include "settingsmanager.h"

class RoomObject;
class Player;

struct RoomConnection
{
    RoomConnection():
        sent_subscribe(false)
    {
    }

    QString id;
    bool sent_subscribe; //this says "we have sent the subscribe packet"
};

class ServerConnection : public QObject {
    Q_OBJECT

public:

    ServerConnection(const QString & s, int p);

    void SendTextMessage(const QString & s);
    void Disconnect();

public slots:

    void connected();
    void disconnected();
    void socketError();
    void binaryMessageReceived(QByteArray b);
    void textMessageReceived(QString b);
    void readPendingDatagrams();

public:

    QWebSocket * tcpsocket;
    QList <QByteArray> incoming_packets;
    QString tcpserver;
    qint64 tcpport;    
    bool use_ssl;
    bool logging_in;
    bool logged_in;

    QMap <QString, RoomConnection> rooms; //indexed by "url" string

    QString last_error_msg;
    QTime reconnect_time;
    unsigned int retries;

    QString user;
    QString password;

    QString last_packet;
    QString sent_enterroom_url_id;

    QUdpSocket * udpsocket;
    int serverudpport;
};

class MultiPlayerManager : public QObject
{

    Q_OBJECT

public:

    MultiPlayerManager();
	~MultiPlayerManager();

    void Initialize();   

    void DoSocketConnect();
    void DoSocketDisconnect();

    void SetEnabled(const bool b);
    bool GetEnabled();

    QString GetCurURL() const;

    QList<QPointer<ServerConnection> > & GetConnectionList();
    QPointer <ServerConnection> GetConnection(const QString & server, const int port);
    QPointer <ServerConnection> AddConnection(const QString & server, const int port);
    void RemoveConnection(QPointer <ServerConnection> c);

    void AddSubscribeURL(QPointer <ServerConnection> c, const QString & url);
    void RemoveSubscribeURL(QPointer <Player> player, QPointer <ServerConnection> c, const QString & url);

    void Update(QPointer <Player> player, const QString & url, const QList <QString> adjacent_urls, const QString & name, const bool room_allows_party_mode, const float delta_time);

    void SetURLToDraw(const QString & s);
    void DrawGL(QPointer <AssetShader> shader, const QVector3D & player_pos, const bool render_left_eye);
    void DrawCursorsGL(QPointer <AssetShader> shader);

    void UpdateAvatarData();
    void LoadAvatarData(const bool load_userid);
    void SaveAvatarData();
    void SetAvatarData(const QString s);
    QString GetAvatarData() const;
    void SetAvatarFromGhost(QPointer <RoomObject> new_ghost);

    void DoUpdateAvatar();

    bool SetChatMessage(QPointer <Player> player, const QString & s);
    void SetSendPortal(const QString & s, const QString & js_id);    
    void SetCursorPosition(const QVector3D & cpos, const QVector3D & cxdir, const QVector3D & cydir, const QVector3D & czdir, const float cscale);    
    void AddMicBuffers(const QList <QByteArray> buffers);   

    QString GetUserID() const;    
    QPointer <RoomObject> GetPlayer();
    QList <QPointer <RoomObject> > GetPlayersInRoom(const QString & url);
    QMap <QString, DOMNode *> GetPlayersInRoomDOMNodeMap(const QString & url);

    QList <QPair <QString, QColor> > GetNewChatMessages();

    void AddChatMessage(const QString & s, const QColor col = QColor(0,0,0));    

    void SetRoomAssetEdit(const QString s);
    void SetRoomEdit(const QPointer <RoomObject> o);
    void SetRoomDeleteCode(const QString & s);

    void SetResetPlayer(const bool b);
    bool GetResetPlayer();

    void SetPartyMode(const bool b);
    bool GetPartyMode() const;

    void SetHeadSrc(const QString & s);
    void SetHeadMtl(const QString & s);
    void SetBodySrc(const QString & s);
    void SetBodyMtl(const QString & s);

    void SetRecording(const bool b, const bool record_everyone = true);
    bool GetRecording() const;

    int GetNumberUsersURL(const QString & url);

    void AddAssetRecording(QPointer <AssetRecording> a);

    void SetCustomPortalShader(const QString shader_src);
    QString GetCustomPortalShader() const;

    QList <QPointer <RoomObject> > & GetOnPlayerEnterEvents();
    QList <QPointer <RoomObject> > & GetOnPlayerExitEvents();

private:            

    void SetNewUserID(const QString id, const bool append_random_number);    

    QString GenerateUserID();
    QColor GenerateColour();   

    QString GetLogonPacket(const QString & userId, const QString & password, const QString & url_id);
    QString GetSubscribePacket(QPointer <Player> player, const QString & url_id);
    QString GetUnsubscribePacket(QPointer <Player> player, const QString & url_id);
    QString GetEnterRoomPacket(QPointer <Player> player, const bool room_allows_party_mode);

    QString GetChatPacket(QPointer <Player> player, const QString & chat_message);
    QString GetPortalPacket(QPointer <Player> player);
    QString GetMovePacket(QPointer <Player> player);
    QString GetMovePacket_Helper(QPointer <Player> player);

    QString GetUserChatPacket(QPointer <Player> player, const QString & chat_message);
    QString GetUserPortalPacket(QPointer <Player> player);
    QString GetUserMovePacket(QPointer <Player> player);

    void DoError(const QJsonValue & v, ServerConnection *s);
    void DoUserMoved(const QVariantMap & m);
    void DoUserChat(const QVariantMap & m);
    void DoUserPortal(const QVariantMap & m);
    void DoUserDisconnected(const QVariantMap & m);   

    bool enabled;    
    bool partymode;

    QMap <QString, QPointer <RoomObject> > players;
    QMap <QString, QPointer <RoomObject> > players_in_room;

    QString cur_url;
    QString cur_url_id;
    QString cur_name;

    QString avatar_data;
    QString avatar_data_encoded;
    QPointer <RoomObject> user_ghost;
    QString chat_message;
    QString send_portal;
    QString send_portal_jsid;

    QString room_edit_assets;
    QList <QPointer <RoomObject> > room_edit_objs;

    bool reset_player;

    QList <QPointer <ServerConnection> > connection_list;    

    QString userid_filename;
    QString userid_backup_filename;

    QString url_to_draw;
    QString url_to_draw_md5;

    QList <QPair <QString, QColor> > chat_messages_log;

    QList <QByteArray> mic_buffers;

    QVector3D cpos_list;
    QVector3D cxdir_list;
    QVector3D cydir_list;
    QVector3D czdir_list;
    float cscale_list;
    bool pending_pos;

    bool avatar_update;
    QTime avatar_update_timer;

    QTime update_time;
    DOMNode last_player_properties;

    bool recording;
    bool recording_everyone;
    QFile ghost_file;
    QTextStream ghost_ofs;
    QList <AssetRecordingPacket> ghost_packets;
    QTime ghost_recording_start_time;
    QList <QByteArray> ghost_recording_mic_buffers;        

    //assetrecordings (emulate server activity)
    QList <QPointer <AssetRecording> > assetrecording_list;

    QList <QPointer <RoomObject> > on_player_enter_events; //for room.onPlayerEnter room.onPlayerExit events
    QList <QPointer <RoomObject> > on_player_exit_events; //for room.onPlayerEnter room.onPlayerExit events

    static unsigned int max_connect_retries;
};

#endif // MULTIPLAYERMANAGER_H
