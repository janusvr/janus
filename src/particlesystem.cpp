#include "particlesystem.h"

ParticleSystem::ParticleSystem() :
    stride(9)
{
    v.resize(4);
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::SetEmitterMesh(QPointer <AssetObject> a)
{
    emitter_mesh = a;
}

QPointer <AssetObject> ParticleSystem::GetEmitterMesh()
{
    return emitter_mesh;
}

void ParticleSystem::Update(QPointer <DOMNode> props, const double dt_sec)
{    
    if (props.isNull()) {
        return;
    }

    const int count = props->GetCount();
    if (particles.size() < count)
	{
        particles = QVector<Particle>(count);

        p_time.start();
        p_emitted = 0;

        CreateVBO();
    }

    const int msec_passed = p_time.elapsed();
    const int target_emitted = int(double(props->GetRate()) * double(msec_passed) * 0.001);
    const int start_emitted = p_emitted;
    const int msec_lifetime = props->GetDuration() * 1000;
    const bool loop = props->GetLoop();

    //56.0 - if emitting via a mesh, it must be ready
    if (emitter_mesh && emitter_mesh->GetGeom() && !emitter_mesh->GetGeom()->GetReady())
	{
        return;
    }

	// Particle update Loop
	uint32_t const particle_count = particles.size();
    uint32_t const verts_per_particle = 4;
	uint32_t const vert_count = verts_per_particle * particle_count;

    uint32_t const elements_per_position = 4;
    m_positions.resize(vert_count * elements_per_position);	// xyz + 1 padding
	uint32_t const elements_per_color = 4;
    m_colors.resize(vert_count * elements_per_color); // rgba

    const float fadein = props->GetFadeIn();
    const float fadeout = props->GetFadeOut();
    const bool emit_local = props->GetEmitLocal();

	for (uint32_t particle_index = 0; particle_index < particle_count; ++particle_index)
    {
		const int i = particle_index;
        Particle & p = particles[i];

        //generate particle logic
        if (p.active == 0)
        {
            if (p_emitted < target_emitted)
            {
                //qDebug() << "CREATING PARTICLE" << p_emitted << p_target_emitted << "rate" << props->GetRate() << "count" << props->GetCount();
                //qDebug() << "vel and randvel" << props->GetVel()->toQVector3D() << props->GetRandVel()->toQVector3D();

                if (emitter_mesh && emitter_mesh->GetGeom() && emitter_mesh->GetGeom()->GetData().GetNumMaterials() > 0)
                {
                    GeomData & data = emitter_mesh->GetGeom()->GetData();

                    QList <QString> mat_names = data.GetMaterialNames();
                    int rand_mat = qrand() % mat_names.size();

                    QVector <GeomTriangle> & tris = data.GetTriangles(mat_names[rand_mat], 0);
                    int rand_tri = qrand() % tris.size();

                    int rand_tri_vert = qrand() % 3;

                    p.pos = QVector3D(tris[rand_tri].p[rand_tri_vert][0],
                                      tris[rand_tri].p[rand_tri_vert][1],
                                      tris[rand_tri].p[rand_tri_vert][2]);
                }
                else
                {
                    if (emit_local) {
                        p.pos = MathUtil::GetRandomValue(props->GetRandPos()->toQVector3D());
                    }
                    else {
                        p.pos = props->GetPos()->toQVector3D() + MathUtil::GetRandomValue(props->GetRandPos()->toQVector3D());
                    }
                }

                p.vel = props->GetVel()->toQVector3D() + MathUtil::GetRandomValue(props->GetRandVel()->toQVector3D());
                p.accel = props->GetAccel()->toQVector3D() + MathUtil::GetRandomValue(props->GetRandAccel()->toQVector3D());
                const QColor c = MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D());
                const QColor c2 = MathUtil::GetVector4AsColour(props->GetRandColour()->toQVector4D());

                const QVector3D cv = QVector3D(c.redF(), c.greenF(), c.blueF()) + MathUtil::GetRandomValue(QVector3D(c2.redF(), c2.greenF(), c2.blueF()));

                p.col.setRgbF(qMin(qMax(0.0f, cv.x()), 1.0f),
                              qMin(qMax(0.0f, cv.y()), 1.0f),
                              qMin(qMax(0.0f, cv.z()), 1.0f));
                p.col.setAlphaF(0.0f);
                p.scale = props->GetScale()->toQVector3D() + MathUtil::GetRandomValue(props->GetRandScale()->toQVector3D());
                p.active = 1;
                p.lifetime.start();
                if (target_emitted <= start_emitted) {
                    p.lifetime_subtract_msec = 0;
                }
                else {
                    //interpolate from start_emitted to target_emitted
                    const float interp = (p_emitted - start_emitted) / (target_emitted-start_emitted);
                    p.lifetime_subtract_msec = interp * dt_sec * 1000.0;
                }
                ++p_emitted;
            }
        }

        //update particle logic
        if (p.active == 1)
		{
            p.vel += p.accel * dt_sec;
            p.pos += p.vel * dt_sec;

            //Set inactive (so we can reallocate)
            const int p_lifetime = p.lifetime.elapsed();
            if (p_lifetime-p.lifetime_subtract_msec > msec_lifetime)
			{
                p.active = (loop ? 0 : 2);

                uint32_t const vertex_base_index = particle_index * verts_per_particle;
				for (uint32_t vertex_index = 0; vertex_index < verts_per_particle; ++vertex_index)
				{
					uint32_t const vertex_final_index = vertex_base_index + vertex_index;

					uint32_t position_offset = vertex_final_index * elements_per_position;

                    m_positions[position_offset + 0] = float(0.0f);
                    m_positions[position_offset + 1] = float(0.0f);
                    m_positions[position_offset + 2] = float(0.0f);
                    m_positions[position_offset + 3] = float(1.0f);
                }
            }

            const float fade_in_alpha = float(p_lifetime) / float(fadein * 1000.0f);
            const float fade_out_alpha = float(msec_lifetime - p_lifetime) / float(fadeout * 1000.0f);
            p.col.setAlphaF(qMax(0.0f, qMin(1.0f, qMin(fade_in_alpha, fade_out_alpha))));

            const QVector3D & pos = particles[i].pos;
            const float s = particles[i].scale.x();

            const QVector3D z = (eye_pos - pos).normalized();
            const QVector3D x = QVector3D::crossProduct(QVector3D(0,1,0), z) * s;
            const QVector3D y = QVector3D::crossProduct(z, x).normalized() * s;

            v[0] = pos - x + y;
            v[1] = pos - x - y;
            v[2] = pos + x - y;
            v[3] = pos + x + y;

            uint32_t const vertex_base_index = particle_index * verts_per_particle;
            float const particle_alpha = p.col.alphaF();
            float const premultipled_red = p.col.redF() * particle_alpha;
            float const premultipled_green = p.col.greenF() * particle_alpha;
            float const premultipled_blue = p.col.blueF() * particle_alpha;
            float const premultipled_alpha = particle_alpha;
			for (uint32_t vertex_index = 0; vertex_index < verts_per_particle; ++vertex_index)
			{
				uint32_t const vertex_final_index = vertex_base_index + vertex_index;

                uint32_t position_offset = vertex_final_index * elements_per_position;
                m_positions[position_offset + 0] = float(v[vertex_index].x());
                m_positions[position_offset + 1] = float(v[vertex_index].y());
                m_positions[position_offset + 2] = float(v[vertex_index].z());
                m_positions[position_offset + 3] = float(1.0f);

                uint32_t const color_offset = vertex_final_index * elements_per_color;
                m_colors[color_offset + 0] = float(premultipled_red);
                m_colors[color_offset + 1] = float(premultipled_green);
                m_colors[color_offset + 2] = float(premultipled_blue);
                m_colors[color_offset + 3] = float(premultipled_alpha);
			}
        }

    }

    if (msec_passed >= 1000) {
        //qDebug() << "p_emitted" << p_emitted;
        p_time.restart();
        p_emitted = 0;
    }

    //update VBO contents
    if (count != 0) {
        Renderer::m_pimpl->UpdateBufferHandleData(m_VBO_positions, 0, static_cast<uint32_t>(m_positions.size() * sizeof(float)), m_positions.data());
        Renderer::m_pimpl->UpdateBufferHandleData(m_VBO_colors, 0, static_cast<uint32_t>(m_colors.size() * sizeof(float)), m_colors.data());
    }
}

