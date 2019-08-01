#include "menu.h"

void MenuOperations::UpdatePageWithJanusObject(QPointer <AssetWebSurface> )
{

}

Q_INVOKABLE void MenuOperations::updatepopulardata(QString s)
{
//    qDebug() << "updatepopulardata()" << popular_data_request.GetStarted() << popular_data_request.GetProcessed() << s;
    if (!popular_data_request.GetStarted() || popular_data_request.GetProcessed()) {
        popular_data_request.Load(QUrl("http://api.janusvr.com/getPopularRooms"+s));
    }
}

Q_INVOKABLE QVariantList MenuOperations::getpopulardata()
{
//    qDebug() << "MenuOperations::getpopulardata()";
    if (popular_data_request.GetLoaded() && !popular_data_request.GetProcessed()) {
        const QByteArray & ba = popular_data_request.GetData();
        popular_data = QJsonDocument::fromJson(ba).toVariant().toMap()["data"].toList();
        popular_data_request.SetProcessed(true);
    }
    return popular_data;
}

Q_INVOKABLE void MenuOperations::updatepartymodedata()
{
//    qDebug() << "updatepartymodedata()" << partymode_data_request.GetStarted() << partymode_data_request.GetProcessed();
    if (!partymode_data_request.GetStarted() || partymode_data_request.GetProcessed()) {
        partymode_data_request.Load(QUrl("https://vesta.janusvr.com/api/party_mode"));
    }
}

Q_INVOKABLE QVariantList MenuOperations::getpartymodedata()
{
//        qDebug() << "getpartymodedata()" << partymode_data_request.GetLoaded() << partymode_data_request.GetProcessed();
    if (partymode_data_request.GetLoaded() && !partymode_data_request.GetProcessed()) {
        const QByteArray & ba = partymode_data_request.GetData();
//            QJsonParseError e;
//            QJsonDocument::fromJson(ba, &e);
//            qDebug() << e.errorString();
        partymode_data = QJsonDocument::fromJson(ba).toVariant().toMap()["data"].toList();
//            qDebug() << partymode_data;
//            qDebug() << "got here" << partymode_data;
        partymode_data_request.SetProcessed(true);
    }
    return partymode_data;
}
