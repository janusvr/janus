#include "geom.h"

bool GeomIOSystem::shutting_down = false;

GeomData::GeomData():
    nTris(0)
{

}

void GeomData::AddMaterial(const QString mat)
{
    if (!materials.contains(mat)) {
        materials[mat] = GeomMaterial();
    }
}

void GeomData::AddTriangle(const QString mat, uint32_t const mesh_UUID, GeomTriangle t)
{
    if (materials[mat].triangles.size() <= (int)mesh_UUID)
    {
        materials[mat].triangles.resize(mesh_UUID + 1);
        materials[mat].triangles[mesh_UUID].reserve(256);
    }
    materials[mat].triangles[mesh_UUID].push_back(t);
}

void GeomData::SetTextureFilename(const QString mat, const unsigned int channel, const QString filename)
{
    if (channel < ASSETSHADER_NUM_TEXTURES) {
        materials[mat].textures[channel].filename = filename.trimmed();
    }
}

void GeomData::Clear()
{    
    for (GeomMaterial & mat : materials) {
        for (int j=0; j<mat.textures.size(); ++j) {
            mat.textures[j].ba.clear();
            if (mat.textures[j].img) {
                delete mat.textures[j].img;
            }            
        }
    }

    materials.clear();
    nTris = 0;
}

unsigned int GeomData::GetNumTris() const
{
    return nTris;
}

unsigned int GeomData::GetNumMaterials() const
{
    return materials.size();
}

QList <QString> GeomData::GetMaterialNames() const
{
    return materials.keys();
}

QString GeomData::GetMaterialMatchName(const GeomMaterial & mat)
{
    QHash <QString, GeomMaterial>::iterator it;
    for (it=materials.begin(); it!=materials.end(); ++it) {
        GeomMaterial & m = it.value();
        if (mat == m) {
            return it.key();
        }
    }
    return QString();
}

GeomMaterial & GeomData::GetMaterial(const QString mat)
{
    return materials[mat];
}

QVector<GeomTriangle> & GeomData::GetTriangles(const QString mat, uint32_t const mesh_UUID)
{
    if (materials[mat].triangles.size() <= (int)mesh_UUID)
    {
        materials[mat].triangles.push_back(QVector<GeomTriangle>());
    }
    return materials[mat].triangles[mesh_UUID];
}

GeomVBOData & GeomData::GetVBOData(const QString mat, uint32_t const mesh_UUID)
{
    if (materials[mat].vbo_data.size() <= (int)mesh_UUID)
    {
        materials[mat].vbo_data.push_back(GeomVBOData());
    }
    return materials[mat].vbo_data[mesh_UUID];
}

GeomIOStream::GeomIOStream() :
    buffer(NULL),
    webasset(NULL)
{
//    qDebug() << "GeomIOStream::GeomIOStream()" << this << url;
}

GeomIOStream::~GeomIOStream()
{
    Close();
//    qDebug() << "GeomIOStream::~GeomIOStream()" << this << url;
}

void GeomIOStream::SetData(const QByteArray & b)
{
//    qDebug() << "GeomIOStream::SetData" << url << b.size();
    ba = b;
    buffer = new QBuffer(&ba);
    buffer->open(QBuffer::ReadOnly);
}

void GeomIOStream::SetUrl(QUrl u)
{
//    qDebug() << "GeomIOStream::SetUrl" << this << u;
    url = u;
}

QUrl GeomIOStream::GetUrl() const
{
    return url;
}

void GeomIOStream::SetWebAsset(QPointer <WebAsset> w)
{
    webasset = w;
}

QPointer <WebAsset> GeomIOStream::GetWebAsset()
{
    return webasset;
}

size_t GeomIOStream::FileSize() const
{
    return buffer->size();
}

void GeomIOStream::Flush()
{
    //no-op, we use this for reading only    
}

size_t GeomIOStream::Read(void *pvBuffer, size_t pSize, size_t pCount)
{
    if (pSize <= 0 || pCount <= 0) {
        return 0;
    }

    const qint64 read_count = buffer->read((char *)pvBuffer, pSize * pCount);
    if (read_count > 0) {
        return read_count / pSize;
    }
    else {
        return 0;
    }
}

aiReturn GeomIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
    const qint64 buf_pos = buffer->pos();
    const size_t buffersize = buffer->size();

    qint64 new_pos = pOffset;
    if (pOrigin == aiOrigin_CUR) {
        new_pos += buf_pos;
    }
    else if (pOrigin == aiOrigin_END) {
        new_pos += buffersize;
    }

    return buffer->seek(new_pos) ? aiReturn_SUCCESS : aiReturn_FAILURE;
}

size_t GeomIOStream::Tell() const
{
    return buffer->pos();
}

size_t GeomIOStream::Write(const void *pvBuffer, size_t pSize, size_t pCount)
{
    return buffer->write((char *)pvBuffer, pSize * pCount);
}

void GeomIOStream::Close()
{
//    qDebug() << "GeomIOStream::Close()" << url;
    if (buffer) {
        if (buffer->isOpen()) {
            buffer->close();
        }
        delete buffer;
        buffer = NULL;

        ba.clear();
    }
}

GeomIOSystem::GeomIOSystem() :
    gzipped(false),
    fake_extension_added(false)
{

}

GeomIOSystem::~GeomIOSystem()
{
    Clear();
}

void GeomIOSystem::SetGZipped(const bool b)
{
    gzipped = b;
}

void GeomIOSystem::SetFakeExtensionAdded(const bool b, const QString s)
{
    fake_extension_added = b;
    fake_extension_string = s;
}

void GeomIOSystem::SetBasePath(QUrl u)
{
    base_path = u;
}

void GeomIOSystem::Clear()
{
    for (int i=0; i<streams.size(); ++i) {
        delete streams[i];
        streams[i] = NULL;
    }
    streams.clear();
    data_cache.clear();
}

void GeomIOSystem:: SetMTLFilePath(const QString & s)
{
    mtl_file_path = s;
}

void GeomIOSystem::SetShuttingDown(const bool b)
{
    shutting_down = b;
}

float GeomIOSystem::UpdateRequests()
{
//    qDebug() << "GeomIOSystem::UpdateRequests()" << base_path << this; // this << streams.size();
    int num_streams = 0;
    float progress = 0.0f;

    for (int i=0; i<streams.size(); ++i) {        
        if (streams[i] && streams[i]->GetWebAsset().isNull()) {
            WebAsset * w = new WebAsset;
            streams[i]->SetWebAsset(w);
            w->Load(streams[i]->GetUrl());
        }
        else if (streams[i] && streams[i]->GetWebAsset()) {
            ++num_streams;
            progress += streams[i]->GetWebAsset()->GetProgress();
        }
    }

    return (num_streams == 0)?0.0f:progress / float(num_streams);
}

Assimp::IOStream * GeomIOSystem::Open(const char *pFile, const char *)
{
//    qDebug() << "GeomIOSystem::Open1" << pFile;
    return Open(pFile);
}

Assimp::IOStream * GeomIOSystem::Open(const std::string & pFile, const std::string &)
{
    return Open(pFile.c_str());
}

Assimp::IOStream * GeomIOSystem::Open(const char *pFile)
{
//    qDebug() << "GeomIOSystem::Open" << pFile;
    QString p(pFile);
    QUrl u;

    if (p.right(4).toLower() == ".mtl" && !mtl_file_path.isEmpty()) {
        u = base_path.resolved(QUrl(mtl_file_path));
    }
    else if (gzipped && base_path.toString().right(p.length()) == p) {
        u = base_path.resolved(QUrl(p+".gz"));
    }
    else if (fake_extension_added && p.right(fake_extension_string.length()) == fake_extension_string) {
        u = base_path.resolved(QUrl(p.left(p.length()-fake_extension_string.length())));
    }
    else if (p.lastIndexOf("http://", -1, Qt::CaseInsensitive) > 0) {
        p = p.right(p.length()-p.lastIndexOf("http://", -1, Qt::CaseInsensitive));
        u = QUrl(p);
    }
    else if (p.lastIndexOf("https://", -1, Qt::CaseInsensitive) > 0) {
        p = p.right(p.length()-p.lastIndexOf("https://", -1, Qt::CaseInsensitive));
        u = QUrl(p);
    }
    else {
        u = base_path.resolved(QUrl(pFile));
    }

    //remove redundant double slashes
    const int i = p.lastIndexOf("//");
    if (i > 8) {
        p = p.left(i) + p.right(p.length()-i-1);
        u = base_path.resolved(QUrl(p));
    }

    const QString u_str = u.toString();
    GeomIOStream * s = new GeomIOStream();
    s->SetUrl(u);

    if (data_cache.contains(u_str)) {
//        qDebug() << "GeomIOSystem::Open Cache hit!" << u_str;
        s->SetData(data_cache[u_str]);
        return s;
    }

    streams.push_back(s);
//    qDebug() << " pushed back stream" << streams.size() << data_cache.size();

    QTime load_time;
    load_time.start();

    float load_progress = 0.0f;

//    qDebug() << "GeomIOSystem::Open" << pFile;
    while (!shutting_down) //release 60.0 - shutting down, it is critical we exit these loops, otherwise active threads do not allow the application to exit cleanly
    {
        QPointer <WebAsset> w = s->GetWebAsset();
        if (w) {
            if (w->GetLoaded()) {
                break;
            }
            if (w->GetError()) {
                break;
            }
            if (w->GetFinished()) {
                break;
            }
            //62.5 - abort this loop waiting if we get no download progress in 5 seconds
            if (load_time.elapsed() > 5000) {
                const float cur_progress = w->GetProgress();
                if (load_progress == cur_progress) {
                    qDebug() << "GeomIOSystem::Open (" << this << ") aborted wait for: " << base_path.toString() << pFile;
                    break;
                }
                load_progress = cur_progress;
                load_time.restart();
            }
        }

        QThread::yieldCurrentThread();
    };

//    qDebug() << "GeomIOSystem::Open" << this << streams.size() << pFile;
    QPointer <WebAsset> w = s->GetWebAsset();
    if (w && w->GetLoaded() && !w->GetError()) {
        if (u_str.right(7).toLower().contains(".obj") && !mtl_file_path.isEmpty()) {
            //61.0 mtllib override
            QByteArray b = w->GetData();
            if (!b.left(1000).contains("mtllib ")) {
                w->SetData(QString("mtllib " + mtl_file_path + "\n").toLatin1() + b);
            }
        }

        //cache the data if fetched again
        if (!data_cache.contains(u_str)) {
            data_cache[u_str] = w->GetData();
        }

        s->SetData(data_cache[u_str]);
        return s;
    }
    else
    {
        //cache errors - improves loading speed a lot
        if (!data_cache.contains(u_str)) {
            data_cache[u_str] = QByteArray();
        }

        streams.removeOne(s); // Remove the stream if it failed so we don't try to update it later.
        delete s;
    }
    return NULL;
}

