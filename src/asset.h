#ifndef ASSET_H
#define ASSET_H

#include <QtCore>
#include <memory>

#include "domnode.h"
#include "webasset.h"

class Asset : public WebAsset
{
    Q_OBJECT

public:
    Asset();      

    void SetProperties(const QVariantMap & d);
    void SetSrc(const QString & base, const QString & src_str);
    QString GetXMLCode() const;
    QVariantMap GetJSONCode() const;
    QPointer <DOMNode> GetProperties();    

protected:

    QPointer <DOMNode> props;       

};

#endif // ASSET_H
