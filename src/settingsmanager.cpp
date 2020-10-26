#include "settingsmanager.h"

QVariantMap SettingsManager::settings;

SettingsManager::SettingsManager()
{

}

void SettingsManager::LoadSettings()
{
    settings["multiplayer"] = true;    
    settings["selfavatar"] = false;
    settings["maxmemory"] = 2048;
    settings["comfortmode"] = true;
    settings["downloadcache"] = true;
    settings["antialiasing"] = true;
    settings["enhanceddepthprecision"] = true;
    settings["assetshaders"] = true;
    settings["assetimages"] = true;
    settings["sounds"] = true;
    settings["volumevoip"] = 50.0f;
    settings["volumeenv"] = 50.0f;
    settings["volumemic"] = 50.0f;
    settings["positionalenv"] = true;
    settings["positionalvoip"] = true;
    settings["leapmotionhmd"] = true;
    settings["editmode"] = false;
    settings["editmodeicons"] = false;
    settings["crosshair"] = false;
    settings["gamepad"] = true;
    settings["portalhotkeys"] = false;
    settings["decouplehead"] = false;
    settings["mousepitch"] = true;
    settings["invertpitch"] = false;
    settings["avatarlighting"] = true;
    settings["launchurl"] = QString("");
    settings["server"] = QString("presence.janusxr.org");
    settings["port"] = 5567; //65.5 - prefer encrypted connection
    settings["rate"] = 200;
    settings["fov"] = 70.0f;
    settings["updatewebsurfaces"] = true;
    settings["updatevoip"] = true;
    settings["updatecmft"] = true;
    settings["updatecustomavatars"] = true;
    settings["rotationspeed"] = 50.0f;
    settings["ipd"] = 0.064f;
    settings["haptics"] = true;
    settings["partymode"] = false;
    settings["perflog"] = true;
    settings["playbackdevice"] = QString("Default device");
    settings["capturedevice"] = QString("Default device");
    settings["micalwayson"] = false;
    settings["micsensitivity"] = -20.0f;
    settings["ui"] = true;
    settings["FPSLimitRender"] = 0;
    settings["FPSLimitUpdate"] = 0;
    settings["vivetrackpadmovement"] = false;
    settings["homeurl"] = QString("https://vesta.janusxr.org/");
    settings["websurfaceurl"] = QString("https://google.com");
    settings["renderportalrooms"] = true;
    settings["pathtovoicefiles"] = QString("assets/sounds/voices/");
    settings["selectedvoice"] = QString("assets/sounds/voices/Female/en-US/female/");

    const QString filename = MathUtil::GetAppDataPath() + "settings.json";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "SettingsManager::LoadSettings(): File " << filename << " not found";
    }
    else {
        const QByteArray ba = file.readAll();
        file.close();

        //49.78 - unite the two maps (defaults and loaded) instead of direct assignment, fixes null launchurls
        QVariantMap loaded_map = QJsonDocument::fromJson(ba).toVariant().toMap();
        for (QVariantMap::iterator i = loaded_map.begin(); i != loaded_map.end(); ++i) {
            settings[i.key()] = i.value();
        }
    }

    //54.5 - force sounds on, even if toggled off last usage
    settings["sounds"] = true;

    //qDebug() << ba;
    qDebug() << "SettingsManager::LoadSettings() - Loaded" << settings.size() << "settings.";
}

void SettingsManager::SaveSettings() {
    const QString filename = MathUtil::GetAppDataPath() + "settings.json";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
	qDebug() << "SettingsManager::SaveSettings() - unable to save " << filename;
        return;
    }

    QTextStream ofs(&file);
    ofs << QJsonDocument::fromVariant(settings).toJson();
    file.close();
    qDebug() << "SettingsManager::SaveSettings() - " << filename << "saved";
}

bool SettingsManager::GetMultiplayerEnabled()
{
    return settings["multiplayer"].toString().toLower() == "true";
}

bool SettingsManager::GetPartyModeEnabled()
{
    return settings["partymode"].toString().toLower() == "true";
}

bool SettingsManager::GetSelfAvatar()
{
    return settings["selfavatar"].toString().toLower() == "true";
}

qint64 SettingsManager::GetDeallocateAmount()
{
    return qint64(settings["maxmemory"].toInt()) * qint64(1000000);
}

bool SettingsManager::GetComfortMode()
{
    return settings["comfortmode"].toString().toLower() == "true";
}

bool SettingsManager::GetHapticsEnabled()
{
    return settings["haptics"].toString().toLower() == "true";
}

bool SettingsManager::GetCacheEnabled()
{
    return settings["downloadcache"].toString().toLower() == "true";
}

bool SettingsManager::GetAntialiasingEnabled()
{
    return settings["antialiasing"].toString().toLower() == "true";
}

bool SettingsManager::GetEnhancedDepthPrecisionEnabled()
{
    return settings["enhanceddepthprecision"].toString().toLower() == "true";
}

bool SettingsManager::GetDecoupleHeadEnabled()
{
    return settings["decouplehead"].toString().toLower() == "true";
}

void SettingsManager::SetSoundsEnabled(const bool b)
{
    settings["sounds"] = (b ? "true" : "false");
}

bool SettingsManager::GetSoundsEnabled()
{
    return settings["sounds"].toString().toLower() == "true";
}

