#ifndef GEOM_H
#define GEOM_H

#include <QtOpenGL>

//Assimp includes (use the C++ interface)
#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/types.h>

#include "assetshader.h"
#include "webasset.h"
#include "assetimage.h"

struct GeomTexture
{
    GeomTexture()
    {
    }

    bool operator==(const GeomTexture & rhs) const {
        return img == rhs.img &&
                filename == rhs.filename &&
                ba == rhs.ba;
    }

    QPointer <AssetImage> img;
    QString filename;
    QString filename_unresolved;
    QByteArray ba;
};

struct GeomTriangle
{
    GeomTriangle()
    {
    }
    half_float::half p[3][3];
    half_float::half n[3][3];
    half_float::half t[3][2];
};

struct GeomMaterial
{
    GeomMaterial() :
        ka(0,0,0),
        ke(0,0,0),
        kd(255,255,255),
        ks(10,10,10), // 10/255 is equivalent to ~4% which is the default f0 value       
        ns(20)
    {        
        textures = QVector<GeomTexture>(ASSETSHADER_NUM_TEXTURES);
    }

    bool operator==(const GeomMaterial & rhs) const {
        for (int i=0; i<textures.size(); ++i) {
            if (!(textures[i] == rhs.textures[i])) {
                return false;
            }
        }
        return ka == rhs.ka &&
                ke == rhs.ke &&
                kd == rhs.kd &&
                ks == rhs.ks &&                
                ns == rhs.ns;
    }

    QColor ka;
    QColor ke;
    QColor kd;
    QColor ks;
    float ns;

    QVector<GeomTexture> textures;
    // Assimp Mesh ID is the lookup here
    // This lets us just store the transforms for instanes
    // rather than transforming its verts into object-space and
    // appending to the one mesh.
    QVector <QVector<GeomTriangle>> triangles;
    QVector <GeomVBOData> vbo_data;
    QVector <QPair<uint32_t, int>> mesh_keys;
};

class GeomData
{
public:

    GeomData();

    void AddMaterial(const QString mat);
    void AddTriangle(const QString mat, const uint32_t mesh_UUID, const GeomTriangle t);

    void SetTextureFilename(const QString mat, const unsigned int channel, const QString filename);

    void Clear();

    unsigned int GetNumMaterials() const;
    unsigned int GetNumTris() const;
    QList <QString> GetMaterialNames() const;
    GeomMaterial & GetMaterial(const QString mat);
    QString GetMaterialMatchName(const GeomMaterial & mat);
    QVector<GeomTriangle> & GetTriangles(const QString mat, uint32_t const mesh_UUID);
    GeomVBOData & GetVBOData(const QString mat, const uint32_t mesh_UUID);
    unsigned int nTris;

private:

    QHash <QString, GeomMaterial> materials;
};

class GeomIOStream : public QObject, public Assimp::IOStream
{
    Q_OBJECT
public:

    GeomIOStream();
    ~GeomIOStream();

    void SetData(const QByteArray & b);
    void SetUrl(QUrl u);
    QUrl GetUrl() const;

    void SetWebAsset(QPointer <WebAsset> w);
    QPointer <WebAsset> GetWebAsset();

    size_t 	FileSize () const;
    void 	Flush ();
    size_t 	Read (void *pvBuffer, size_t pSize, size_t pCount);
    aiReturn 	Seek (size_t pOffset, aiOrigin pOrigin);
    size_t 	Tell () const;
    size_t 	Write (const void *pvBuffer, size_t pSize, size_t pCount);
    void Close();

private:

    QByteArray ba;
    QBuffer * buffer;
    QUrl url;
    QPointer <WebAsset> webasset;

};

class GeomIOSystem : public Assimp::IOSystem
{
public:
    GeomIOSystem();
    ~GeomIOSystem();

    void SetGZipped(const bool b);
    void SetFakeExtensionAdded(const bool b, const QString s);
    void SetBasePath(QUrl u);
    void 	Close (Assimp::IOStream *pFile);
    bool 	ComparePaths (const char *one, const char *second) const;
    bool 	ComparePaths (const std::string &one, const std::string &second) const;
    bool 	Exists (const std::string &pFile) const;
    bool 	Exists (const char *pFile) const;
    char 	getOsSeparator () const;
    Assimp::IOStream * 	Open (const char *pFile, const char *pMode);
    Assimp::IOStream * 	Open (const std::string &pFile, const std::string &pMode);

    bool PushDirectory( const std::string &path );
    bool PopDirectory();

    float UpdateRequests();
    void Clear();

    void SetMTLFilePath(const QString & s);

    static void SetShuttingDown(const bool b);

private:

    Assimp::IOStream * Open(const char *pFile);

    bool gzipped;
    QString fake_extension_string;
    bool fake_extension_added;
    QUrl base_path;
    QList <QPointer <GeomIOStream> > streams;

    QHash <QString, QByteArray> data_cache;
    QString mtl_file_path;

    static bool shutting_down;

};

class Geom : public QObject
{
public:
    Geom();
    ~Geom();

    void SetPath(const QString & p);

    void SetMesh(const QVariantMap & property_list);
    bool GetHasMeshData() const;

    void Load();
    void Unload();

    void Update();
    bool UpdateGL();
    void DrawGL(QPointer <AssetShader> shader, const QColor col = QColor(255,255,255));