void GeomIOSystem::Close(Assimp::IOStream *pFile)
{
//    qDebug() << "GeomIOSystem::Close" << this << streams.size();
    for (int i=0; i<streams.size(); ++i) {
        if (streams[i].data() == pFile) {
//            qDebug() << "GeomIOSystem::Close" << this << streams.size();
            //streams[i]->Close();
            delete streams[i];
            streams[i] = NULL;
        }
    }
//    if (dynamic_cast<GeomIOStream *>(pFile)) {
//        dynamic_cast<GeomIOStream *>(pFile)->Close();
//    }
}

bool GeomIOSystem::ComparePaths(const char *one, const char *second) const
{
    return (QUrl(one) == QUrl(second));
}

bool GeomIOSystem::ComparePaths(const std::string &one, const std::string &second) const
{
    return ComparePaths(one.c_str(), second.c_str());
}

bool GeomIOSystem::Exists(const std::string &pFile) const
{
    return Exists(pFile.c_str());
}

bool GeomIOSystem::Exists(const char *pFile) const
{
    Q_UNUSED(pFile)
//    qDebug() << "GeomIOSystem::Exists" << pFile;
    return true;
}

bool GeomIOSystem::PushDirectory( const std::string & )
{
    return true;
}

bool GeomIOSystem::PopDirectory()
{
    return true;
}

char GeomIOSystem::getOsSeparator() const
{
    return '/';
}

Geom::Geom() :
    center(false),
    ready(false),
    started(false),
    error(false),
    textures_started(false),
    textures_ready(false),
    texture_progress(0.0f),
    progress(0.0f),
    built_vbos(false),
    bbox_min(FLT_MAX, FLT_MAX, FLT_MAX),
    bbox_max(-FLT_MAX, -FLT_MAX, -FLT_MAX),
    tex_clamp(false),
    tex_linear(true),
    tex_compress(false),
    tex_mipmap(true),
    tex_premultiply(true),
    tex_alpha("undefined"),
    tex_colorspace("sRGB"),
    scene(NULL),
    uses_tex_file(false),
    cur_time(0.0),
    anim_speed(1.0f),
    loop(false)
{
//    qDebug() << "Geom::Geom()" << this;
    time.start();
    skin_joints.resize(ASSETSHADER_MAX_JOINTS);
    final_poses.resize(ASSETSHADER_MAX_JOINTS);
    iosystem = new GeomIOSystem();    
}

Geom::~Geom()
{
//    qDebug() << "Geom::~Geom()" << this;
//    delete iosystem; //59.9 - should we delete this iosystem?  memleak if we don't
}

void Geom::SetPath(const QString & p)
{
    path = p;
}

unsigned int Geom::GetNumTris() const
{
    return data.GetNumTris();
}

void Geom::SetReady(const bool b)
{
    ready = b;
}

bool Geom::GetReady() const
{
    return ready;
}

bool Geom::GetStarted() const
{
    return started;
}

float Geom::GetProgress() const
{
//    qDebug() << "Geom::GetProgress()" << path << progress;
    return (error ? 1.0f : progress);
}

bool Geom::GetTexturesReady() const
{
    return textures_ready;
}

float Geom::GetTextureProgress() const
{
    return (error ? 1.0f : texture_progress);
}

QVector3D Geom::GetBBoxMin() const
{
    return bbox_min;
}

QVector3D Geom::GetBBoxMax() const
{
    return bbox_max;
}

 GeomData & Geom::GetData()
{
    return data;
}

void Geom::SetTextureClamp(const bool b)
{
    tex_clamp = b;
}

bool Geom::GetTextureClamp() const
{
    return tex_clamp;
}

void Geom::SetTextureLinear(const bool b)
{
    tex_linear = b;
}

bool Geom::GetTextureLinear() const
{
    return tex_linear;
}

void Geom::SetTextureCompress(const bool b)
{
    tex_compress = b;
}

bool Geom::GetTextureCompress() const
{
    return tex_compress;
}

void Geom::SetTextureMipmap(const bool b)
{
    tex_mipmap = b;
}

bool Geom::GetTextureMipmap() const
{
    return tex_mipmap;
}

void Geom::SetTexturePreMultiply(const bool b)
{
    tex_premultiply = b;
}

bool Geom::GetTexturePreMultiply() const
{
    return tex_premultiply;
}

void Geom::SetTextureAlphaType(const QString & alpha_type)
{
    tex_alpha = alpha_type;
}

QString Geom::GetTextureAlphaType() const
{
    return tex_alpha;
}

void Geom::SetTextureColorSpace(const QString & color_space)
{
    tex_colorspace = color_space;
}

QString Geom::GetTextureColorSpace() const
{
    return tex_colorspace;
}

void Geom::SetCenter(const bool b)
{
    center = b;
}

bool Geom::GetCenter() const
{
    return center;
}

void Geom::SetMesh(const QVariantMap & property_list)
{
    //parse the information into a file that assimp can load
//    qDebug() << "Geom::SetMesh" << property_list;
    const QVariantList verts = property_list["mesh_verts"].toList();
    const QVariantList ns = property_list["mesh_normals"].toList();
    const QVariantList uvs = property_list["mesh_uvs"].toList();
    const QVariantList faces = property_list["mesh_faces"].toList();
    const QVariantList face_uvs = property_list["mesh_face_uvs"].toList();
    const QVariantList face_normals = property_list["mesh_face_normals"].toList();

    mesh_data.clear();

    QTextStream ofs(&mesh_data);

    //verts
    for (int i=0; i<verts.size()-2; i+=3) {
        ofs << QString("v ") << verts[i].toString() << " " << verts[i+1].toString() << " " << verts[i+2].toString() << '\n';
    }

    //uvs
    for (int i=0; i<ns.size()-2; i+=3) {
        ofs << QString("n ") << ns[i].toString() << " " << ns[i+1].toString() << " " << ns[i+2].toString() << '\n';
    }

    //uvs
    for (int i=0; i<uvs.size()-2; i+=3) {
        ofs << QString("vt ") << uvs[i].toString() << " " << uvs[i+1].toString() << " " << uvs[i+2].toString() << '\n';
    }

    //faces (Note!  OBJ files use 1-indexing)
    for (int i=0; i<faces.size()-2; i+=3) {
        if (!uvs.isEmpty() && i<face_uvs.size()-2 && !ns.isEmpty() && i <face_normals.size()-2) {
            ofs << QString("f " )
                << faces[i].toInt()+1 << "/" << face_uvs[i].toInt()+1 << "/" << face_normals[i].toInt()+1 << " "
                << faces[i+1].toInt()+1 << "/" << face_uvs[i+1].toInt()+1 << "/" << face_normals[i+1].toInt()+1 << " "
                << faces[i+2].toInt()+1 << "/" << face_uvs[i+2].toInt()+1 << "/" << face_normals[i+2].toInt()+1 << '\n';
        }
        else if (!uvs.isEmpty() && i<face_uvs.size()-2) {
            ofs << QString("f " )
                << faces[i].toInt()+1 << "/" << face_uvs[i].toInt()+1 << " "
                << faces[i+1].toInt()+1 << "/" << face_uvs[i+1].toInt()+1 << " "
                << faces[i+2].toInt()+1 << "/" << face_uvs[i+2].toInt()+1 << '\n';
        }
        else if (!ns.isEmpty() && i <face_normals.size()-2) {
            ofs << QString("f " )
                << faces[i].toInt()+1 << "//" << face_normals[i].toInt()+1 << " "
                << faces[i+1].toInt()+1 << "//" << face_normals[i+1].toInt()+1 << " "
                << faces[i+2].toInt()+1 << "//" << face_normals[i+2].toInt()+1 << '\n';
        }
        else {
            ofs << QString("f " ) << faces[i].toInt()+1 << " " << faces[i+1].toInt()+1 << " " << faces[i+2].toInt()+1 << '\n';
        }
    }

    ofs.flush();
//    qDebug() << mesh_data;
}

