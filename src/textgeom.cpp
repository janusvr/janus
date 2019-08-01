#include "textgeom.h"

TextGeom::TextGeom() :    
    do_fixed_size(false),
    fixed_size(0.05f),
    maxx(1.0f),
    maxy(1.0f),    
    len(0.0f),
    height(1.5f),
    pos(0, 0, 0),
    dir(1, 0, 0),    
    vbo_rebuild(true),
    left_justify(false)
{
}

TextGeom::~TextGeom()
{
    Clear();
}

void TextGeom::initializeGL()
{
    QImage img(MathUtil::GetApplicationPath() + "assets/fonts/ubuntumono.png");
    img = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    auto tex_handle = RendererInterface::m_pimpl->CreateTextureQImage(img, true, true, true, TextureHandle::ALPHA_TYPE::BLENDED, TextureHandle::COLOR_SPACE::SRGB);
	RendererInterface::m_pimpl->SetDefaultFontGlyphAtlas(tex_handle);
}

void TextGeom::SetMaxSize(float x, float y)
{
    maxx = x;
    maxy = y;
}

float TextGeom::GetMaxSizeX() const
{
    return maxx;
}

float TextGeom::GetMaxSizeY() const
{
    return maxy;
}

void TextGeom::AddText(const QString & s, const QColor col)
{
//    qDebug() << "TextGeom::AddText" << s << col;
    TextGeomLine l;
    l.text = s;
    l.col = col;
    texts.push_back(l);

    vbo_rebuild = true;
}

void TextGeom::SetColour(const QColor & c)
{
    for (int i=0; i<texts.size(); ++i) {
        texts[i].col = c;
    }
}

QColor TextGeom::GetColour() const
{
    if (!texts.empty()) {
        return texts.first().col;
    }
    else {
        return QColor(255,255,255);
    }
}

void TextGeom::SetText(const QString & s, const QColor col)
{
//    qDebug() << "TextGeom::SetText" << s;
    if (!texts.empty() && QString::compare(texts.first().text, s) == 0) {
        return;
    }

    Clear();

    TextGeomLine l;
    l.text = s;
    l.col = col;
    texts.push_back(l);   

    len = ((do_fixed_size ? fixed_size : 1.0f) * s.length());

    vbo_rebuild = true;
}

QString TextGeom::GetText() const
{
    if (texts.empty()) {
        return QString("");
    }
    else {
        return texts.first().text;
    }
}

float TextGeom::GetTextLength() const
{
    return len;
}

QVector <TextGeomLine> TextGeom::GetAllText() const
{
    return texts;
}

void TextGeom::SetFixedSize(const bool b, const float f)
{
    do_fixed_size = b;
    fixed_size = f;
}

float TextGeom::GetFixedSize() const
{
    return fixed_size;
}

void TextGeom::SetLeftJustify(const bool b)
{
    left_justify = b;
}

bool TextGeom::GetLeftJustify() const
{
    return left_justify;
}

float TextGeom::GetScale() const
{    
    if (!do_fixed_size && len == 0.0f) {
        return 0.0f;
    }
    return (do_fixed_size ? fixed_size : qMin(maxx / len, maxy * height));
}

float TextGeom::GetHeight() const
{
    return height;
}

QMatrix4x4 TextGeom::GetModelMatrix() const
{
    const float s = GetScale();    
    QMatrix4x4 m;
    m.scale(s);

    if (!left_justify) {
        if (do_fixed_size) {
            m.translate(QVector3D(-len / fixed_size / 2.0f, 0, 0));
        }
        else {
            m.translate(QVector3D(-len / 2.0f, 0, 0));
        }
    }

    return m;
}