float SettingsManager::GetVolumeEnv()
{
    if (settings["volumeenv"].toFloat() > 100) return 100.0f;
    else if (settings["volumeenv"].toFloat() < 0) return 0.0f;
    return settings["volumeenv"].toFloat();
}

float SettingsManager::GetVolumeVOIP()
{
    if (settings["volumevoip"].toFloat() > 100) return 100.0f;
    else if (settings["volumevoip"].toFloat() < 0) return 0.0f;
    return settings["volumevoip"].toFloat();
}

float SettingsManager::GetMicSensitivity()
{
    if (settings["micsensitivity"].toFloat() > 0) return 0.0f;
    else if (settings["micsensitivity"].toFloat() < -100) return -100.0f;
    return settings["micsensitivity"].toFloat();
}

float SettingsManager::GetVolumeMic()
{
    if (settings["volumemic"].toFloat() > 100) return 100.0f;
    else if (settings["volumemic"].toFloat() < 0) return 0.0f;
    return settings["volumemic"].toFloat();
}

bool SettingsManager::GetPositionalEnvEnabled()
{
    return settings["positionalenv"].toString().toLower() == "true";
}

bool SettingsManager::GetPositionalVOIPEnabled()
{
    return settings["positionalvoip"].toString().toLower() == "true";
}

void SettingsManager::SetMicAlwaysOn(const bool b)
{
    settings["micalwayson"] = (b ? "true" : "false");
}

bool SettingsManager::GetMicAlwaysOn()
{
    return settings["micalwayson"].toString().toLower() == "true";
}

bool SettingsManager::GetShadersEnabled()
{
    return settings["assetshaders"].toString().toLower() == "true";
}

bool SettingsManager::GetAssetImagesEnabled()
{
    return settings["assetimages"].toString().toLower() == "true";
}

void SettingsManager::SetMousePitchEnabled(const bool b)
{
    settings["mousepitch"] = b;
}

bool SettingsManager::GetMousePitchEnabled()
{
    return settings["mousepitch"].toString().toLower() == "true";
}

bool SettingsManager::GetInvertYEnabled()
{
    return settings["invertpitch"].toString().toLower() == "true";
}

bool SettingsManager::GetLeapOnHMDEnabled()
{
    return settings["leapmotionhmd"].toString().toLower() == "true";
}

bool SettingsManager::GetEditModeEnabled()
{
    return settings["editmode"].toString().toLower() == "true";
}

bool SettingsManager::GetEditModeIconsEnabled()
{
    return settings["editmodeicons"].toString().toLower() == "true";
}

bool SettingsManager::GetCrosshairEnabled()
{
    return settings["crosshair"].toString().toLower() == "true";
}

bool SettingsManager::GetGamepadEnabled()
{
    return settings["gamepad"].toString().toLower() == "true";
}

bool SettingsManager::GetPortalHotkeys()
{
    return settings["portalhotkeys"].toString().toLower() == "true";
}

void SettingsManager::SetLaunchURL(const QString & s)
{
    settings["launchurl"] = s;
}

QString SettingsManager::GetLaunchURL()
{
    return settings["launchurl"].toString();
}

void SettingsManager::SetServer(const QString s)
{
    settings["server"] = s;
}

QString SettingsManager::GetServer()
{
    return settings["server"].toString();
}

void SettingsManager::SetPort(const int i)
{
    settings["port"] = i;
}

int SettingsManager::GetPort()
{
    return settings["port"].toInt();
}

int SettingsManager::GetRate()
{
    return settings["rate"].toInt();
}

float SettingsManager::GetIPD()
{
    return settings["ipd"].toFloat();
}

bool SettingsManager::GetHaptics()
{
    return settings["haptics"].toBool();
}

bool SettingsManager::GetPerfLog()
{
    return settings["perflog"].toBool();
}

QString SettingsManager::GetPlaybackDevice()
{
    return settings["playbackdevice"].toString();
}

QString SettingsManager::GetCaptureDevice()
{
    return settings["capturedevice"].toString();
}

float SettingsManager::GetRotationSpeed()
{
    return settings["rotationspeed"].toFloat();
}

float SettingsManager::GetFOV()
{
    return settings["fov"].toFloat();
}

bool SettingsManager::GetUpdateWebsurfaces()
{
    return settings["updatewebsurfaces"].toBool();
}

bool SettingsManager::GetUpdateVOIP()
{
    return settings["updatevoip"].toBool();
}

bool SettingsManager::GetUpdateCMFT()
{
    return settings["updatecmft"].toBool();
}

bool SettingsManager::GetUpdateCustomAvatars()
{
    return settings["updatecustomavatars"].toBool();
}

bool SettingsManager::GetViveTrackpadMovement()
{
    return settings["vivetrackpadmovement"].toBool();
}

QString SettingsManager::GetHomeURL()
{
    return settings["homeurl"].toString();
}

QString SettingsManager::GetWebsurfaceURL()
{
    return settings["websurfaceurl"].toString();
}

bool SettingsManager::GetRenderPortalRooms()
{
    return settings["renderportalrooms"].toBool();
}

QString SettingsManager::GetVoicePath()
{
    return settings["pathtovoicefiles"].toString();
}

QString SettingsManager::GetSelectedVoice()
{
    return settings["selectedvoice"].toString();
}

void SettingsManager::SetSelectedVoice( const QString new_voice )
{
    settings["selectedvoice"] = new_voice;
}