bool Geom::GetHasMeshData() const
{
    return !mesh_data.isEmpty();
}

void Geom::Load()
{    
    if (started || ready) {
        return;
    }

    started = true;

    //C++ method with IO handlers (load files from network)
    QString p = path;
    bool gzipped = false;
    if (p.right(3).toLower() == ".gz") {
        p = p.left(p.length()-3);
        gzipped = true;
    }

    //62.7 - necessary to give assimp a .glb filename hint, when they store files without extension/MIME data
    bool fake_extension = false;
    QString fake_extension_str;
    if (path.contains("content.decentraland.today/contents")) {
        fake_extension = true;
        fake_extension_str = ".glb";
    }
    else if (path.contains("?v=")) {
        const int ind1 = path.lastIndexOf("?v=");
        const int ind0 = path.lastIndexOf(".", ind1-5);
        fake_extension = true;
        fake_extension_str = path.mid(ind0, ind1-ind0);
    }
    if (fake_extension) {
        p += fake_extension_str;
    }

    if (iosystem) {
        iosystem->SetGZipped(gzipped);
        iosystem->SetFakeExtensionAdded(fake_extension, fake_extension_str);
        iosystem->SetBasePath(QUrl(p));
        importer.SetIOHandler(iosystem);
    }
    //scene = importer.ReadFile(base_path.toLocal8Bit().data(), aiProcessPreset_TargetRealtime_Fast);
    auto post_process_flags =
        aiProcess_LimitBoneWeights
//        | aiProcess_GenNormals
        | aiProcess_GenSmoothNormals //59.13 - this flag is necessary for smooth normal generation
        | aiProcess_GenUVCoords
        | aiProcess_Triangulate
        | aiProcess_SortByPType
        | aiProcess_JoinIdenticalVertices
        | aiProcess_SplitLargeMeshes
        | aiProcess_ValidateDataStructure
            //62.3 - enabling some of these messes up our FBX support
//        | aiProcess_OptimizeGraph // SLOW
        //| aiProcess_OptimizeMeshes // SLOW
//        | aiProcess_RemoveRedundantMaterials // SLOW
//        | aiProcess_ImproveCacheLocality // SLOW
//        | aiProcess_FindInstances // SLOWW
            ;

    //59.4 - catch exceptions in assimp
    try {
        if (mesh_data.isEmpty()) {
            scene = (aiScene *)(importer.ReadFile(p.toLocal8Bit().data(), post_process_flags));
        }
        else {
            scene = (aiScene *)(importer.ReadFileFromMemory(mesh_data.data(), mesh_data.size(), post_process_flags, "obj"));
        }
    }
    catch (int e) {
        qDebug() << "Geom::Load() caught assimp exception, code:" << e;
    }

    if (scene) {
        aiVector3D scene_min, scene_max;
        get_bounding_box(&scene_min,&scene_max);

        bbox_min.setX(scene_min.x);
        bbox_min.setY(scene_min.y);
        bbox_min.setZ(scene_min.z);

        bbox_max.setX(scene_max.x);
        bbox_max.setY(scene_max.y);
        bbox_max.setZ(scene_max.z);

        PrepareVBOs();
    }
    else {        
        ready = true;
        error = true;
        error_str = QString("Geom::Load() Error: ") + importer.GetErrorString() + " Path: " + path;
        MathUtil::ErrorLog(error_str);// + "File: " + path);
    }

//    qDebug() << "Geom::Load completed" << path;
}

void Geom::Unload()
{
    //iosystem.Clear();
//    importer.FreeScene(); //called automatically by destructor
//    scene = NULL;
    /*if (scene) {
        delete scene;
        scene = nullptr;
    }*/
    ready = false;
    started = false;
    error = false;
    textures_started = false;
    textures_ready = false;
    data.Clear();

    anims.clear();
    bone_to_boneid.clear();
    bone_to_node.clear();
    node_list.clear();
    linked_anim.clear();
    skin_joints.clear();
    final_poses.clear();
    bone_offset_matrix.clear();
}

void Geom::Update()
{
//    qDebug() << "Geom::Update()" << path << ready << iosystem;
    if (scene && ready && !textures_started) {
        textures_started = true;

        //load textures from main thread
        QList <QString> mat_names = data.GetMaterialNames();
        for (int i=0; i<mat_names.size(); ++i) {
            GeomMaterial & mat = data.GetMaterial(mat_names[i]);

//            qDebug() << "Geom::Update()" << path << mat_names[i] << i << mat.textures.size();
            for (int j=0; j<mat.textures.size(); ++j) {
                QString s = mat.textures[j].filename.trimmed();
//                qDebug() << "unprocessed path" << s;
                s = QUrl::toPercentEncoding(s,":/"); // params for exclude/include
                s = s.replace("\\", "/");
                s = s.replace("%5C", "/");               

                // Search embedded textures for a filename match
                QString s3(mat.textures[j].filename_unresolved);
                if (s3.contains("/")) {
                    s3 = s3.right(s3.length()-s3.lastIndexOf("/"));
                }
                if (s3.contains("\\")) {
                    s3 = s3.right(s3.length()-s3.lastIndexOf("\\"));
                }

                int tex_index = -1;


//                qDebug() << "Geom::Update()" << path << s << s3 << scene->mNumTextures;
                if (s3.left(1) == "*") { //62.3 - hacky fix for Assimp where it uses a special asterisk indexing scheme
                    const unsigned int t_ind = s3.mid(1).toInt();
                    //if (t_ind >= 0 && t_ind <scene->mNumTextures) {
                    if (t_ind <scene->mNumTextures) {
                        tex_index = t_ind;
                    }
                }
                else {
                    for (unsigned int k=0; k<scene->mNumTextures; ++k) {
                        QString s2(scene->mTextures[k]->mFilename.C_Str());
                        if (!s3.isEmpty() && s2.contains(s3)) {
                            tex_index = k;
                            break;
                        }
                    }
                }

                if (tex_index >= 0) {
//                    qDebug() << "Geom::Update() * test" << ok << tex_index << scene->mNumTextures << s+"."+QString(scene->mTextures[tex_index]->achFormatHint);
                    aiTexture * t = scene->mTextures[tex_index];
                    unsigned int size_data = 0;

                    if (t->mHeight > 0) {
                        size_data = t->mWidth * t->mHeight * 4; //width/height in texels, mult by 4 bytes as they are RGBA8888 formatted
                    }
                    else {
                        size_data = t->mWidth; //specified in bytes, not # texels
                    }

                    if (t->pcData == (aiTexel*)0xabababababababab)
                    {
                        qDebug() << "ERROR: Geom::Update() bad textureData pointer";
                    }
                    mat.textures[j].ba.resize(size_data);
                    memcpy(mat.textures[j].ba.data(), t->pcData, size_data);

                    QPointer <AssetImage> new_img = new AssetImage();
                    new_img->GetProperties()->SetTexClamp(tex_clamp);
                    new_img->GetProperties()->SetTexLinear(tex_linear);
                    new_img->GetProperties()->SetTexCompress(tex_compress);
                    new_img->GetProperties()->SetTexMipmap(tex_mipmap);
                    new_img->GetProperties()->SetTexPremultiply((j == 0) ? tex_premultiply : false);
                    new_img->GetProperties()->SetTexAlpha((j == 0) ? tex_alpha : "none");
                    // We use linear colorspace for data textures like roughness, normals, heightmaps etc.
                    // We set texture 2-Specular/3-Normal/4-Height/7-DetailMask to linear as they are data-textures which should have
                    // linear data stored in them (i.e. 8-bit 128 grey is half-smoothness)
                    new_img->GetProperties()->SetTexColorspace((j == 0 || j == 1 || j == 5 || j == 6 || j == 8) ? tex_colorspace : "linear");
                    new_img->SetSrc(path, s+"."+QString(scene->mTextures[tex_index]->achFormatHint));
                    new_img->CreateFromData(mat.textures[j].ba);
                    mat.textures[j].img = new_img;

                }
                else if (!s.isEmpty()) {
//                    qDebug() << "External texture" << s;
                    QPointer <AssetImage> new_img = new AssetImage();
                    new_img->GetProperties()->SetTexClamp(tex_clamp);
                    new_img->GetProperties()->SetTexLinear(tex_linear);
                    new_img->GetProperties()->SetTexCompress(tex_compress);
                    new_img->GetProperties()->SetTexMipmap(tex_mipmap);
                    new_img->GetProperties()->SetTexPremultiply((j == 0) ? tex_premultiply : false);
                    new_img->GetProperties()->SetTexAlpha((j == 0) ? tex_alpha : "none");
                    new_img->GetProperties()->SetTexColorspace((j == 0 || j == 1 || j == 5 || j == 6 || j == 8) ? tex_colorspace : "linear");
                    new_img->SetSrc(s, s);
                    new_img->Load();
                    mat.textures[j].img = new_img;
                }
            }
        }       
    }
    else if (ready && textures_started && !textures_ready) {

        bool tex_ready = true;
        int nTextures = 0;
        float nProgressTextures = 0.0f;

        //load textures from main thread
        QList <QString> mat_names = data.GetMaterialNames();

        for (int i=0; i<mat_names.size(); ++i) {
            GeomMaterial & mat = data.GetMaterial(mat_names[i]);
            for (int j=0; j<mat.textures.size(); ++j) {
                const QPointer <AssetImage> img = mat.textures[j].img;

                nTextures++;

                if (img) {
                    if (img->GetError()) {
//                        qDebug() << "img error";
                        //59.0 - ad-hoc fix for broken texture (just use first un-broken one)
                        for (int k=0; k<mat_names.size(); ++k) {
                            if (i == k) {
                                continue;
                            }

                            for (int l=0; l<data.GetMaterial(mat_names[k]).textures.size(); ++l) {
                                QPointer <AssetImage> new_img = data.GetMaterial(mat_names[k]).textures[l].img;
                                if (new_img && !new_img->GetError()) {
//                                    qDebug() << "found match" << i << j << k << l;                                    
                                    mat.textures[j].img = new_img;
                                    tex_ready = false;
                                    break;
                                }
                            }
                        }
                    }
                    else if (!img->GetFinished()) {
//                        qDebug() << "img unfinished" << img->GetS("src");
                        tex_ready = false;
                        break;
                    }

                    if (img->GetFinished() || img->GetError()) {
//                        qDebug() << "img applying progress" << img->GetProgress();
                        nProgressTextures += img->GetProgress();
                    }
                }
            }
        }

        if (tex_ready) {
            textures_ready = true;
            texture_progress = 1.0f;
        }
        else {
            texture_progress = (nTextures == 0)?0.0f:nProgressTextures / float(nTextures);
        }
    }

    if (iosystem) {
        progress = iosystem->UpdateRequests();
    }

    if (ready) {
        progress = 1.0f;
        UpdateAnimation();
    }

    if (textures_ready) {
        texture_progress = 1.0f;
    }
}

