#ifndef ENVROOMTEMPLATE_H
#define ENVROOMTEMPLATE_H

#include <QtGui>

#include "assetobject.h"

#include "mathutil.h"
#include "roomobject.h"

class RoomObject;

class RoomTemplate : public QObject
{

public:

    RoomTemplate();

    void Load(const QString & name);

    int GetNumMounts() const;
    void GetMount(const int index, QVector3D & pt, QVector3D & dir) const;    

    QPointer <RoomObject> GetEnvObject();   

    void Clear();

    void UpdateAssets();

private:

    void LoadData(const QString & filename);

    QPointer <AssetObject> asset_obj;
    QPointer <AssetObject> asset_collision_obj;
    QPointer <RoomObject> env_obj;

    QList <QVector3D> mount_pts;
    QList <QVector3D> mount_dirs;     
};

#endif // ENVROOMTEMPLATE_H
