#ifndef TEXTGEOM_H
#define TEXTGEOM_H

#include <QtGui>

#include "mathutil.h"
#include "assetshader.h"
#include "assetimage.h"

struct TextGeomLine
{
    TextGeomLine()
    {
    }

    QString text;    
    QColor col;
    QVector<float> m_positions;
    QVector<float> m_texcoords;
    QVector<uint32_t> m_indices;
    QPointer<MeshHandle> m_mesh_handle;
    QPointer<BufferHandle> m_position_handle;
    QPointer<BufferHandle> m_texcoord_handle;
    QPointer<BufferHandle> m_index_handle;
};

class TextGeom : public QObject
{
public:

    TextGeom();
    ~TextGeom();

    static void initializeGL();

    void SetMaxSize(float x, float y);
    float GetMaxSizeX() const;
    float GetMaxSizeY() const;

    void AddText(const QString & s, const QColor col = QColor(255,255,255));
    void SetText(const QString & s, const QColor col = QColor(255,255,255));
    QString GetText() const;
    float GetTextLength() const;   

    QVector <TextGeomLine> GetAllText() const;

    void SetColour(const QColor & c);
    QColor GetColour() const;

    void SetFixedSize(const bool b, const float f);
    float GetFixedSize() const;

    void SetLeftJustify(const bool b);
    bool GetLeftJustify() const;

    float GetScale() const;
    float GetHeight() const;

    void DrawGL(QPointer <AssetShader> shader);
    void DrawSelectedGL(QPointer <AssetShader> shader);

    QMatrix4x4 GetModelMatrix() const;

    void Clear();   

private:

    void CreateVBO();    

    QVector <TextGeomLine> texts;    

    bool do_fixed_size;
    float fixed_size;

    float maxx;
    float maxy;
    float len;
    float height;    

    QVector3D pos;
    QVector3D dir;

    bool vbo_rebuild;
	bool left_justify;
};

#endif // TEXTGEOM_H