bool Geom::UpdateGL()
{
//    qDebug() << "Geom::UpdateGL()" << path << ready << textures_ready;
    if (!ready) {
        return false;
    }

    if (!built_vbos) {
        BuildVBOsGL();
        return true;
    }

    return false;
}

void Geom::SetMTLFile(const QString & s)
{
    mtl_file_path = s;
    if (iosystem) {
        iosystem->SetMTLFilePath(s);
    }
}

void Geom::SetMaterialTexture(const QString & tex_url, const int channel)
{
    QUrl u(path);
    data.SetTextureFilename("0", channel, u.resolved(tex_url).toString());
}

void Geom::DrawGL(QPointer <AssetShader> shader, const QColor col)
{
//    qDebug() << "Geom::DrawGL" << path << ready << error << shader;
    if (!ready || error || shader == NULL || !shader->GetCompiled()) {
        return;
    }

    QList <QString> materials = data.GetMaterialNames();

    shader->SetConstColour(QVector4D(col.redF(), col.greenF(), col.blueF(), col.alphaF())); //break the colour out of the display list
    Renderer * renderer = Renderer::m_pimpl;

    for (int i=0; i<materials.size(); ++i)
    {
        GeomMaterial & mat = data.GetMaterial(materials[i]);

        const int mesh_count = mat.vbo_data.size();
        if (mesh_count == 0)
        {
            // Skip materials with no meshes
            continue;
        }

        QVector <GeomTexture> & textures = (uses_tex_file ? data.GetMaterial("0").textures : mat.textures); //56.0 - use global texture if doing tex[0/1/2/3/4] override

        if (textures[5].img.isNull() && (mat.ka.redF() > 0.0f || mat.ka.greenF() > 0.0f || mat.ka.blueF() > 0.0f))
        {
            shader->SetAmbient(QVector3D(mat.ka.redF(), mat.ka.greenF(), mat.ka.blueF()));
        }
        else
        {
            shader->SetAmbient(QVector3D(1,1,1));
        }

        if (textures[6].img.isNull())
        {
            shader->SetEmission(QVector3D(mat.ke.redF(), mat.ke.greenF(), mat.ke.blueF()));
        }
        else
        {
            shader->SetEmission(QVector3D(0,0,0));
        }

        //58.0 - Change diffuse only when lighting is enabled (or some surfaces, menu/websurface, appear dark),
        //later change to restore since it causes inconsistent behaviour
        //changes again, this time to not use it if there's a texture override (this should work ok)
        if (textures[0].img.isNull() && !shader->GetOverrideTexture())
        {
            shader->SetDiffuse(QVector4D(mat.kd.redF(), mat.kd.greenF(), mat.kd.blueF(), mat.kd.alphaF()));
        }
        else
        {
            if (mat.kd.alphaF() != 0.0f) {
                shader->SetDiffuse(QVector4D(1, 1, 1, mat.kd.alphaF())); //61.0 - support transparency (d value in OBJ .mtl)
            }
            else {
                shader->SetDiffuse(QVector4D(1, 1, 1, 1)); //61.0 - support transparency (d value in OBJ .mtl)
            }
        }

        if (textures[1].img.isNull())
        {
            shader->SetSpecular(QVector3D(mat.ks.redF(), mat.ks.greenF(), mat.ks.blueF()));
        }
        else
        {
            shader->SetSpecular(QVector3D(0.04f,0.04f,0.04f));
        }

        if (textures[2].img.isNull())
        {
            shader->SetShininess(qMin(qMax(mat.ns, 0.0f), 2048.0f));
        }
        else
        {
            shader->SetShininess(20.0f);
        }

        //iterate over and activate all textures
        if (!shader->GetOverrideTexture())
        {
            shader->SetUseTextureAll(false);

            //62.9 - lightmap override
            if (shader->GetOverrideLightmap()) {
                shader->SetUseTexture(8, true); //8 - material lightmap channel
            }

            for (int j=0; j<textures.size(); ++j)
            {
                QPointer <AssetImage> a = textures[j].img;
                if (a) {
                    a->UpdateGL();
                    auto tex_id = a->GetTextureHandle(true);
                    Renderer::m_pimpl->BindTextureHandle(j, tex_id);
                    shader->SetUseTexture(j, true);
                }
            }
        }

        for (int mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
        {
            GeomVBOData & vbo_data = data.GetVBOData(materials[i], mesh_index);

            if (!vbo_data.m_mesh_handle) {
                continue;
            }

            if (vbo_data.use_skelanim) {
                shader->SetUseSkelAnim(true);
                shader->SetSkelAnimJoints(skin_joints);
            }
            else {
                shader->SetUseSkelAnim(false);
            }

            AbstractRenderCommand base_command(PrimitiveType::TRIANGLES,
                                    vbo_data.GetIndexCount(),
                                    0,
                                    0,
                                    0,
                                    vbo_data.m_mesh_handle,
                                    shader->GetProgramHandle(),
                                    shader->GetFrameUniforms(),
                                    shader->GetRoomUniforms(),
                                    shader->GetObjectUniforms(),
                                    shader->GetMaterialUniforms(),
                                    renderer->GetCurrentlyBoundTextures(),
                                    renderer->GetDefaultFaceCullMode(),
                                    renderer->GetDepthFunc(),
                                    renderer->GetDepthMask(),
                                    renderer->GetStencilFunc(),
                                    renderer->GetStencilOp(),
                                    renderer->GetColorMask());

            const int instance_count = vbo_data.m_instance_transforms.size();
            for (int instance_index = 0; instance_index < instance_count; ++instance_index)
            {
                MathUtil::PushModelMatrix();
                MathUtil::MultModelMatrix(vbo_data.m_instance_transforms[instance_index]);
                shader->UpdateObjectUniforms();
                MathUtil::PopModelMatrix();
                base_command.SetObjectUniforms(shader->GetObjectUniforms());
                renderer->PushAbstractRenderCommand(base_command);
            }
        }

        //iterate over and deactivate all textures
        if (!shader->GetOverrideTexture())
        {
            for (int j=0; j<textures.size(); ++j)
            {
                QPointer <AssetImage> a = textures[j].img;
                if (a) {
                    shader->SetUseTexture(j, false);
                }
            }
        }

        // Default some material parameters
        shader->SetAmbient(QVector3D(1,1,1));
        shader->SetEmission(QVector3D(0,0,0));
        shader->SetDiffuse(QVector3D(1,1,1));
        shader->SetSpecular(QVector3D(0.04f,0.04f,0.04f));
        shader->SetShininess(20.0f);
    }

    shader->SetUseSkelAnim(false);
    shader->SetUseTextureAll(false);
}

void Geom::SetUsesTexFile(const bool b)
{
    uses_tex_file = b;
}

bool Geom::GetUsesTexFile() const
{
    return uses_tex_file;
}

void Geom::SetLinkAnimation(QPointer <Geom> anim)
{
//    qDebug() << "GeomFBX::SetLinkAnimation" << anim_fbx;
    //to link animations means we need to find model<->model one to one correspondences based on the long long int id's...
    if (!anim.isNull() && linked_anim != anim && anim->GetReady()) {
        linked_anim = anim;
        cur_time = 0.0;
    }
}

QPointer <Geom> Geom::GetLinkAnimation()
{
    return linked_anim;
}

QMatrix4x4 Geom::GetFinalPose(const QString& bone_id)
{
    const QString b = bone_id.toLower();

    if (bone_to_boneid.contains(b) && bone_to_boneid[b] < final_poses.size()) {
        return final_poses[bone_to_boneid[b]];
    }
    return QMatrix4x4();
}

void Geom::UpdateAnimation()
{
    //53.9 - make avatars update no less than 20ms apart
    if (ready && !error && linked_anim && !linked_anim->GetError()) {

        //const QPointer <Geom> geom = (linked_anim ? linked_anim : QPointer <Geom> (this)); //58.0 - this causes a crash for some reason
        const QPointer <Geom> geom = linked_anim;
//        Geom * geom = linked_anim ? linked_anim.data() : this;

//        qDebug() << "Geom::UpdateAnimation()" << base_path << geom->scene;
        if (geom && geom->scene && geom->scene->mNumAnimations > 0) {

//            qDebug() << "Geom::UpdateAnimation() anim" << base_path << geom->scene << geom->scene->mNumAnimations << geom->scene->mAnimations[0];
            aiAnimation * a = geom->scene->mAnimations[0];

            //t0 and t1 expressed in seconds
            double t0 = 0.0;
            double t1 = a->mDuration;

            const double new_time = double(time.restart()) * 0.001 * a->mTicksPerSecond * anim_speed;

            if (loop) {
                if (t1 > t0) {
                    cur_time = t0 + fmod((cur_time - t0) + new_time, t1-t0); //49.42 - fixed potential SIGFPE
                }
                else {
                    cur_time = t0;
                }
            }
            else {
                double new_cur_time = t0 + (((cur_time - t0) + new_time));
                cur_time = ((new_cur_time < t1) ? new_cur_time : t1);
            }

            CalculateFinalPoses(); //this call is the FPS killer
        }
    }
}

QString Geom::GetProcessedNodeName(const QString s)
{
    //return s;
    QString n = s.toLower();
    if (n.contains(":")) {
        n = n.right(n.length() - n.lastIndexOf(":") - 1);
    }
    return n;
}

void Geom::PrepareVBOs()
{
    //qDebug() << "Geom::PrepareVBOs()" << scene << ready;
    if (scene == NULL)
    {
        ready = true;
        return;
    }

    QVector3D c;
    if (center)
    {
        c = (bbox_min + bbox_max) * 0.5f;
    }

    //set up animations
    //NOTE: mesh may not have animations, but be linked to one with animations, cannot use this as a shortcut
    if (scene->HasAnimations())
    {
        for (unsigned int i=0; i<scene->mAnimations[0]->mNumChannels; ++i)
        {
            QString orig_s = GetProcessedNodeName(scene->mAnimations[0]->mChannels[i]->mNodeName.C_Str());
            //qDebug() << "Geom::PrepareVBOs()" << path << orig_s;
            anims[orig_s] = scene->mAnimations[0]->mChannels[i];
        }
    }

    /* draw all meshes assigned to this node */
    //iterate through everything
    QVector<aiNode*> nodes_to_process;
    nodes_to_process.reserve(512);
    QVector<QMatrix4x4> nodes_parent_xforms;
    nodes_parent_xforms.reserve(512);
    QVector<int> node_depth;
    node_depth.reserve(512);
    node_list.reserve(512);

    if (scene->mRootNode)
    {
        nodes_to_process.push_back(scene->mRootNode);
        nodes_parent_xforms.push_back(QMatrix4x4());
        node_depth.push_back(0);

        m_globalInverseTransform = aiToQMatrix4x4(scene->mRootNode->mTransformation).inverted();        
    }

    //on first pass, set up node hierarchy and node indexes for whole scene
    while (!nodes_to_process.empty())
    {
        aiNode * nd = nodes_to_process.back();
        nodes_to_process.pop_back();

        QMatrix4x4 m_p = nodes_parent_xforms.back();
        nodes_parent_xforms.pop_back();

        int n_depth = node_depth.back();
        node_depth.pop_back();

        const QMatrix4x4 m = m_p * aiToQMatrix4x4(nd->mTransformation);

        //process bone transformation hierarchy (just create a flat list)
        if (std::find(node_list.begin(), node_list.end(), nd) != node_list.end())
        {

            //TODO - save specific indexes so some bones can have special transforms applied to them
            //qDebug() << "Adding bone!" << nd->mName.C_Str() << "index" << node_list.size();
            const QString node_name = GetProcessedNodeName(nd->mName.C_Str());
            bone_to_node[node_name] = node_list.size();
            node_list.push_back(nd);
        }

        //add the children to process list
        for (unsigned int n = 0; n < nd->mNumChildren; ++n)
        {
            if (nd->mChildren[n])
            {
                nodes_to_process.push_back(nd->mChildren[n]);
                nodes_parent_xforms.push_back(m);
                node_depth.push_back(n_depth+1);
            }
        }
    }

    if (scene->mRootNode)
    {
        nodes_to_process.push_back(scene->mRootNode);
        nodes_parent_xforms.push_back(QMatrix4x4());
    }

    //on second pass, process meshes
    while (!nodes_to_process.empty())
    {
        aiNode * nd = nodes_to_process.back();
        nodes_to_process.pop_back();
        //qDebug() << "mName" << mesh->mName.C_Str() << n;        
        //62.9 - ad-hoc decentraland convention to reserve nodes with name "_collider" for collision only
        if (QString(nd->mName.C_Str()).contains("_collider")) {
            continue;
        }

        const QMatrix4x4 m_p = nodes_parent_xforms.back();
        nodes_parent_xforms.pop_back();

        const QMatrix4x4 m3 = aiToQMatrix4x4(nd->mTransformation);
        const QMatrix4x4 m = m_p * m3;

        //process meshes
        //qDebug() << "Geom::PrepareVBOs()" << nd->mNumMeshes;
        for (unsigned int n=0; n < nd->mNumMeshes; ++n)
        {

            const aiMesh * mesh = scene->mMeshes[nd->mMeshes[n]];
            const aiMaterial * mtl = scene->mMaterials[mesh->mMaterialIndex];                      

            if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
                // Skip meshes that are not triangulated as we do not support rendering them currently
				continue;
			}                        

            //material processing
            aiString mat_name2;
            QString mat_name;
            if(AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_NAME, &mat_name2))
            {
                mat_name = QString(mat_name2.C_Str());
            }
            else
            {
                mat_name = QString("mat") + QString::number(mesh->mMaterialIndex);
            }

            if (!data.GetMaterialNames().contains(mat_name))
            {
                //first determine if there is an existing texture with all the same attributes
                GeomMaterial mat;
                create_material(mtl, mat);
                const QString match_name = data.GetMaterialMatchName(mat);

                if (match_name.isEmpty())
                {
                    GeomMaterial & mat = data.GetMaterial(mat_name);
                    //qDebug() << "create material" << mat_name << scene->mNumTextures;
                    create_material(mtl, mat);
                }
                else
                {
                    mat_name = match_name;
                }
            }

            // If we have not already processed this mesh do so.
            GeomMaterial & mat = data.GetMaterial(mat_name);
            bool mesh_has_been_processed = false;
            int mesh_index = mat.mesh_keys.size();
            for (QPair<uint32_t, int>& mesh_key : mat.mesh_keys)
            {
                if (mesh_key.first == nd->mMeshes[n])
                {
                    mesh_has_been_processed = true;
                    mesh_index = mesh_key.second;
                    break;
                }
            }

            GeomVBOData & vbo_data = data.GetVBOData(mat_name, mesh_index);
            if (mesh_has_been_processed == false)
            {
                //mesh processing, for now only triangles are supported
                const uint32_t num_verts = mesh->mNumVertices;
                const uint32_t indice_offset = vbo_data.m_positions.size() / 4;
                vbo_data.m_indices.reserve(vbo_data.m_indices.size() + mesh->mNumFaces * 3);
                vbo_data.m_positions.reserve(vbo_data.m_positions.size() + num_verts * 4);
                vbo_data.m_normals.reserve(vbo_data.m_normals.size() + num_verts * 4);
                vbo_data.m_tex_coords.reserve(vbo_data.m_tex_coords.size() + num_verts * 4);
                vbo_data.m_colors.reserve(vbo_data.m_colors.size() + num_verts * 4);
                vbo_data.m_skel_anim_weights.reserve(vbo_data.m_skel_anim_weights.size() + num_verts * 4);
                vbo_data.m_skel_anim_indices.reserve(vbo_data.m_skel_anim_indices.size() + num_verts * 4);

                // Vertex indices
                for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
                {
                    aiFace const * face = &mesh->mFaces[faceIndex];
                    vbo_data.m_indices.push_back(face->mIndices[0] + indice_offset);
                    vbo_data.m_indices.push_back(face->mIndices[1] + indice_offset);
                    vbo_data.m_indices.push_back(face->mIndices[2] + indice_offset);
                } //for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)

                // Vertex attributes
                for (uint32_t vertex_index = 0; vertex_index < num_verts; ++vertex_index)
                {
                    // Vertex position
                    vbo_data.m_positions.push_back(float(mesh->mVertices[vertex_index].x));
                    vbo_data.m_positions.push_back(float(mesh->mVertices[vertex_index].y));
                    vbo_data.m_positions.push_back(float(mesh->mVertices[vertex_index].z));
                    vbo_data.m_positions.push_back(float(1.0f));

                    // Vertex normal
                    vbo_data.m_normals.push_back(float(mesh->mNormals[vertex_index].x));
                    vbo_data.m_normals.push_back(float(mesh->mNormals[vertex_index].y));
                    vbo_data.m_normals.push_back(float(mesh->mNormals[vertex_index].z));
                    vbo_data.m_normals.push_back(float(0.0f));

                    // Vertex UV0
                    float UVs_0_x = 0.0f;
                    float UVs_0_y = 0.0f;
                    if (mesh->mTextureCoords[0] != nullptr)
                    {
                        UVs_0_x = mesh->mTextureCoords[0][vertex_index].x;
                        UVs_0_y = mesh->mTextureCoords[0][vertex_index].y;
                    }

                    vbo_data.m_tex_coords.push_back(float(UVs_0_x));
                    vbo_data.m_tex_coords.push_back(float(UVs_0_y));

                    // Vertex UV1s
                    float UVs_1_x = UVs_0_x;
                    float UVs_1_y = UVs_0_y;
                    if (mesh->mTextureCoords[1] != nullptr)
                    {
                        UVs_1_x = mesh->mTextureCoords[1][vertex_index].x;
                        UVs_1_y = mesh->mTextureCoords[1][vertex_index].y;
                    }

                    vbo_data.m_tex_coords.push_back(float(UVs_1_x));
                    vbo_data.m_tex_coords.push_back(float(UVs_1_y));

                    // Vertex colors
                    float colors[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                    if (mesh->mColors[0] != nullptr)
                    {
                        colors[0] = mesh->mColors[0][vertex_index].r;
                        colors[1] = mesh->mColors[0][vertex_index].g;
                        colors[2] = mesh->mColors[0][vertex_index].b;
                        colors[3] = mesh->mColors[0][vertex_index].a;
                    }

                    // assosiate colors with alpha for correct blending
                    vbo_data.m_colors.push_back(float(colors[0] * colors[3]));
                    vbo_data.m_colors.push_back(float(colors[1] * colors[3]));
                    vbo_data.m_colors.push_back(float(colors[2] * colors[3]));
                    vbo_data.m_colors.push_back(float(colors[3]));
                } // for (uint32_t vertex_index = 0; vertex_index < num_verts; ++vertex_index)

                // Construct Physics Triangles
                const int triangle_count = vbo_data.m_indices.size() / 3;
                for (int triangle_index = 0; triangle_index < triangle_count; ++triangle_index)
                {
                    GeomTriangle tri;
                    for (uint32_t vertex_index = 0; vertex_index < 3; ++vertex_index)
                    {
                        auto const index = vbo_data.m_indices[triangle_index * 3 + vertex_index];

                        tri.p[vertex_index][0] = half_float::half(vbo_data.m_positions[index * 4    ]);
                        tri.p[vertex_index][1] = half_float::half(vbo_data.m_positions[index * 4 + 1]);
                        tri.p[vertex_index][2] = half_float::half(vbo_data.m_positions[index * 4 + 2]);

                        tri.n[vertex_index][0] = half_float::half(vbo_data.m_normals[index * 4    ]);
                        tri.n[vertex_index][1] = half_float::half(vbo_data.m_normals[index * 4 + 1]);
                        tri.n[vertex_index][2] = half_float::half(vbo_data.m_normals[index * 4 + 2]);

                        tri.t[vertex_index][0] = half_float::half(vbo_data.m_tex_coords[index * 4    ]);
                        tri.t[vertex_index][1] = half_float::half(vbo_data.m_tex_coords[index * 4 + 1]);
                    }

                    data.AddTriangle(mat_name, mesh_index, tri);
                } // for (uint32_t triangle_index = 0; triangle_index < triangle_count; ++triangle_index)

                //bone processing
                struct VertexBoneInfo {
                    QString bone;
                    uint8_t bone_index;
                    float weight;
                };

                QHash<uint32_t, QVector<VertexBoneInfo>> bone_info;

                // Prepare Bone weights and indices
                for (uint32_t bone_index = 0; bone_index < mesh->mNumBones; ++bone_index)
                {
                    for (uint32_t bone_weight_index = 0; bone_weight_index < mesh->mBones[bone_index]->mNumWeights; ++bone_weight_index)
                    {
                        VertexBoneInfo v;
                        v.bone = GetProcessedNodeName(mesh->mBones[bone_index]->mName.C_Str());
                        v.weight = mesh->mBones[bone_index]->mWeights[bone_weight_index].mWeight;
                        v.bone_index = 0;

                        if (!bone_to_boneid.contains(v.bone))
                        {
                            bone_to_boneid[v.bone] = bone_to_boneid.size();
                        }
                        v.bone_index = bone_to_boneid[v.bone];

                        if (bone_info[mesh->mBones[bone_index]->mWeights[bone_weight_index].mVertexId].size() < 4)
                        { //limit to 4 bone weights per vertex
                            bone_info[mesh->mBones[bone_index]->mWeights[bone_weight_index].mVertexId].push_back(v);
                        }

                        if (!bone_offset_matrix.contains(v.bone))
                        {
                            bone_offset_matrix[v.bone] = aiToQMatrix4x4(mesh->mBones[bone_index]->mOffsetMatrix);
                        }
                    }
                } // for (uint32_t bone_index = 0; bone_index < mesh->mNumBones; ++bone_index)

                // Skel anim indices and weights
                for (uint32_t vertex_index = 0; vertex_index < num_verts; ++vertex_index)
                {
                    QVector<VertexBoneInfo>& bones = bone_info[vertex_index];

                    const int bone_count = bones.size();
                    if (bone_count != 0)
                    {
                        vbo_data.use_skelanim = true;
                    }
                    float sum_weight = 0.0f;
                    uint8_t indices[4] = {0xff, 0xff, 0xff, 0xff};
                    float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};

                    for (int bone_index = 0; bone_index < bone_count; ++bone_index)
                    {
                        indices[bone_index] = bones[bone_index].bone_index;
                        weights[bone_index] = bones[bone_index].weight;
                        sum_weight += bones[bone_index].weight;
                    }

                    if (sum_weight == 0.0f)
                    {
                        vbo_data.m_skel_anim_weights.push_back(float(0.0f));
                        vbo_data.m_skel_anim_weights.push_back(float(0.0f));
                        vbo_data.m_skel_anim_weights.push_back(float(0.0f));
                        vbo_data.m_skel_anim_weights.push_back(float(0.0f));
                    }
                    else
                    {
                        vbo_data.m_skel_anim_weights.push_back(float(weights[0] / sum_weight));
                        vbo_data.m_skel_anim_weights.push_back(float(weights[1] / sum_weight));
                        vbo_data.m_skel_anim_weights.push_back(float(weights[2] / sum_weight));
                        vbo_data.m_skel_anim_weights.push_back(float(weights[3] / sum_weight));
                    }

                    vbo_data.m_skel_anim_indices.push_back(indices[0]);
                    vbo_data.m_skel_anim_indices.push_back(indices[1]);
                    vbo_data.m_skel_anim_indices.push_back(indices[2]);
                    vbo_data.m_skel_anim_indices.push_back(indices[3]);
                } // for (uint32_t vertex_index = 0; vertex_index < num_verts; ++vertex_index)

                // Store key as we need this for parsing in the physics code
                mat.mesh_keys.push_back(QPair<uint32_t, int>(nd->mMeshes[n], mesh_index));
            } // if (mesh_has_been_processed == false)

            // Add new transform for this mesh this is the object-space to instance-space transform for this instance
            vbo_data.m_instance_transforms.push_back(m);
            //qDebug() << "instance xform" << path << mesh->mName.C_Str() << m3 << m;

            // Increment nTris by the triangle count of this mesh if we are creating a new instance of it.
            // TODO: Perhaps this should store the number of triangles loaded rather than drawn, as that number will vary
            // when I implement working frustum-culling and/or other triangle culling techniques.
            data.nTris += vbo_data.m_indices.size() / 3;
        }
        //add the children to process list
        for (unsigned int n = 0; n < nd->mNumChildren; ++n)
        {
            if (nd->mChildren[n])
            {
                nodes_to_process.push_back(nd->mChildren[n]);
                nodes_parent_xforms.push_back(m);
            }
        }
    }

    nodes_to_process.clear();
    nodes_parent_xforms.clear();
    node_depth.clear();

    ready = true;
}