void ParticleSystem::DrawGL(QPointer <AssetShader> shader, const QVector3D eye_pos, QPointer <AssetObject> obj)
{
    //qDebug() << "ParticleSystem::DrawGL" << vbo_id;
    // draw
    this->eye_pos = eye_pos;   

    if (obj && obj->GetFinished())
    {

        //slow matrix push per object method (instancing could improve this a lot)        
        const int count = particles.size();
        for (int j=0; j<count; ++j)
        {
            //const int i = ((j + p_to_draw) % count);
            const int i = j;
            const Particle & p = particles[i];

            if (p.active == 1)
            {
                QMatrix4x4 mat;
                mat.translate(p.pos);
                mat.scale(p.scale);

                MathUtil::PushModelMatrix();
                MathUtil::MultModelMatrix(mat);                
                obj->DrawGL(shader, p.col);
                MathUtil::PopModelMatrix();
            }
        }
    }
    else
    {
        //fast VBO-based method
        uint32_t const indice_count = static_cast<uint32_t>(m_indices.size());
        if (m_mesh_handle != nullptr && indice_count != 0)
        {
            //shader->SetUseParticleColours(true); //55.10
            shader->UpdateObjectUniforms();

            Renderer * renderer = Renderer::m_pimpl;

            AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                            indice_count,
                            0,
                            0,
                            0,
                            m_mesh_handle,
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
            renderer->PushAbstractRenderCommand(a);
        }
    }
}

