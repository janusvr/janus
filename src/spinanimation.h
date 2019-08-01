#ifndef SPINANIMATION_H
#define SPINANIMATION_H

#include <QtGui>

#include "mathutil.h"
#include "assetobject.h"

class SpinAnimation : public QObject
{

public:

    SpinAnimation();
    ~SpinAnimation();

    static void initializeGL();
    static float GetIconScale();
    static void DrawGL(QPointer <AssetShader> shader, const float value, const bool billboard);
    static void DrawIconGL(QPointer <AssetShader> shader, const bool billboard, TextureHandle* texture0, const QColor color);
    static void DrawPlaneGL(QPointer <AssetShader> shader, const QColor col);

    static void UpdateAssets();

private:

    static QPointer <AssetObject> plane_obj;
    static QVector <QPointer <AssetImage> > loading_imgs;
    static float icon_scale;
};

#endif // SPINANIMATION_H