void Geom::BuildVBOsGL()
{
    QList <QString> materials = data.GetMaterialNames();
    for (int i = 0; i < materials.size(); ++i)
    {
        GeomMaterial & mat = data.GetMaterial(materials[i]);

        // For each mesh
        const int mesh_count = mat.vbo_data.size();
        for (int mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
        {
            GeomVBOData & vbo_data = data.GetVBOData(materials[i], mesh_index);
            Renderer::m_pimpl->CreateMeshHandleForGeomVBOData(vbo_data);
            vbo_data.ClearVertexData();
        }
    }
    built_vbos = true;
}

void Geom::get_bounding_box(aiVector3D* bmin, aiVector3D* bmax)
{
    //compute bounding box
    if (scene && scene->mRootNode) {
        aiMatrix4x4 trafo;
        aiIdentityMatrix4(&trafo);

        bmin->x = bmin->y = bmin->z = FLT_MAX;
        bmax->x = bmax->y = bmax->z = -FLT_MAX;
        get_bounding_box_for_node(scene->mRootNode, bmin, bmax, &trafo);
    }
}

void Geom::get_bounding_box_for_node(const struct aiNode* nd,
    aiVector3D* bmin,
    aiVector3D* bmax,
    aiMatrix4x4* trafo)
{
    aiMatrix4x4 prev;
    unsigned int n = 0, t;

    prev = *trafo;
    aiMultiplyMatrix4(trafo,&nd->mTransformation);

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {

            aiVector3D tmp = mesh->mVertices[t];
            aiTransformVecByMatrix4(&tmp,trafo);

            bmin->x = qMin(bmin->x,tmp.x);
            bmin->y = qMin(bmin->y,tmp.y);
            bmin->z = qMin(bmin->z,tmp.z);

            bmax->x = qMax(bmax->x,tmp.x);
            bmax->y = qMax(bmax->y,tmp.y);
            bmax->z = qMax(bmax->z,tmp.z);
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        if (nd->mChildren[n]) {
            get_bounding_box_for_node(nd->mChildren[n],bmin,bmax,trafo);
        }
    }
    *trafo = prev;
}

void Geom::SetupMaterialPath(const struct aiMaterial *mtl, GeomMaterial & mat, aiTextureType t, const int i)
{
//    qDebug() << "Geom::SetupMaterialPath mtlfilepath" << mtl_file_path;
    aiString texturePath;
    mtl->GetTexture(t, 0, &texturePath);
//    qDebug() << "Geom::SetupMaterialPath" << mtl_file_path << i << texturePath.C_Str();
    if (t != aiTextureType_NONE && mtl->GetTextureCount(t) > 0 && mtl->GetTexture(t, 0, &texturePath) == AI_SUCCESS)
    {        
        const QString s(texturePath.C_Str());
        mat.textures[i].filename_unresolved = s;

//        qDebug() << "Geom::SetupMaterialPath" << i << s;
        //58.0 - look for any external images in a place relative to the material path, if specified
        QUrl u;
        if (mtl_file_path.isEmpty()) {
            u = QUrl(path);
        }
        else {
            if (!QUrl(mtl_file_path).isRelative()) {
                u = QUrl(mtl_file_path);
            }
            else {
                u = QUrl(path).resolved(mtl_file_path);
            }
        }
        mat.textures[i].filename = u.resolved(s).toString();
//        qDebug() << "Geom::SetupMaterialPath" << mat.textures[i].filename;
    }
}

QColor Geom::GetColourFromArray(float * c)
{
    return QColor::fromRgbF(qMax(0.0f, qMin(c[0], 1.0f)),
                     qMax(0.0f, qMin(c[1], 1.0f)),
                     qMax(0.0f, qMin(c[2], 1.0f)),
                     qMax(0.0f, qMin(c[3], 1.0f)));
}

void Geom::create_material(const struct aiMaterial *mtl, GeomMaterial & mat)
{
    float c[4];

    int ret1, ret2;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    aiColor4D emission;
    ai_real shininess, strength, transparency;
    unsigned int max;

    set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
        color4_to_float4(&diffuse, c);
        mat.kd = GetColourFromArray(c);        
    }

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular)) {
        color4_to_float4(&specular, c);
        mat.ks = GetColourFromArray(c);
    }

    set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient)) {
        color4_to_float4(&ambient, c);
        mat.ka = GetColourFromArray(c);
    }

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission)) {
        color4_to_float4(&emission, c);
        mat.ke = GetColourFromArray(c);
    }

    max = 1;
    ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    if(ret1 == AI_SUCCESS) {
        max = 1;
        ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
        if(ret2 == AI_SUCCESS) {
            mat.ns = shininess * strength;
        }
        else {
            mat.ns = shininess;
        }
    }

    if(AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_OPACITY, &transparency)
            && transparency != 1)
    {
         mat.kd.setAlphaF(transparency);
    }