void ParticleSystem::CreateVBO()
{
    //qDebug() << "ParticleSystem::CreateVBO()";
	uint32_t const particle_count = particles.size();
    uint32_t const verts_per_particle = 4;
    uint32_t const indices_per_particle = 6;
	uint32_t const vert_count = verts_per_particle * particle_count;
    uint32_t const index_count = indices_per_particle * particle_count;

    uint32_t const elements_per_position = 4;
    m_positions.resize(vert_count * elements_per_position);	// xyz +1 padding
	uint32_t const elements_per_color = 4;
	m_colors.resize(vert_count * elements_per_color);	// rgba
    uint32_t const elements_per_tex_coord = 4;
    m_tex_coords.resize(vert_count * elements_per_tex_coord); //st + 2 padding
    uint32_t const elements_per_index = 1;
    m_indices.resize(index_count * elements_per_index); //i

    // For each particle
    for (uint32_t particle_index = 0; particle_index < particle_count; ++particle_index)
	{
        // Initialize it's 6 indices
        uint32_t const index_base_index = particle_index * verts_per_particle;
        for (uint32_t index_index = 0; index_index < indices_per_particle; ++index_index)
        {
            uint32_t const index_final_index = particle_index * indices_per_particle + index_index;
            switch (index_index)
            {
            case 0:
                m_indices[index_final_index] = 0 + index_base_index;
                break;
            case 1:
                m_indices[index_final_index] = 1 + index_base_index;
                break;
            case 2:
                m_indices[index_final_index] = 2 + index_base_index;
                break;
            case 3:
                m_indices[index_final_index] = 0 + index_base_index;
                break;
            case 4:
                m_indices[index_final_index] = 2 + index_base_index;
                break;
            case 5:
                m_indices[index_final_index] = 3 + index_base_index;
                break;
            default:
                break;
            }
        }

        // Initialize it's 4 vertices
        uint32_t const vertex_base_index = particle_index * verts_per_particle;
        for (uint32_t vertex_index = 0; vertex_index < verts_per_particle; ++vertex_index)
        {
            uint32_t const vertex_final_index = vertex_base_index + vertex_index;

            uint32_t position_offset = vertex_final_index * elements_per_position;
            m_positions[position_offset + 0] = float(0.0f);
            m_positions[position_offset + 1] = float(0.0f);
            m_positions[position_offset + 2] = float(0.0f);
            m_positions[position_offset + 3] = float(1.0f);

            uint32_t const color_offset = vertex_final_index * elements_per_color;
            m_colors[color_offset + 0] = float(1.0f);
            m_colors[color_offset + 1] = float(1.0f);
            m_colors[color_offset + 2] = float(1.0f);
            m_colors[color_offset + 3] = float(1.0f);

            uint32_t const tex_coord_offset = vertex_final_index * elements_per_tex_coord;
            switch (vertex_index)
            {
            case 0:
                m_tex_coords[tex_coord_offset + 0] = 0;
                m_tex_coords[tex_coord_offset + 1] = 255;
                break;
            case 1:
                m_tex_coords[tex_coord_offset + 0] = 0;
                m_tex_coords[tex_coord_offset + 1] = 0;
                break;
            case 2:
                m_tex_coords[tex_coord_offset + 0] = 255;
                m_tex_coords[tex_coord_offset + 1] = 0;
                break;
            case 3:
                m_tex_coords[tex_coord_offset + 0] = 255;
                m_tex_coords[tex_coord_offset + 1] = 255;
                break;
            default:
                break;
            }
            m_tex_coords[tex_coord_offset + 2] = 0;
            m_tex_coords[tex_coord_offset + 3] = 0;
        }
    }

    int32_t float_type = GL_FLOAT;
    int32_t float_size = sizeof(float);

    VertexAttributeLayout layout;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_count = 3;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].stride_in_bytes = elements_per_position * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_count = 2;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_type = GL_UNSIGNED_BYTE;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].stride_in_bytes = elements_per_tex_coord * sizeof(uint8_t);
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].element_count = elements_per_color;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].stride_in_bytes = elements_per_color * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_count = elements_per_index;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_type = GL_UNSIGNED_INT;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].stride_in_bytes = elements_per_index * sizeof(uint32_t);
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].offset_in_bytes = 0;

    m_mesh_handle = Renderer::m_pimpl->CreateMeshHandle(layout);    

    QVector<QPointer<BufferHandle>> buffers = Renderer::m_pimpl->GetBufferHandlesForMeshHandle(m_mesh_handle);

    if (buffers.size() >= VAO_ATTRIB::NUM_ATTRIBS) {
        m_VBO_positions = buffers[(GLuint)VAO_ATTRIB::POSITION];
        if (m_VBO_positions) {
            Renderer::m_pimpl->BindBufferHandle(m_VBO_positions);
            Renderer::m_pimpl->ConfigureBufferHandleData(m_VBO_positions, static_cast<uint32_t>(m_positions.size() * float_size), m_positions.data(), BufferHandle::BUFFER_USAGE::DYNAMIC_DRAW);
        }

        m_VBO_tex_coords0 = buffers[(GLuint)VAO_ATTRIB::TEXCOORD0];
        if (m_VBO_tex_coords0) {
            Renderer::m_pimpl->BindBufferHandle(m_VBO_tex_coords0);
            Renderer::m_pimpl->ConfigureBufferHandleData(m_VBO_tex_coords0, static_cast<uint32_t>(m_tex_coords.size() * sizeof(uint8_t)), m_tex_coords.data(), BufferHandle::BUFFER_USAGE::STATIC_DRAW);
        }

        m_VBO_colors = buffers[(GLuint)VAO_ATTRIB::COLOR];
        if (m_VBO_colors) {
            Renderer::m_pimpl->BindBufferHandle(m_VBO_colors);
            Renderer::m_pimpl->ConfigureBufferHandleData(m_VBO_colors, static_cast<uint32_t>(m_colors.size() * float_size), m_colors.data(), BufferHandle::BUFFER_USAGE::DYNAMIC_DRAW);
        }

        m_VBO_indices = buffers[(GLuint)VAO_ATTRIB::INDICES];
        if (m_VBO_indices) {
            Renderer::m_pimpl->BindBufferHandle(m_VBO_indices);
            Renderer::m_pimpl->ConfigureBufferHandleData(m_VBO_indices, static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t)), m_indices.data(), BufferHandle::BUFFER_USAGE::STATIC_DRAW);
        }
    }

}