void TextGeom::CreateVBO()
{
//    qDebug() << "TextGeom::CreateVBO()" << this;
    const uint8_t indices_per_glyph = 6;
    const uint8_t vertices_per_glyph = 4;
    const uint8_t elements_per_position = 4;
    const uint8_t elements_per_texcoord = 2;

    const int32_t line_count = texts.size();
    for (int32_t text_index = 0; text_index < line_count; ++text_index)
    {
        const QByteArray eachline = texts[text_index].text.toLatin1();
        const int32_t glyph_count = eachline.length();
        bool index_buffer_dirty = false;
        bool position_buffer_dirty = false;
        bool texcoord_buffer_dirty = false;

        // Skip empty lines
        if (glyph_count == 0) {
            continue;
        }

        position_buffer_dirty = true;
        texcoord_buffer_dirty = true;
        index_buffer_dirty = true;
        texts[text_index].m_positions.resize(glyph_count * vertices_per_glyph * 4);
        texts[text_index].m_texcoords.resize(glyph_count * vertices_per_glyph * 2);
        texts[text_index].m_indices.resize(glyph_count * indices_per_glyph);

        //set index data
        for (int32_t glyph_index = 0; glyph_index < glyph_count; ++glyph_index)
        {
            uint32_t const index_offset = glyph_index * 6;
            uint32_t const index_vert_offset = glyph_index * vertices_per_glyph;
            texts[text_index].m_indices[index_offset + 0] = index_vert_offset + 0;
            texts[text_index].m_indices[index_offset + 1] = index_vert_offset + 1;
            texts[text_index].m_indices[index_offset + 2] = index_vert_offset + 2;
            texts[text_index].m_indices[index_offset + 3] = index_vert_offset + 0;
            texts[text_index].m_indices[index_offset + 4] = index_vert_offset + 2;
            texts[text_index].m_indices[index_offset + 5] = index_vert_offset + 3;
        }

        //set vertex data
        for (int32_t glyph_index = 0; glyph_index < glyph_count; ++glyph_index)
        {
            for (uint32_t vert_index = 0; vert_index < vertices_per_glyph; ++vert_index)
            {
                const uint32_t base_vertex = glyph_index * vertices_per_glyph;
                const float yMin = (-float(text_index)-0.5f) * height;
                const float yMax = (-float(text_index)+0.5f) * height;

                const uint32_t position_base = base_vertex * elements_per_position;
                const uint32_t position_offset = vert_index * elements_per_position + position_base;

                switch (vert_index)
                {
                case 0:
                    texts[text_index].m_positions[position_offset + 0] = float(glyph_index);
                    texts[text_index].m_positions[position_offset + 1] = float(yMax);
                    texts[text_index].m_positions[position_offset + 2] = float(0);
                    texts[text_index].m_positions[position_offset + 3] = float(1);
                    break;
                case 1:
                    texts[text_index].m_positions[position_offset + 0] = float(glyph_index);
                    texts[text_index].m_positions[position_offset + 1] = float(yMin);
                    texts[text_index].m_positions[position_offset + 2] = float(0);
                    texts[text_index].m_positions[position_offset + 3] = float(1);
                    break;
                case 2:
                    texts[text_index].m_positions[position_offset + 0] = float(glyph_index + 1);
                    texts[text_index].m_positions[position_offset + 1] = float(yMin);
                    texts[text_index].m_positions[position_offset + 2] = float(0);
                    texts[text_index].m_positions[position_offset + 3] = float(1);
                    break;
                case 3:
                    texts[text_index].m_positions[position_offset + 0] = float(glyph_index + 1);
                    texts[text_index].m_positions[position_offset + 1] = float(yMax);
                    texts[text_index].m_positions[position_offset + 2] = float(0);
                    texts[text_index].m_positions[position_offset + 3] = float(1);
                    break;
                default:
                    break;
                }
            }
        }

        //set texcoord data
        for (int32_t glyph_index = 0; glyph_index < glyph_count; ++glyph_index)
        {
            uint8_t const glyph_code = eachline[glyph_index];
            uint8_t const glyph_x = glyph_code % 16;
            uint8_t const glyph_y = (glyph_code - glyph_x) / 16;
            float const space = 0.02f;
            float const tx1 = float(glyph_x) / 16.0f;
            float const tx2 = float(glyph_x+1) / 16.0f - space;
            float const ty1 = float(16-(glyph_y+1)) / 16.0f - space  * 0.7f;
            float const ty2 = float(16-glyph_y) / 16.0f - space * 0.8f;

            for (uint32_t vert_index = 0; vert_index < vertices_per_glyph; ++vert_index)
            {
                uint32_t const base_vertex = glyph_index * vertices_per_glyph;
                uint32_t const texcoord_base = base_vertex * elements_per_texcoord;
                uint32_t const texcoord_offset = vert_index * elements_per_texcoord + texcoord_base;

                switch (vert_index)
                {
                case 0:
                    texts[text_index].m_texcoords[texcoord_offset + 0] = float(tx1);
                    texts[text_index].m_texcoords[texcoord_offset + 1] = float(ty2);
                    break;
                case 1:
                    texts[text_index].m_texcoords[texcoord_offset + 0] = float(tx1);
                    texts[text_index].m_texcoords[texcoord_offset + 1] = float(ty1);
                    break;
                case 2:
                    texts[text_index].m_texcoords[texcoord_offset + 0] = float(tx2);
                    texts[text_index].m_texcoords[texcoord_offset + 1] = float(ty1);
                    break;
                case 3:
                    texts[text_index].m_texcoords[texcoord_offset + 0] = float(tx2);
                    texts[text_index].m_texcoords[texcoord_offset + 1] = float(ty2);
                    break;
                default:
                    break;
                }
            }
        }

        //set VAO
        const int32_t float_type = GL_FLOAT;
        const int32_t float_size = sizeof(float);

        //65.3 - deallocate previous buffers and mesh (reallocation will allow for index reuse)
        if (!texts[text_index].m_mesh_handle.isNull()) {
            QVector<QPointer<BufferHandle>> buffers = RendererInterface::m_pimpl->GetBufferHandlesForMeshHandle(texts[text_index].m_mesh_handle);
            for (int i=0; i<buffers.size(); ++i) {
                RendererInterface::m_pimpl->RemoveBufferHandleFromMap(buffers[i]);
            }
            RendererInterface::m_pimpl->RemoveMeshHandleFromMap(texts[text_index].m_mesh_handle);
        }

        VertexAttributeLayout layout;
        layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].in_use = true;
        layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].buffer_id = VAO_ATTRIB::POSITION;
        layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_count = 3;
        layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_type = float_type;
        layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].offset_in_bytes = 0;
        layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].stride_in_bytes = 4 * float_size;

        layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].in_use = true;
        layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].buffer_id = VAO_ATTRIB::TEXCOORD0;
        layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_count = 2;
        layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_type = float_type;
        layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].offset_in_bytes = 0;
        layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].stride_in_bytes = 2 * float_size;

        layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].in_use = true;
        layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].buffer_id = VAO_ATTRIB::INDICES;
        layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_count = 1;
        layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_type = GL_UNSIGNED_INT;
        layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].offset_in_bytes = 0;
        layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].stride_in_bytes = 1 * sizeof(uint32_t);

        texts[text_index].m_mesh_handle = RendererInterface::m_pimpl->CreateMeshHandle(layout);

        QVector<QPointer<BufferHandle>> buffers = RendererInterface::m_pimpl->GetBufferHandlesForMeshHandle(texts[text_index].m_mesh_handle);

        if (buffers.size() >= VAO_ATTRIB::NUM_ATTRIBS) {
            texts[text_index].m_position_handle = buffers[(GLuint)VAO_ATTRIB::POSITION];
            RendererInterface::m_pimpl->BindBufferHandle(texts[text_index].m_position_handle);
            RendererInterface::m_pimpl->ConfigureBufferHandleData(texts[text_index].m_position_handle, glyph_count * vertices_per_glyph * 4 * float_size, texts[text_index].m_positions.data(), BufferHandle::BUFFER_USAGE::STATIC_DRAW);

            texts[text_index].m_texcoord_handle = buffers[(GLuint)VAO_ATTRIB::TEXCOORD0];
            RendererInterface::m_pimpl->BindBufferHandle(texts[text_index].m_texcoord_handle);
            RendererInterface::m_pimpl->ConfigureBufferHandleData(texts[text_index].m_texcoord_handle, glyph_count * vertices_per_glyph * 2 * float_size, texts[text_index].m_texcoords.data(), BufferHandle::BUFFER_USAGE::STATIC_DRAW);

            texts[text_index].m_index_handle = buffers[(GLuint)VAO_ATTRIB::INDICES];
            RendererInterface::m_pimpl->BindBufferHandle(texts[text_index].m_index_handle);
            RendererInterface::m_pimpl->ConfigureBufferHandleData(texts[text_index].m_index_handle, glyph_count * indices_per_glyph * 1 * sizeof(uint32_t), texts[text_index].m_indices.data(), BufferHandle::BUFFER_USAGE::STATIC_DRAW);
        }
    }    
}