//    qDebug() << path << mtl << str.C_Str() << mat.kd << transparency; //mat.ka << mat.kd << mat.ks << mat.ke << mat.ns << transparency;

    SetupMaterialPath(mtl, mat, aiTextureType_DIFFUSE, 0);
    SetupMaterialPath(mtl, mat, aiTextureType_SPECULAR, 1);
    SetupMaterialPath(mtl, mat, aiTextureType_SHININESS, 2);
    SetupMaterialPath(mtl, mat, aiTextureType_OPACITY, 2);
    SetupMaterialPath(mtl, mat, aiTextureType_NORMALS, 3);
    SetupMaterialPath(mtl, mat, aiTextureType_HEIGHT, 3); //some normal maps are heightmaps for assimp (e.g. OBJ MTL's)
    SetupMaterialPath(mtl, mat, aiTextureType_DISPLACEMENT, 4);
    SetupMaterialPath(mtl, mat, aiTextureType_AMBIENT, 5); // map_ka is used as a lightmap for mtl
    SetupMaterialPath(mtl, mat, aiTextureType_LIGHTMAP, 8);
    SetupMaterialPath(mtl, mat, aiTextureType_EMISSIVE, 6);
//    qDebug() << base_path << mtl << mtl->GetTextureCount(aiTextureType_DIFFUSE);
}

