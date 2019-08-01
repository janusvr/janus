#include "roomtemplate.h"

RoomTemplate::RoomTemplate()
{
    mount_pts.push_back(QVector3D(0, 0, 0));
    mount_dirs.push_back(QVector3D(0, 0, -1));
}

void RoomTemplate::Load(const QString & name)
{
    QString prefix = QString("assets/rooms/");
    QString obj_name = prefix + name + QString(".obj");
    QString col_name = prefix + name + QString("_collision.obj");
    QString tex_name = prefix + name + QString(".png");
    QString dat_name = prefix + name + QString(".txt");

    const QString tile_d = prefix + "tile_d.png";
    const QString tile_g = prefix + "tile_g.png";
    const QString tile_n = prefix + "tile_n.png";
    const QString tile_s = prefix + "tile_s.png";

    mount_pts.clear();
    mount_dirs.clear();

    asset_obj = new AssetObject();
    asset_obj->SetSrc(MathUtil::GetApplicationURL(), obj_name);
    if (obj_name.contains("plane")) {
        asset_obj->SetTextureFile(tile_d, 0);
        asset_obj->SetTextureFile(tile_g, 1);
        asset_obj->SetTextureFile(tile_s, 2);
        asset_obj->SetTextureFile(tile_n, 3);
    }
    else {
        asset_obj->SetTextureFile(tex_name, 0);
    }
    asset_obj->Load();

    asset_collision_obj = new AssetObject();
    asset_collision_obj->SetSrc(MathUtil::GetApplicationURL(), col_name);
    asset_collision_obj->Load();

    env_obj = new RoomObject();
    env_obj->SetType(TYPE_OBJECT);
    env_obj->SetAssetObject(asset_obj);
    env_obj->SetCollisionAssetObject(asset_collision_obj);
    env_obj->GetProperties()->SetZDir(QVector3D(0,0,1));
    env_obj->GetProperties()->SetJSID("__" + name);

    LoadData(MathUtil::GetApplicationPath() + dat_name);
}

void RoomTemplate::LoadData(const QString & filename)
{
    //qDebug() << "RoomTemplate::LoadData" << filename;
    QFile file( filename );
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qDebug() << "RoomTemplate::LoadData - Warning, could not open data file" << filename;
        return;
    }

    //set base path (for loading resources in same dir)
    QTextStream ifs(&file);

    //get num colliders
    QStringList eachline = ifs.readLine().split(" ");
    const int nColliders = eachline.last().toInt();

    //read rectangular colliders
    for (int i=0; i<nColliders; ++i) {
        eachline = ifs.readLine().split(" ");
    }

    //read mount points
    eachline = ifs.readLine().split(" ");
    const int nMounts = eachline.last().toInt();
//    qDebug() << "nMounts" << nMounts;

    for (int i=0; i<nMounts; ++i) {
        eachline = ifs.readLine().split(" ");

        const float px = eachline[0].toFloat();
        const float py = eachline[1].toFloat();
        const float pz = eachline[2].toFloat();

        const float dx = eachline[3].toFloat();
        const float dy = eachline[4].toFloat();
        const float dz = eachline[5].toFloat();

        mount_pts.push_back(QVector3D(px, py, pz));
        mount_dirs.push_back(QVector3D(dx, dy, dz));
    }

    file.close();
}

QPointer <RoomObject> RoomTemplate::GetEnvObject()
{
    return env_obj;
}

void RoomTemplate::GetMount(const int index, QVector3D & pt, QVector3D & dir) const
{

    if (index < 0 || index >= mount_pts.size()) {
        qDebug() << "EnvRoomTemplate::GetMount - Warning, index was " << index << "but only " << mount_pts.size() << "mountpoints.";
        return;
    }   

    pt = mount_pts[index];
    dir = mount_dirs[index];
}

int RoomTemplate::GetNumMounts() const
{
    return mount_pts.size();
}

void RoomTemplate::Clear()
{    
    mount_pts.clear();
    mount_dirs.clear();
}

void RoomTemplate::UpdateAssets()
{
    if (asset_obj) {
        asset_obj->Update();
        asset_obj->UpdateGL();
    }
    if (asset_collision_obj) {
        asset_collision_obj->Update();
        asset_collision_obj->UpdateGL();
    }
}