void TextGeom::DrawSelectedGL(QPointer <AssetShader> shader)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    if (texts.isEmpty()) {
        return;
    }

    for (int j=0; j<texts.size(); ++j) {

        const int glyph_count = texts[j].text.length();
        if (glyph_count == 0) {
            continue;
        }

        RendererInterface * renderer = RendererInterface::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                glyph_count * 6,
                                0,
                                0,
                                0,
                                texts[j].m_mesh_handle,
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

void TextGeom::DrawGL(QPointer <AssetShader> shader)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }   

//    qDebug() << "TextGeom::DrawGL" << texts.size();
    if (texts.isEmpty()) {
        return;
    }

    if (vbo_rebuild) {
//        qDebug() << "TextGeom::DrawGL" << vbo_rebuild << texts.first().text;
        CreateVBO();
        vbo_rebuild = false;
    }

    shader->SetUseTextureAll(false);
    shader->SetUseTexture(0, true);
    shader->SetAmbient(QVector3D(1,1,1));
    shader->SetDiffuse(QVector3D(1,1,1));
    shader->SetEmission(QVector3D(0,0,0));
    shader->SetUseLighting(false);

    RendererInterface::m_pimpl->BindTextureHandle(0, RendererInterface::m_pimpl->GetDefaultFontGlyphAtlas());
    for (int j=0; j<texts.size(); ++j) {

        const int glyph_count = texts[j].text.length();
        if (glyph_count == 0) {
            continue;
        }

//        qDebug() << "TextGeom::DrawGL" << texts[j].col << texts[j].text;
        shader->SetConstColour(QVector4D(texts[j].col.redF(), texts[j].col.greenF(), texts[j].col.blueF(), texts[j].col.alphaF()));
        shader->UpdateObjectUniforms();

        RendererInterface * renderer = RendererInterface::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                glyph_count * 6,
                                0,
                                0,
                                0,
                                texts[j].m_mesh_handle,
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
    shader->SetUseTexture(0, false);
    shader->SetConstColour(QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
}

void TextGeom::Clear()
{        
    texts.clear();
    len = 0.0f;
    vbo_rebuild = true;
}