void Geom::color4_to_float4(const aiColor4D *c, float f[4])
{
    f[0] = c->r;
    f[1] = c->g;
    f[2] = c->b;
    f[3] = c->a;
}

void Geom::set_float4(float f[4], float a, float b, float c, float d)
{
    f[0] = a;
    f[1] = b;
    f[2] = c;
    f[3] = d;
}

void Geom::SetAnimSpeed(const float f)
{
    anim_speed = f;
}

float Geom::GetAnimSpeed() const
{
    return anim_speed;
}


void Geom::SetLoop(const bool b)
{
    loop = b;
}

bool Geom::GetLoop() const
{
    return loop;
}

void Geom::CalcInterpolatedScaling(QVector3D & s, const double t, aiNodeAnim * a)
{
    s = QVector3D(1,1,1);
    if (a->mNumScalingKeys == 1) {
        aiVector3D s0 = a->mScalingKeys[0].mValue;
        s = QVector3D(s0.x, s0.y, s0.z);
    }
    else {
        for (unsigned int i=1; i<a->mNumScalingKeys; ++i) {

            double t0 = a->mScalingKeys[i-1].mTime;
            double t1 = a->mScalingKeys[i].mTime;

            if (t0 <= t && t <= t1) {
                aiVector3D s0 = a->mScalingKeys[i-1].mValue;
                aiVector3D s1 = a->mScalingKeys[i].mValue;

                double interp = (t - t0) / (t1 - t0);

                s = QVector3D(s0.x * (1.0 - interp) + s1.x * interp,
                              s0.y * (1.0 - interp) + s1.y * interp,
                              s0.z * (1.0 - interp) + s1.z * interp);
                return;
            }
        }
    }
}

void Geom::CalcInterpolatedRotation(aiQuaternion & r, const double t, aiNodeAnim * a)
{
    r = aiQuaternion(1,0,0,0);
    if (a->mNumRotationKeys == 1) {
        r = a->mRotationKeys[0].mValue;
    }
    else {
        for (unsigned int i=1; i<a->mNumRotationKeys; ++i) {

            double t0 = a->mRotationKeys[i-1].mTime;
            double t1 = a->mRotationKeys[i].mTime;

            if (t0 <= t && t <= t1) {
                aiQuaternion p0 = a->mRotationKeys[i-1].mValue;
                aiQuaternion p1 = a->mRotationKeys[i].mValue;

                double interp = (t - t0) / (t1 - t0);

                aiQuaternion::Interpolate(r, p0, p1, interp);
                return;
            }
        }
    }
}

