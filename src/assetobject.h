#ifndef ASSETOBJECT_H
#define ASSETOBJECT_H

#include <QtNetwork>
#include <QtGui>
#include <QtConcurrent/QtConcurrent>

#include "asset.h"
#include "assetshader.h"
#include "geom.h"
#include "webasset.h"
#include "assetimage.h"

class Geom;

class AssetObject : public Asset
{
    Q_OBJECT

public:

    AssetObject();
    virtual ~AssetObject();

    void SetProperties(const QVariantMap & d);

    void SetTextureFile(const QString & tex, const unsigned int index);
    void SetMTLFile(const QString & mtl);

    void Load();
    void Unload();

    void Save(const QString & filename);

    void Update();
    bool UpdateGL();

    void DrawGL(QPointer <AssetShader> shader, const QColor col = QColor(255,255,255));
    void DrawLoadingGL();

    bool GetFinished();
    bool GetTexturesFinished();

    QPointer <Geom> GetGeom();

    QVector3D GetBBoxMin();
    QVector3D GetBBoxMax();

    bool GetUsesTexFile();
    QString GetTexURL(const unsigned int index) const;

    void SetTextureClamp(const bool b);
    bool GetTextureClamp();

    void SetTextureLinear(const bool b);
    bool GetTextureLinear();

    void SetTextureCompress(const bool b);
    bool GetTextureCompress();

    void SetTextureMipmap(const bool b);
    bool GetTextureMipmap();

    int GetNumTris();

    float GetProgress();

    void SetTextureAlphaType(QString p_alpha_type);
    QString GetTextureAlphaType();

private:

    QPointer <Geom> geom;
    QPointer <AssetImage> img_error;
    QVector <QString> tex_url_str; 

    QTime time;
};

#endif // ASSETOBJECT_H