    void SetReady(const bool b);
    bool GetReady() const;
    bool GetStarted() const;
    float GetProgress() const;
    bool GetTexturesReady() const;
    float GetTextureProgress() const;

    QVector3D GetBBoxMin() const;
    QVector3D GetBBoxMax() const;

    unsigned int GetNumTris() const;

    GeomData & GetData();

    void SetTextureClamp(const bool b);
    bool GetTextureClamp() const;

    void SetTextureLinear(const bool b);
    bool GetTextureLinear() const;

    void SetTextureCompress(const bool b);
    bool GetTextureCompress() const;

    void SetTextureMipmap(const bool b);
    bool GetTextureMipmap() const;

    void SetCenter(const bool b);
    bool GetCenter() const;

    void SetUsesTexFile(const bool b);
    bool GetUsesTexFile() const;

    void SetAnimSpeed(const float f);
    float GetAnimSpeed() const;

    void SetLoop(const bool b);
    bool GetLoop() const;

    void SetMTLFile(const QString & s);
    void SetMaterialTexture(const QString & tex_url, const int channel);

    //animation related
    void SetLinkAnimation(QPointer <Geom> anim);
    QPointer <Geom> GetLinkAnimation();
    void UpdateAnimation();

    QMatrix4x4 GetFinalPose(const QString &bone_id);

    //animation extra transforms
    bool HasBone(const QString & bone_name);
    void SetRelativeTransform(const QString & bone_name, const QMatrix4x4 & mat);
    QMatrix4x4 GetRelativeTransform(const QString & bone_name) const;

    void SetGlobalTransform(const QString & bone_name, const QMatrix4x4 & mat);
    QMatrix4x4 GetGlobalTransform(const QString & bone_name) const;

    void SetGlobalRotationTransform(const QString & bone_name, const QMatrix4x4 & mat);
    QMatrix4x4 GetGlobalRotationTransform(const QString & bone_name) const;

    void ClearTransforms();

    QString GetTextureAlphaType() const;
    void SetTextureAlphaType(const QString & alpha_type);
    bool GetTexturePreMultiply() const;
    void SetTexturePreMultiply(const bool b);
    void SetTextureColorSpace(const QString & color_space);
    QString GetTextureColorSpace() const;

    bool GetError() const;

protected:

    QString path;
    QByteArray mesh_data;

    bool center;

    bool ready;
    bool started;
    bool error;
    QString error_str;
    bool textures_started;
    bool textures_ready;
    float texture_progress;
    float progress;
    bool built_vbos;

    QVector3D bbox_min;
    QVector3D bbox_max;

    bool tex_clamp;
    bool tex_linear;
    bool tex_compress;
    bool tex_mipmap;
    bool tex_premultiply;
    QString tex_alpha;
    QString tex_colorspace;

    GeomData data; //replace with intermediate format

private:

    void CalculateFinalPoses();
    void DoLocalTransformation(aiNodeAnim * a, QMatrix4x4 & mat, bool translate=false);
    void CalcInterpolatedPosition(QVector3D & p, const double t, aiNodeAnim * a);
    void CalcInterpolatedRotation(aiQuaternion & r, const double t, aiNodeAnim * a);
    void CalcInterpolatedScaling(QVector3D & s, const double t, aiNodeAnim * a);

    void PrepareVBOs();
    void BuildVBOsGL();

    void get_bounding_box(aiVector3D* bmin, aiVector3D* bmax);
    void get_bounding_box_for_node(const aiNode* nd, aiVector3D* bmin, aiVector3D* bmax, aiMatrix4x4* trafo);
    void create_material(const aiMaterial *mtl, GeomMaterial & mat);
    void SetupMaterialPath(const struct aiMaterial *mtl, GeomMaterial & mat, aiTextureType t, const int i);

    void color4_to_float4(const aiColor4D *c, float f[4]);
    void set_float4(float f[4], float a, float b, float c, float d);

    QColor GetColourFromArray(float * c);    

    QMatrix4x4 aiToQMatrix4x4(const aiMatrix3x3 & m2);
    QMatrix4x4 aiToQMatrix4x4(const aiMatrix4x4 & m2);

    QString GetProcessedNodeName(const QString s);

    /* the global Assimp scene object */
    Assimp::Importer importer;
    GeomIOSystem * iosystem;
    aiScene * scene;

    QString mtl_file_path;

    QHash <QString, int> bone_to_node; //maps bone names to node indexes
    QHash <QString, int> bone_to_boneid; //maps bone names to bone indexes (skips some nodes, since there can only be up to 128 bones)
    QHash <QString, aiNodeAnim *> anims;
    QVector <aiNode *> node_list; //maps indexes to aiNodes
    bool uses_tex_file;

    QMatrix4x4 m_globalInverseTransform;

    QPointer <Geom> linked_anim;
    QTime time;
    double cur_time;
    float anim_speed;
    bool loop;
    QVector<QMatrix4x4> final_poses;
    QVector<QMatrix4x4> skin_joints;
    QHash <QString, QMatrix4x4> bone_offset_matrix;
    QHash <int, QMatrix4x4> extra_global_transforms; //bone indexes to xform
    QHash <int, QMatrix4x4> extra_global_rotation_transforms; //bone indexes to xform
    QHash <int, QMatrix4x4> extra_relative_transforms; //bone indexes to xform
    QVector <aiNode *> nodes_to_process;
    QVector <QMatrix4x4> nodes_parent_xforms;
};

#endif // GEOM_H
