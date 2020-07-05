#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <QtGui>

#include "assetobject.h"
#include "domnode.h"
#include "mathutil.h"

class AssetObject;

struct Particle
{
    Particle():
        col(255,255,255,0),
        scale(1,1,1),
        active(0),
        lifetime_subtract_msec(0)
    {
    }

    QVector3D pos;
    QVector3D vel;
    QVector3D accel;
    QColor col;
    QVector3D scale;
    int active;
    QTime lifetime;
    int lifetime_subtract_msec;
};

class ParticleSystem : public QObject
{

public:

    ParticleSystem();
    ~ParticleSystem();

    void SetEmitterMesh(QPointer <AssetObject> a);
    QPointer <AssetObject> GetEmitterMesh();

    void Update(QPointer <DOMNode> props, const double dt_sec);
    void DrawGL(QPointer <AssetShader> shader, const QVector3D eye_pos, QPointer <AssetObject> obj);

private:

    void CreateVBO();

    QVector <Particle> particles;

    QTime p_time;
    float p_time_elapsed;
    int p_emitted;
    //int p_to_draw;

    QPointer <AssetObject> emitter_mesh;

    QPointer<MeshHandle> m_mesh_handle;
    QPointer<BufferHandle> m_VBO_positions;
    QPointer<BufferHandle> m_VBO_tex_coords0;
    QPointer<BufferHandle> m_VBO_colors;
    QPointer<BufferHandle> m_VBO_indices;

    QVector<float> m_positions;
    QVector<uint8_t> m_tex_coords;
    QVector<float> m_colors;
    QVector<uint32_t> m_indices;

    QVector<QVector3D> v;

    int stride;

    QVector3D eye_pos;

	uint16_t FloatToFloat16(float value)
	{
		uint16_t   fltInt16;
		int     fltInt32;
		memcpy(&fltInt32, &value, sizeof(float));
		fltInt16 = ((fltInt32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
		fltInt16 |= ((fltInt32 & 0x80000000) >> 16);

		return fltInt16;
	}

};

#endif // PARTICLESYSTEM_H