void Geom::CalcInterpolatedPosition(QVector3D & p, const double t, aiNodeAnim * a)
{
    p = QVector3D(0,0,0);
    if (a->mNumPositionKeys == 1) {
       aiVector3D p0 = a->mPositionKeys[0].mValue;
       p = QVector3D(p0.x, p0.y, p0.z);
       return;
    }
    else {
        for (unsigned int i=1; i<a->mNumPositionKeys; ++i) {

            double t0 = a->mPositionKeys[i-1].mTime;
            double t1 = a->mPositionKeys[i].mTime;

            if (t0 <= t && t <= t1) {
                aiVector3D p0 = a->mPositionKeys[i-1].mValue;
                aiVector3D p1 = a->mPositionKeys[i].mValue;

                double interp = (t - t0) / (t1 - t0);

                p = QVector3D(p0.x * (1.0 - interp) + p1.x * interp,
                              p0.y * (1.0 - interp) + p1.y * interp,
                              p0.z * (1.0 - interp) + p1.z * interp);
                return;
            }
        }
    }
}

void Geom::DoLocalTransformation(aiNodeAnim * a, QMatrix4x4 & mat, bool translate)
{
    aiQuaternion q;
    QVector3D p, s;

    if (translate) {
        CalcInterpolatedPosition(p, cur_time, a);
        mat.translate(p.x(), p.y(), p.z());
    }

    CalcInterpolatedRotation(q, cur_time, a);
    mat.rotate(QQuaternion(q.w, q.x, q.y, q.z));

    CalcInterpolatedScaling(s, cur_time, a);
    mat.scale(s);
}

QMatrix4x4 Geom::aiToQMatrix4x4(const aiMatrix3x3 & m2)
{
    QMatrix4x4 m;
    m.setRow(0, QVector4D(m2.a1, m2.a2, m2.a3, 0));
    m.setRow(1, QVector4D(m2.b1, m2.b2, m2.b3, 0));
    m.setRow(2, QVector4D(m2.c1, m2.c2, m2.c3, 0));
    m.setRow(3, QVector4D(0, 0, 0, 1));
    return m;
}

QMatrix4x4 Geom::aiToQMatrix4x4(const aiMatrix4x4 & m2)
{
    QMatrix4x4 m((float *)&m2);
    return m;
}

void Geom::CalculateFinalPoses()
{
//    qDebug() << bone_offset_matrix;
    skin_joints.clear();
    skin_joints.resize(ASSETSHADER_MAX_JOINTS);

    //59.0 - bugfix prevent crash updating mesh if it doesn't exist/could not be loaded
    if (scene == NULL)
    {
        return;
    }

    QPointer <Geom> geom = (linked_anim ? linked_anim : QPointer<Geom>(this));
    if (geom->scene == NULL) {        
        return;
    }

    QMatrix4x4 git = m_globalInverseTransform;
    if (geom != this) {
        git = git * geom->m_globalInverseTransform;
    }

    /* draw all meshes assigned to this node */
    //iterate through everything
    nodes_to_process.clear();
    nodes_to_process.reserve(1024);
    nodes_parent_xforms.clear();
    nodes_parent_xforms.reserve(1024);

    //iterate over nodes of BASE OBJECT
    if (scene->mRootNode)
    {
        nodes_to_process.push_back(scene->mRootNode);
        nodes_parent_xforms.push_back(QMatrix4x4());
    }

    //on first pass, set up node hierarchy and node indexes for whole scene
    while (nodes_to_process.size() != 0)
    {
        aiNode * nd = nodes_to_process.back();
        nodes_to_process.pop_back();
        const QString node_name = GetProcessedNodeName(nd->mName.C_Str());

        QMatrix4x4 parentTransform = nodes_parent_xforms.back();
        nodes_parent_xforms.pop_back();

        QMatrix4x4 globalTransform;
        QMatrix4x4 nodeTransform; // = aiToQMatrix4x4(nd->mTransformation);

        if (bone_to_boneid.contains(node_name)) { //its a bone

            const unsigned int bone_id = bone_to_boneid[node_name];
//            if we can't find the animation, just multiply by the base transform
//            anim associated with the bone can be either (call bone name X):
//              X
//              X_$assimpfbx$_Y

//             where y is a specific ordered set of transformations as defined by FBX
//            Local Transform = Translation * RotationOffset * RotationPivot * PreRotation
//                               * Rotation * PostRotation * RotationPivotInverse * ScalingOffset
//                               * ScalingPivot * Scaling * ScalingPivotInverse
//             where multiply is right to left.


#ifdef __linux__
            const bool apply_translate = true;
#else
            const bool apply_translate = false;
#endif

            bool xform_anim = false;
            if (geom->anims.contains(node_name+"_$assimpfbx$_translation"))
            {
                DoLocalTransformation(geom->anims[node_name+"_$assimpfbx$_translation"], nodeTransform, apply_translate);
                xform_anim = true;
            }
            if (geom->anims.contains(node_name+"_$assimpfbx$_prerotation"))
            {
                DoLocalTransformation(geom->anims[node_name+"_$assimpfbx$_prerotation"], nodeTransform, apply_translate);
                xform_anim = true;
            }
            if (geom->anims.contains(node_name+"_$assimpfbx$_rotation"))
            {
                DoLocalTransformation(geom->anims[node_name+"_$assimpfbx$_rotation"], nodeTransform, apply_translate);
                xform_anim = true;
            }
            if (geom->anims.contains(node_name))
            {               
                DoLocalTransformation(geom->anims[node_name], nodeTransform, apply_translate);
                xform_anim = true;
            }

            if (!xform_anim) {
                nodeTransform *= aiToQMatrix4x4(nd->mTransformation);
            }

            globalTransform = parentTransform * nodeTransform;           

            if (extra_global_transforms.contains(bone_id))
            {                
                globalTransform = extra_global_transforms[bone_id];
            }
            else if (extra_global_rotation_transforms.contains(bone_id))
            {
                //overwrite orientation only
                globalTransform.setColumn(0, extra_global_rotation_transforms[bone_id].column(0));
                globalTransform.setColumn(1, extra_global_rotation_transforms[bone_id].column(1));
                globalTransform.setColumn(2, extra_global_rotation_transforms[bone_id].column(2));
            }

            if (extra_relative_transforms.contains(bone_id))
            {
                globalTransform *= extra_relative_transforms[bone_id];
            }

            QMatrix4x4 m = git * globalTransform;

            QVector4D c = m.column(3);
            c.setW(1.0f);
            m.setColumn(3, c);

            final_poses[bone_to_boneid[node_name]] = m;
            skin_joints[bone_to_boneid[node_name]] = m * bone_offset_matrix[node_name];
        }
        else {
            globalTransform = parentTransform * aiToQMatrix4x4(nd->mTransformation);
        }

        //add the children to process list
        for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
            if (nd->mChildren[n])  {
//                qDebug() << node_name << nd->mChildren[n]->mName.C_Str();
                nodes_to_process.push_back(nd->mChildren[n]);
                nodes_parent_xforms.push_back(globalTransform);
            }
        }
    }
}

bool Geom::HasBone(const QString & bone_name)
{
    return bone_to_boneid.contains(bone_name);
}

void Geom::SetRelativeTransform(const QString & bone_name, const QMatrix4x4 & mat)
{
    if (bone_to_boneid.contains(bone_name)) {
        extra_relative_transforms[bone_to_boneid[bone_name]] = mat;
    }
}

QMatrix4x4 Geom::GetRelativeTransform(const QString & bone_name) const
{
    if (bone_to_boneid.contains(bone_name)) {
        return extra_relative_transforms[bone_to_boneid[bone_name]];
    }
    return QMatrix4x4();
}

void Geom::SetGlobalRotationTransform(const QString & bone_name, const QMatrix4x4 & mat)
{
    if (bone_to_boneid.contains(bone_name)) {
        extra_global_rotation_transforms[bone_to_boneid[bone_name]] = mat;
    }
}

QMatrix4x4 Geom::GetGlobalRotationTransform(const QString & bone_name) const
{
    if (bone_to_boneid.contains(bone_name)) {
        return extra_global_rotation_transforms[bone_to_boneid[bone_name]];
    }
    return QMatrix4x4();
}

void Geom::SetGlobalTransform(const QString & bone_name, const QMatrix4x4 & mat)
{
    if (bone_to_boneid.contains(bone_name)) {
        extra_global_transforms[bone_to_boneid[bone_name]] = mat;
    }
}

QMatrix4x4 Geom::GetGlobalTransform(const QString & bone_name) const
{
    if (bone_to_boneid.contains(bone_name)) {
        return extra_global_transforms[bone_to_boneid[bone_name]];
    }
    return QMatrix4x4();
}

void Geom::ClearTransforms()
{
    //    extra_relative_transforms.clear();
    extra_global_transforms.clear();
    extra_global_rotation_transforms.clear();
}

bool Geom::GetError() const
{
    return error;
}
