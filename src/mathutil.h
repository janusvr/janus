#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <QtGui>

#include <qopengl.h>
#include <qopenglext.h>

#include <QtWidgets>
#include <QVector3D>

#include <GL/glu.h>

#ifdef WIN32
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <cfloat>
#include <QtDebug>
#include <memory>

#include "scriptablevector.h"

#define GLM_FORCE_INLINE
#define GLM_FORCE_EXPLICIT_CTOR

#define MAX_LIGHTS                          64
#define ASSETSHADER_MAX_JOINTS              128
#define ASSETSHADER_NUM_CUBEMAPS            4
#define ASSETSHADER_NUM_TEXTURES            10
#define ASSETSHADER_NUM_COMBINED_TEXURES    14
#define ASSETSHADER_MAX_TEXTURE_UNITS       16
#define BUFFER_CHUNK_COUNT                  6

enum VAO_ATTRIB
{
    POSITION			=  0,
    NORMAL				=  1,
    TEXCOORD0			=  2,
    TEXCOORD1			=  3,
    COLOR				=  4,
    BLENDV0				=  5,
    BLENDV1				=  6,
    BLENDV2				=  7,
    BLENDN0				=  8,
    BLENDN1				=  9,
    BLENDN2				= 10,
    SKELANIMINDICES		= 11,
    SKELANIMWEIGHTS		= 12,
    INDICES             = 13,
    NUM_ATTRIBS			= 14,
};

class VertexAttributeLayout
{
public:
    class VertexAttribute
    {
    public:
        VertexAttribute()
            : in_use(false),
              buffer_id(VAO_ATTRIB::NUM_ATTRIBS),
              attrib(VAO_ATTRIB::NUM_ATTRIBS),
              element_count(4),
              element_type(0),
              stride_in_bytes(0),
              offset_in_bytes(0),
              is_normalized(true),
              is_float_attrib(true)
        {

        }

        bool in_use;
        // If we are interleaved this needs to be the ID of the attrib buffer we are in
        VAO_ATTRIB buffer_id;
        VAO_ATTRIB attrib;
        int32_t element_count;
        int32_t element_type;
        int32_t stride_in_bytes;
        intptr_t offset_in_bytes;
        bool is_normalized;
        bool is_float_attrib;
    };

    VertexAttributeLayout()
    {
        auto num_attribs = (uint32_t)VAO_ATTRIB::NUM_ATTRIBS;
        for (uint32_t attrib_index = 0; attrib_index < num_attribs; ++attrib_index)
        {
            attributes[attrib_index].attrib = (VAO_ATTRIB)attrib_index;
            attributes[attrib_index].buffer_id = (VAO_ATTRIB)attrib_index;
        }

    }

    VertexAttribute attributes[VAO_ATTRIB::NUM_ATTRIBS];
};


class ProgramHandle : public QObject
{
public:
    class ProgramUUID
    {
    public:
        ProgramUUID()
        {
            m_UUID = 0;
            m_in_use_flag = 0;
        }
        uint32_t m_UUID : 32;
        uint16_t m_in_use_flag : 1;
    };

    ProgramHandle()
    {
    }

    ProgramHandle(uint32_t const p_UUID)
    {
        m_UUID.m_UUID = p_UUID;
        m_UUID.m_in_use_flag = 1;
    }
    ~ProgramHandle()
    {

    }

    ProgramUUID m_UUID;
};

namespace FBO_TEXTURE
{
    enum
    {
        COLOR                   = 0,
        AO                      = 1,
        TRANSMISSION            = 2,
        MODULATION_DIFFUSION    = 3,
        DELTA                   = 4,
        COMPOSITED              = 5,
        DEPTH_STENCIL           = 6,
        COUNT                   = 7
    };
}
typedef uint32_t FBO_TEXTURE_ENUM;

namespace FBO_TEXTURE_BITFIELD
{
    enum
    {
        NONE                    = 0,
        COLOR                   = 1 << 0,
        AO                      = 1 << 1,
        TRANSMISSION            = 1 << 2,
        MODULATION_DIFFUSION    = 1 << 3,
        DELTA                   = 1 << 4,
        COMPOSITED              = 1 << 5,
        DEPTH_STENCIL           = 1 << 6,
        ALL                     = COLOR | DEPTH_STENCIL // For now I don't want to bind the other layers
    };
}
typedef uint32_t FBO_TEXTURE_BITFIELD_ENUM;

namespace RENDERER
{
    enum RENDER_SCOPE
    {
        CURRENT_ROOM_SKYBOX = 0,
        CURRENT_ROOM_PORTAL_THUMBS = 1,
        CURRENT_ROOM_OBJECTS_OPAQUE = 2,
        CURRENT_ROOM_OBJECTS_CUTOUT = 3,
        CURRENT_ROOM_PORTAL_STENCILS = 4,
        CHILD_ROOM_SKYBOX = 5,
        CHILD_ROOM_OBJECTS_OPAQUE = 6,
        CHILD_ROOM_OBJECTS_CUTOUT = 7,
        CHILD_ROOM_PORTAL_DECORATIONS = 8,
        CHILD_ROOM_OBJECTS_BLENDED = 9,
        CURRENT_ROOM_PORTAL_DEPTH_REFRESH = 10,
        CURRENT_ROOM_PORTAL_DECORATIONS = 11,
        CURRENT_ROOM_OBJECTS_BLENDED = 12,
        VIRTUAL_MENU = 13,
        CONTROLLERS = 14,
        AVATARS = 15,
        MENU = 16,
        CURSOR = 17,
        OVERLAYS = 18,
        POST_PROCESS = 19,
        SCOPE_COUNT = 20,
        CURRENT_ROOM_OBJECTS = 21,
        CHILD_ROOM_OBJECTS = 22,
        ALL = 23,
        NONE = 24
    };
}

static inline bool Float16ArrayCompare(float const * lhs, float const * rhs)
{
    for (int i = 0; i < 16; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

static inline bool Float4ArrayCompare(float const * lhs, float const * rhs)
{
    for (int i = 0; i < 4; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

static inline bool JointsArrayCompare(float const * lhs, float const * rhs)
{
    for (int i = 0; i < 16 * ASSETSHADER_MAX_JOINTS; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

static inline bool UseTextureArrayCompare(float const * lhs, float const * rhs)
{
    for (int i = 0; i < ASSETSHADER_NUM_COMBINED_TEXURES; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

class AssetShader_Frame
{
public:
    AssetShader_Frame() :
        iLeftEye(0, 0, 0, 0),
        iMouse(0, 0, 0, 0),
        iResolution(0, 0, 0, 0),
        iGlobalTime(0, 0, 0, 0)
    {

    }

    bool operator ==(const AssetShader_Frame &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Frame)) == 0);
    }

    bool operator !=(const AssetShader_Frame &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Frame)) != 0);
    }

    QVector4D iLeftEye;
    QVector4D iMouse;
    QVector4D iResolution;
    QVector4D iGlobalTime;
    QVector4D iViewportCount;
};

class AssetShader_Room
{
public:
    AssetShader_Room();

    bool operator ==(const AssetShader_Room &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Room)) == 0);
    }

    bool operator !=(const AssetShader_Room &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Room)) != 0);
    }

    float iMiscRoomData[16];
    QVector4D iPlayerPosition;
    QVector4D iUseClipPlane;
    QVector4D iClipPlane;
    QVector4D iFogEnabled;
    QVector4D iFogMode;
    QVector4D iFogDensity;
    QVector4D iFogStart;
    QVector4D iFogEnd;
    QVector4D iFogCol;
    float iRoomMatrix[16];
};

// Forward declaration to allow constructor and assignment overloads
class AssetShader_Object;

class AssetShader_Object_Compact
{
public:

    AssetShader_Object_Compact();

    ~AssetShader_Object_Compact();

    AssetShader_Object_Compact(const AssetShader_Object& rhs);

    bool operator ==(const AssetShader_Object_Compact &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Object_Compact)) == 0);
    }

    bool operator !=(const AssetShader_Object_Compact &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Object_Compact)) != 0);
    }

    float iModelMatrix[16];
    float iViewMatrix[16];
    float iInverseViewMatrix[16];
    float iProjectionMatrix[16];
    float iModelViewMatrix[16];
    float iModelViewProjectionMatrix[16];
    float iTransposeInverseModelMatrix[16];
    float iTransposeInverseModelViewMatrix[16];
};

class AssetShader_Object : public AssetShader_Object_Compact
{
public:
    AssetShader_Object();

    ~AssetShader_Object();

    bool operator ==(const AssetShader_Object &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Object)) == 0);
    }

    bool operator !=(const AssetShader_Object &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Object)) != 0);
    }

    int32_t m_draw_layer;
    QVector4D m_room_space_position_and_distance;
    int32_t iMiscObjectData[4];
    float iConstColour[4];
    float iChromaKeyColour[4];
    float iUseFlags[4];
    QVector<float> iSkelAnimJoints;
};

class AssetShader_Material
{
public:
    AssetShader_Material();

    bool operator ==(const AssetShader_Material &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Material)) == 0);
    }

    bool operator !=(const AssetShader_Material &rhs) const
    {
        return (memcmp(this, &(rhs), sizeof(AssetShader_Material)) != 0);
    }

    QVector4D iAmbient;
    QVector4D iDiffuse;
    QVector4D iSpecular;
    QVector4D iShininess;
    QVector4D iEmission;
    QVector4D iTiling;
    QVector4D iLightmapScale;
    float iUseTexture[16];
};

enum class PrimitiveType : uint32_t
{
    POINTS = 0x0000,
    LINES = 0x0001,
    LINE_LOOP = 0x0002,
    LINE_STRIP = 0x0003,
    TRIANGLES = 0x0004,
    TRIANGLE_STRIP = 0x0005,
    TRIANGLE_FAN = 0x0006,
    LINE_STRIP_ADJACENCY = 0x000B,
    TRIANGLES_ADJACENCY = 0x000C,
    TRIANGLE_STRIP_ADJACENCY = 0x000D,
    PATCHES = 0x000E
};

enum class FaceCullMode : uint32_t
{
    FRONT = 0x0404,
    BACK = 0x0405,
    FRONT_AND_BACK = 0x0408,
    DISABLED = 0x0B44
};

enum class DepthFunc : uint32_t
{
    NEVER = 0x0200,
    LESS = 0x0201,
    EQUAL = 0x0202,
    LEQUAL = 0x0203,
    GREATER = 0x0204,
    NOTEQUAL = 0x0205,
    GEQUAL = 0x0206,
    ALWAYS = 0x0207
};

enum class DepthMask : uint32_t
{
    DEPTH_WRITES_DISABLED = 0,
    DEPTH_WRITES_ENABLED = 1,
};

enum class ColorMask : uint32_t
{
    COLOR_WRITES_DISABLED = 0,
    COLOR_WRITES_ENABLED = 1
};

enum class StencilTestFuncion : uint32_t
{
    NEVER = 0x0200,
    LESS = 0x0201,
    EQUAL = 0x0202,
    LEQUAL = 0x0203,
    GREATER = 0x0204,
    NOTEQUAL = 0x0205,
    GEQUAL = 0x0206,
    ALWAYS = 0x0207
};

typedef GLint StencilReferenceValue;
typedef GLuint StencilMask;

enum class StencilOpAction : uint32_t
{
    KEEP = 0x1E00,
    ZERO = 0,
    REPLACE = 0x1E01,
    INCREMENT = 0x1E02,
    INCREMENT_WRAP = 0x8507,
    DECREMENT = 0x1E03,
    DECREMENT_WRAP = 0x8508,
    INVERT = 0x150A
};

enum class BlendFunction : uint32_t
{
    ZERO = 0,
    ONE = 1,
    SRC_COLOR = 0x0300,
    ONE_MINUS_SRC_COLOR = 0x0301,
    DST_COLOR = 0x0306,
    ONE_MINUS_DST_COLOR = 0x0307,
    SRC_ALPHA = 0x0302,
    ONE_MINUS_SRC_ALPHA = 0x0303,
    DST_ALPHA = 0x0304,
    ONE_MINUS_DST_ALPHA = 0x0305,
    CONSTANT_COLOR = 0x8001,
    ONE_MINUS_CONSTANT_COLOR = 0x8002,
    CONSTANT_ALPHA = 0x8003,
    ONE_MINUS_CONSTANT_ALPHA = 0x8004,
    SRC_ALPHA_SATURATE = 0x0308,
    SRC1_COLOR = 0x88F9,
    ONE_MINUS_SRC1_COLOR = 0x88FA,
    SRC1_ALPHA = 0x8589
};

enum class TextureTarget : uint32_t
{
    TEXTURE_2D = 0x0DE1,
    TEXTURE_CUBEMAP = 0x8513
};

class StencilOp
{
public:
    StencilOp(StencilOpAction p_stencil_fail, StencilOpAction p_depth_fail, StencilOpAction p_pass)
        : m_stencil_fail(p_stencil_fail), m_depth_fail(p_depth_fail), m_pass(p_pass)
    {

    }

    StencilOp()
        : m_stencil_fail(StencilOpAction::KEEP), m_depth_fail(StencilOpAction::KEEP), m_pass(StencilOpAction::KEEP)
    {

    }

    bool operator== (StencilOp const & rhs)
    {
        if (m_stencil_fail != rhs.m_stencil_fail)
        {
            return false;
        }
        else if (m_depth_fail != rhs.m_depth_fail)
        {
            return false;
        }
        else if (m_pass != rhs.m_pass)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool operator!= (StencilOp const & rhs)
    {
        return !(*this == rhs);
    }

    StencilOpAction GetStencilFailAction() const { return m_stencil_fail; }
    void SetStencilFailAction(StencilOpAction p_stencil_fail) { m_stencil_fail = p_stencil_fail; }
    StencilOpAction GetDepthFailAction() const { return m_depth_fail; }
    void SetDepthFailAction(StencilOpAction p_depth_fail) { m_depth_fail = p_depth_fail; }
    StencilOpAction GetPassAction() const { return m_pass; }
    void SetPassAction(StencilOpAction p_pass) { m_pass = p_pass; }

//private:
    StencilOpAction m_stencil_fail;
    StencilOpAction m_depth_fail;
    StencilOpAction m_pass;
};

class StencilFunc
{
public:
    StencilFunc(StencilTestFuncion p_test_function, StencilReferenceValue p_ref_value, StencilMask p_mask)
        : m_test_function(p_test_function), m_ref_value(p_ref_value), m_mask(p_mask)
    {

    }

    StencilFunc()
        : m_test_function(StencilTestFuncion::ALWAYS), m_ref_value(0), m_mask(0xffffffff)
    {

    }

    ~StencilFunc() {}
    bool operator==(StencilFunc const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(StencilFunc)) == 0);
    }

    bool operator!=(StencilFunc const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(StencilFunc)) != 0);
    }

    StencilTestFuncion GetStencilTestFunction() const { return m_test_function; }
    void SetStencilTestFunction(StencilTestFuncion p_test_function) { m_test_function = p_test_function; }
    StencilReferenceValue GetStencilReferenceValue() const { return m_ref_value; }
    void SetStencilReferenceValue(StencilReferenceValue p_ref_value) { m_ref_value = p_ref_value; }
    StencilMask GetStencilMask() const { return m_mask; }
    void SetStencilMask(StencilMask p_mask) { m_mask = p_mask; }

//private:

    StencilTestFuncion m_test_function;
    StencilReferenceValue m_ref_value;
    StencilMask m_mask;
};

// This class always has a shared_ptr stored in the renderer, at a set tick-rate the renderer walks the vector of shared_ptr and looks for any
// with a ref count of 1 meaning that all other references are gone, when this is detected the renderer marks the texture as free-to-delete
// Deletions do not happen right away, instead we keep the texture around using a LRU system to free old textures when a new one is needed
// This lets us keep textures for things like multi-player avatars around in memory between rooms to speed up the repeated loading of them even
// after there are no instances of that texture alive for a short time,
class TextureHandle : public QObject
{
public:
    class Texture_UUID
    {
    public:
        Texture_UUID()
        {
            m_UUID = 0;
            m_in_use_flag = 0;
            m_texture_type = 0;
            m_color_space = 0;
            m_alpha_type = 0;
        }
        uint32_t m_UUID : 26;
        uint32_t m_in_use_flag : 1;
        uint32_t m_texture_type : 2;
        uint32_t m_color_space : 1;
        uint32_t m_alpha_type : 2;
    };

    enum TEXTURE_TYPE : uint32_t
    {
        TEXTURE_2D = 0,
        TEXTURE_CUBEMAP = 1,
        TEXTURE_RECTANGLE = 2
    };

    enum COLOR_SPACE : uint32_t
    {
        LINEAR = 0,
        SRGB = 1,
    };

    enum ALPHA_TYPE : uint32_t
    {
        NONE = 0,
        CUTOUT = 1,
        BLENDED = 2,
        MIXED = 3,
        UNDEFINED = 4
    };

    TextureHandle()
    {
    }

    TextureHandle(uint32_t p_UUID, TEXTURE_TYPE p_texture_type, COLOR_SPACE p_color_space, ALPHA_TYPE p_alpha_type)
    {
        m_UUID.m_UUID = p_UUID;
        m_UUID.m_in_use_flag = 1;
        m_UUID.m_texture_type = (uint32_t)p_texture_type;
        m_UUID.m_color_space = (uint32_t)p_color_space;
        m_UUID.m_alpha_type = (uint32_t)p_alpha_type;
    }
    ~TextureHandle() {}

    bool operator!=(TextureHandle const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(TextureHandle)) != 0);
    }

    bool operator==(TextureHandle const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(TextureHandle)) == 0);
    }

    Texture_UUID GetUUID() { return m_UUID; }
    bool GetInUseFlag() { return m_UUID.m_in_use_flag == 1; }
    TEXTURE_TYPE GetTextureType() { return (TEXTURE_TYPE)m_UUID.m_texture_type; }
    COLOR_SPACE GetColorSpace() { return (COLOR_SPACE)m_UUID.m_color_space; }
    ALPHA_TYPE GetAlphaType() { return (ALPHA_TYPE)m_UUID.m_alpha_type; }
    void Clear() { m_UUID = Texture_UUID(); }

    Texture_UUID m_UUID;
};

// This class always has a shared_ptr stored in the renderer, at a set tick-rate the renderer walks the vector of shared_ptr and looks for any
// with a ref count of 1 meaning that all other references are gone, when this is detected the renderer marks the texture as free-to-delete
// Deletions do not happen right away, instead we keep the texture around using a LRU system to free old textures when a new one is needed
// This lets us keep textures for things like multi-player avatars around in memory between rooms to speed up the repeated loading of them even
// after there are no instances of that texture alive for a short time,
class MeshHandle : public QObject
{
public:
    class MeshUUID
    {
    public:
        MeshUUID()
        {
            m_UUID = 0;
            m_in_use_flag = 0;
            m_has_POSITION = 0;
            m_has_NORMAL = 0;
            m_has_TEXCOORD0 = 0;
            m_has_TEXCOORD1 = 0;
            m_has_COLOR = 0;
            m_has_BLENDV0 = 0;
            m_has_BLENDV1 = 0;
            m_has_BLENDV2 = 0;
            m_has_BLENDN0 = 0;
            m_has_BLENDN1 = 0;
            m_has_BLENDN2 = 0;
            m_has_SKELANIMINDICES = 0;
            m_has_SKELANIMWEIGHTS = 0;
            m_has_INDICES = 0;
        }
        uint32_t m_UUID : 32;
        uint16_t m_in_use_flag : 1;
        uint16_t m_has_POSITION : 1;
        uint16_t m_has_NORMAL : 1;
        uint16_t m_has_TEXCOORD0 : 1;
        uint16_t m_has_TEXCOORD1 : 1;
        uint16_t m_has_COLOR : 1;
        uint16_t m_has_BLENDV0 : 1;
        uint16_t m_has_BLENDV1 : 1;
        uint16_t m_has_BLENDV2 : 1;
        uint16_t m_has_BLENDN0 : 1;
        uint16_t m_has_BLENDN1 : 1;
        uint16_t m_has_BLENDN2 : 1;
        uint16_t m_has_SKELANIMINDICES : 1;
        uint16_t m_has_SKELANIMWEIGHTS : 1;
        uint16_t m_has_INDICES : 1;
    };

    MeshHandle()
    {
    }

    MeshHandle(uint32_t const p_UUID,
        bool const p_has_POSITION,
        bool const p_has_NORMAL,
        bool const p_has_TEXCOORD0,
        bool const p_has_TEXCOORD1,
        bool const p_has_COLOR,
        bool const p_has_BLENDV0,
        bool const p_has_BLENDV1,
        bool const p_has_BLENDV2,
        bool const p_has_BLENDN0,
        bool const p_has_BLENDN1,
        bool const p_has_BLENDN2,
        bool const p_has_SKELANIMINDICES,
        bool const p_has_SKELANIMWEIGHTS,
        bool const p_has_INDICES)
    {
        m_UUID.m_UUID = p_UUID;
        m_UUID.m_in_use_flag = 1;
        m_UUID.m_has_POSITION = (p_has_POSITION == true) ? 1 : 0;
        m_UUID.m_has_NORMAL = (p_has_NORMAL == true) ? 1 : 0;
        m_UUID.m_has_TEXCOORD0 = (p_has_TEXCOORD0 == true) ? 1 : 0;
        m_UUID.m_has_TEXCOORD1 = (p_has_TEXCOORD1 == true) ? 1 : 0;
        m_UUID.m_has_COLOR = (p_has_COLOR == true) ? 1 : 0;
        m_UUID.m_has_BLENDV0 = (p_has_BLENDV0 == true) ? 1 : 0;
        m_UUID.m_has_BLENDV1 = (p_has_BLENDV1 == true) ? 1 : 0;
        m_UUID.m_has_BLENDV2 = (p_has_BLENDV2 == true) ? 1 : 0;
        m_UUID.m_has_BLENDN0 = (p_has_BLENDN0 == true) ? 1 : 0;
        m_UUID.m_has_BLENDN1 = (p_has_BLENDN1 == true) ? 1 : 0;
        m_UUID.m_has_BLENDN2 = (p_has_BLENDN2 == true) ? 1 : 0;
        m_UUID.m_has_SKELANIMINDICES = (p_has_SKELANIMINDICES == true) ? 1 : 0;
        m_UUID.m_has_SKELANIMWEIGHTS = (p_has_SKELANIMWEIGHTS == true) ? 1 : 0;
        m_UUID.m_has_INDICES = (p_has_INDICES == true) ? 1 : 0;
    }
    ~MeshHandle() {}

    bool operator!=(MeshHandle const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(MeshHandle)) != 0);
    }

    bool operator==(MeshHandle const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(MeshHandle)) == 0);
    }

    void Clear() { m_UUID = MeshUUID(); }

    MeshUUID m_UUID;
};

class BufferHandle : public QObject
{
public:

    enum BUFFER_TYPE : uint16_t
    {
        ARRAY_BUFFER = 1,
        ATOMIC_COUNTER_BUFFER = 2,
        COPY_READ_BUFFER = 3,
        COPY_WRITE_BUFFER = 4,
        DISPATCH_INDIRECT_BUFFER = 5,
        DRAW_INDIRECT_BUFFER = 6,
        ELEMENT_ARRAY_BUFFER = 7,
        PIXEL_PACK_BUFFER = 8,
        PIXEL_UNPACK_BUFFER = 9,
        QUERY_BUFFER = 10,
        SHADER_STORAGE_BUFFER = 11,
        TEXTURE_BUFFER = 12,
        TRANSFORM_FEEDBACK_BUFFER = 13,
        UNIFORM_BUFFER = 14,

    };

    #define BUFFER_TYPE_COUNT 14

    enum BUFFER_TYPE_ENUM : uint32_t
    {
        ARRAY_BUFFER_ENUM = 0x8892,
        ATOMIC_COUNTER_BUFFER_ENUM = 0x92C0,
        COPY_READ_BUFFER_ENUM = 0x8F36,
        COPY_WRITE_BUFFER_ENUM = 0x8F37,
        DISPATCH_INDIRECT_BUFFER_ENUM = 0x90EE,
        DRAW_INDIRECT_BUFFER_ENUM = 0x8F3F,
        ELEMENT_ARRAY_BUFFER_ENUM = 0x8893,
        PIXEL_PACK_BUFFER_ENUM = 0x88EB,
        PIXEL_UNPACK_BUFFER_ENUM = 0x88EC,
        QUERY_BUFFER_ENUM = 0x9192,
        SHADER_STORAGE_BUFFER_ENUM = 0x90D2,
        TEXTURE_BUFFER_ENUM = 0x8C2A,
        TRANSFORM_FEEDBACK_BUFFER_ENUM = 0x8C8E,
        UNIFORM_BUFFER_ENUM = 0x8A11
    };

    enum BUFFER_USAGE : uint16_t
    {
        STREAM_DRAW = 1,
        STREAM_READ = 2,
        STREAM_COPY = 3,
        STATIC_DRAW = 4,
        STATIC_READ = 5,
        STATIC_COPY = 6,
        DYNAMIC_DRAW = 7,
        DYNAMIC_READ = 8,
        DYNAMIC_COPY = 9
    };


    enum BUFFER_USAGE_ENUM : uint32_t
    {
        STREAM_DRAW_ENUM = 0x88E0,
        STREAM_READ_ENUM = 0x88E1,
        STREAM_COPY_ENUM = 0x88E2,
        STATIC_DRAW_ENUM = 0x88E4,
        STATIC_READ_ENUM = 0x88E5,
        STATIC_COPY_ENUM = 0x88E6,
        DYNAMIC_DRAW_ENUM = 0x88E8,
        DYNAMIC_READ_ENUM = 0x88E9,
        DYNAMIC_COPY_ENUM = 0x88EA
    };

    class BufferUUID
    {
    public:
        BufferUUID()
        {
            m_UUID = 0;
            m_in_use_flag = 0;
            m_buffer_type = 0;
            m_buffer_usage = 0;

        }
        uint32_t m_UUID : 32;
        uint16_t m_in_use_flag : 1;
        uint16_t m_buffer_type : 4;
        uint16_t m_buffer_usage : 4;
    };

    BufferHandle()
    {
    }

    BufferHandle(uint32_t const p_UUID,
        BUFFER_TYPE const p_buffer_type,
        BUFFER_USAGE const p_buffer_usage)
    {
        m_UUID.m_UUID = p_UUID;
        m_UUID.m_in_use_flag = 1;
        m_UUID.m_buffer_type = (uint16_t)p_buffer_type;
        m_UUID.m_buffer_usage = (uint16_t)p_buffer_usage;
    }
    ~BufferHandle() {}

    bool operator!=(BufferHandle const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(BufferHandle)) != 0);
    }

    bool operator==(BufferHandle const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(BufferHandle)) == 0);
    }

    void Clear() { m_UUID = BufferUUID(); }

    uint32_t GetBufferTypeEnum() const
    {
        return GetBufferTypeEnum((BUFFER_TYPE)m_UUID.m_buffer_type);
    }

    static inline uint32_t GetBufferTypeEnum(BUFFER_TYPE p_buffer_type)
    {
        uint32_t return_enum;

        switch (p_buffer_type)
        {
        case 1: return_enum = (uint32_t)BUFFER_TYPE_ENUM::ARRAY_BUFFER_ENUM; break;
        case 2: return_enum = (uint32_t)BUFFER_TYPE_ENUM::ATOMIC_COUNTER_BUFFER_ENUM; break;
        case 3: return_enum = (uint32_t)BUFFER_TYPE_ENUM::COPY_READ_BUFFER_ENUM; break;
        case 4: return_enum = (uint32_t)BUFFER_TYPE_ENUM::COPY_WRITE_BUFFER_ENUM; break;
        case 5: return_enum = (uint32_t)BUFFER_TYPE_ENUM::DISPATCH_INDIRECT_BUFFER_ENUM; break;
        case 6: return_enum = (uint32_t)BUFFER_TYPE_ENUM::DRAW_INDIRECT_BUFFER_ENUM; break;
        case 7: return_enum = (uint32_t)BUFFER_TYPE_ENUM::ELEMENT_ARRAY_BUFFER_ENUM; break;
        case 8: return_enum = (uint32_t)BUFFER_TYPE_ENUM::PIXEL_PACK_BUFFER_ENUM; break;
        case 9: return_enum = (uint32_t)BUFFER_TYPE_ENUM::PIXEL_UNPACK_BUFFER_ENUM; break;
        case 10: return_enum = (uint32_t)BUFFER_TYPE_ENUM::QUERY_BUFFER_ENUM; break;
        case 11: return_enum = (uint32_t)BUFFER_TYPE_ENUM::SHADER_STORAGE_BUFFER_ENUM; break;
        case 12: return_enum = (uint32_t)BUFFER_TYPE_ENUM::TEXTURE_BUFFER_ENUM; break;
        case 13: return_enum = (uint32_t)BUFFER_TYPE_ENUM::TRANSFORM_FEEDBACK_BUFFER_ENUM; break;
        case 14: return_enum = (uint32_t)BUFFER_TYPE_ENUM::UNIFORM_BUFFER_ENUM; break;
        default: return_enum = -1; break;
        }
        return return_enum;
    }

    static inline QString GetBufferTypeString(BUFFER_TYPE const p_buffer_type)
    {
        QString return_string;
        switch (p_buffer_type)
        {
        case 1: return_string = "ARRAY_BUFFER"; break;
        case 2: return_string = "ATOMIC_COUNTER_BUFFER"; break;
        case 3: return_string = "COPY_READ_BUFFER"; break;
        case 4: return_string = "COPY_WRITE_BUFFER"; break;
        case 5: return_string = "DISPATCH_INDIRECT_BUFFER"; break;
        case 6: return_string = "DRAW_INDIRECT_BUFFER"; break;
        case 7: return_string = "ELEMENT_ARRAY_BUFFER"; break;
        case 8: return_string = "PIXEL_PACK_BUFFER"; break;
        case 9: return_string = "PIXEL_UNPACK_BUFFER"; break;
        case 10: return_string = "QUERY_BUFFER"; break;
        case 11: return_string = "SHADER_STORAGE_BUFFER"; break;
        case 12: return_string = "TEXTURE_BUFFER; break";
        case 13: return_string = "TRANSFORM_FEEDBACK_BUFFER"; break;
        case 14: return_string = "UNIFORM_BUFFER"; break;
        default: return_string = "INVALID BUFFER TYPE"; break;
        }
        return return_string;
    }

    uint32_t GetBufferUsageEnum() const
    {
        uint32_t return_enum;
        return_enum = BufferHandle::GetBufferUsageEnum((BUFFER_USAGE)m_UUID.m_buffer_usage);
        return return_enum;
    }

    static inline uint32_t GetBufferUsageEnum(BUFFER_USAGE const p_buffer_usage)
    {
        uint32_t return_enum;

        switch (p_buffer_usage)
        {
        case 1: return_enum = (uint32_t)BUFFER_USAGE_ENUM::STREAM_DRAW_ENUM; break;
        case 2: return_enum = (uint32_t)BUFFER_USAGE_ENUM::STREAM_READ_ENUM; break;
        case 3: return_enum = (uint32_t)BUFFER_USAGE_ENUM::STREAM_COPY_ENUM; break;
        case 4: return_enum = (uint32_t)BUFFER_USAGE_ENUM::STATIC_DRAW_ENUM; break;
        case 5: return_enum = (uint32_t)BUFFER_USAGE_ENUM::STATIC_READ_ENUM; break;
        case 6: return_enum = (uint32_t)BUFFER_USAGE_ENUM::STATIC_COPY_ENUM; break;
        case 7: return_enum = (uint32_t)BUFFER_USAGE_ENUM::DYNAMIC_DRAW_ENUM; break;
        case 8: return_enum = (uint32_t)BUFFER_USAGE_ENUM::DYNAMIC_READ_ENUM; break;
        case 9: return_enum = (uint32_t)BUFFER_USAGE_ENUM::DYNAMIC_COPY_ENUM; break;
        default: return_enum = -1; break;
        }
        return return_enum;
    }

    static inline QString GetBufferUsageString(BUFFER_USAGE const p_buffer_usage)
    {
        QString return_string;
        switch (p_buffer_usage)
        {
        case 1: return_string  = "STREAM_DRAW"; break;
        case 2: return_string  = "STREAM_READ"; break;
        case 3: return_string  = "STREAM_COPY"; break;
        case 4: return_string  = "STATIC_DRAW"; break;
        case 5: return_string  = "STATIC_READ"; break;
        case 6: return_string  = "STATIC_COPY"; break;
        case 7: return_string  = "DYNAMIC_DRAW"; break;
        case 8: return_string  = "DYNAMIC_READ"; break;
        case 9: return_string  = "DYNAMIC_COPY"; break;
        default: return_string  = "INVALID BUFFER USAGE"; break;
        }
        return return_string;
    }

    BufferUUID m_UUID;
};

class TextureSet
{
public:
    TextureSet():
        m_is_3D_texture(false)
    {
        for (int i = 0; i < ASSETSHADER_NUM_COMBINED_TEXURES; ++i)
        {
            m_texture_handles_left[i] = nullptr;
            m_texture_handles_right[i] = nullptr;
        }
    }

    ~TextureSet()
    {

    }

    bool operator==(TextureSet const & rhs) const;

    bool operator!=(TextureSet const & rhs) const;

    void SetTextureHandle(int p_index, TextureHandle* p_id, uint p_is_stereo_image = 0);

    bool GetHasAlpha(int p_index) const;

    QPointer <TextureHandle> GetTextureHandle(int p_index, bool p_is_left_texture = true) const;

    TextureHandle::ALPHA_TYPE GetAlphaType(int p_index) const;
    bool GetIs3DTexture() const;

private:

    QPointer <TextureHandle> m_texture_handles_left[ASSETSHADER_NUM_COMBINED_TEXURES];
    QPointer <TextureHandle> m_texture_handles_right[ASSETSHADER_NUM_COMBINED_TEXURES];
    bool m_is_3D_texture;
};

enum class PROCESSING_STATE : unsigned long
{
    INVALID = 0,
    PENDING = 1,
    PROCESSING = 2,
    READY = 3
};

struct Cubemaps
{
    Cubemaps()
        : m_cubemap(nullptr),
          m_cube_maps(),
          m_channel_size(-1),
          m_processing_state(PROCESSING_STATE::INVALID)
    {

    }

    Cubemaps(QVector<QString> p_cube_maps, TextureHandle* p_cubemap)
        : m_cubemap(p_cubemap),
          m_cube_maps(p_cube_maps),
          m_channel_size(-1),
          m_processing_state(PROCESSING_STATE::PENDING)
    {

    }

    void clear_DDS_data()
    {
        m_dds_data[0].clear();
        m_dds_data[1].clear();
        m_dds_data[2].clear();
        m_dds_data[3].clear();
        m_dds_data[4].clear();
        m_dds_data[5].clear();
    }

    // First 6 QString are the input cubemap face paths, anything after that is used for output paths.
    QPointer <TextureHandle> m_cubemap;
    QVector <QString> m_cube_maps;
    QVector <char> m_dds_data[6];
    int32_t m_channel_size;
    PROCESSING_STATE m_processing_state;
};

class AbstractRenderCommand
{
public:

    AbstractRenderCommand();
    AbstractRenderCommand(PrimitiveType p_primitive_type,
        GLuint p_primitive_count,
        GLuint p_first_index,
        GLuint p_base_vertex,
        GLuint p_base_instance,
        MeshHandle* p_mesh_handle,
        ProgramHandle* p_shader,
        AssetShader_Frame p_frame_uniforms,
        AssetShader_Room p_room_uniforms,
        AssetShader_Object p_object_uniforms,
        AssetShader_Material p_material_uniforms,
        TextureSet p_texture_set,
        FaceCullMode p_face_cull_mode,
        DepthFunc p_depth_func,
        DepthMask p_depth_mask,
        StencilFunc p_stencil_func,
        StencilOp p_stencil_op,
        ColorMask p_color_mask);
    AbstractRenderCommand(const AbstractRenderCommand& p_copy);
    AbstractRenderCommand(AbstractRenderCommand&& p_move);

    ~AbstractRenderCommand();

    // Copy and move assignment operators
    AbstractRenderCommand& operator=(const AbstractRenderCommand& p_copy);
    AbstractRenderCommand& operator=(AbstractRenderCommand&& p_move);

    // Accessors
    GLuint GetPrimitiveCount() const;
    GLuint GetFirstIndex() const;
    GLuint GetBaseVertex() const;
    FaceCullMode GetFaceCullMode() const;
    TextureSet GetTextureSet() const;
    DepthFunc GetDepthFunc() const;
    DepthMask GetDepthMask() const;
    StencilFunc GetStencilFunc() const;
    StencilOp GetStencilOp() const;
    ColorMask GetColorMask() const;

    // Mutators, these are for special cases only
    void SetObjectUniforms(AssetShader_Object p_object_uniforms);

    QPointer <ProgramHandle> GetShaderRef();
    void SetShader(QPointer <ProgramHandle> p_shader);

    inline PrimitiveType GetPrimitiveType() const
    {
        return m_primitive_type;
    }

    inline GLuint GetBaseInstance() const
    {
        return m_base_instance;
    }

    inline QPointer <MeshHandle> GetMeshHandle() const
    {
        return m_mesh_handle;
    }

    inline QPointer <ProgramHandle> GetShader() const
    {
        return m_shader;
    }

    inline AssetShader_Frame GetFrameUniforms() const
    {
        return m_frame_uniforms;
    }

    inline AssetShader_Frame const * GetFrameUniformsPointer() const
    {
        return &m_frame_uniforms;
    }

    inline AssetShader_Room GetRoomUniforms() const
    {
        return m_room_uniforms;
    }

    inline AssetShader_Room const * GetRoomUniformsPointer() const
    {
        return &m_room_uniforms;
    }

    inline AssetShader_Object GetObjectUniforms() const
    {
        return m_object_uniforms;
    }

    inline AssetShader_Object const * GetObjectUniformsPointer() const
    {
        return &m_object_uniforms;
    }

    inline AssetShader_Object & GetObjectUniformsReference()
    {
        return m_object_uniforms;
    }

    inline AssetShader_Material GetMaterialUniforms() const
    {
        return m_material_uniforms;
    }

    inline AssetShader_Material const * GetMaterialUniformsPointer() const
    {
        return &m_material_uniforms;
    }

    void SetDepthMask(DepthMask p_depth_mask)
    {
        m_depth_mask = p_depth_mask;
    }

    TextureHandle::ALPHA_TYPE GetAlphaType() const
    {
        return (m_material_uniforms.iUseTexture[0] == 1.0f)
                ? m_texture_set.GetAlphaType(0)
                : TextureHandle::ALPHA_TYPE::NONE;
    }

    bool IsMaterialTransparent() const
    {
        if (m_material_uniforms.iUseTexture[0] == 1.0f && m_texture_set.GetHasAlpha(0))
        {
            return true;
        }

        if (m_material_uniforms.iDiffuse.w() != 1.0f)
        {
            return true;
        }

        if (m_object_uniforms.iConstColour[3] != 1.0f)
        {
            return true;
        }

        if (m_object_uniforms.iChromaKeyColour[3] != 0.0f)
        {
            return true;
        }

        return false;
    }

    uint32_t m_draw_id;
    uint32_t m_camera_id;

    QPointer <MeshHandle> m_mesh_handle;

private:

    // Renderer is made a friend class so that it can mutate an AbstractRenderCommand
    // when needed for special cases such as mirrors, without the need of constructing another AbstractRenderCommand.
    friend class Renderer;
    friend class AbstractRenderer;
    friend class AbstractRenderCommand_sort;

    // Data required for MultiDrawIndirect
    PrimitiveType m_primitive_type;
    GLuint m_primitive_count;
    GLuint m_first_index;
    GLuint m_base_vertex;
    GLuint m_base_instance;

    // Generic OpenGL data
    QPointer <ProgramHandle> m_shader;
    TextureSet m_texture_set;
    AssetShader_Frame m_frame_uniforms;
    AssetShader_Room m_room_uniforms;
    AssetShader_Object m_object_uniforms;
    AssetShader_Material m_material_uniforms;
    FaceCullMode m_active_face_cull_mode;
    DepthFunc m_depth_func;
    DepthMask m_depth_mask;
    StencilFunc m_stencil_func;
    StencilOp m_stencil_op;
    ColorMask m_color_mask;
};

class AbstractRenderCommand_sort
{
public:

    AbstractRenderCommand_sort()
        : m_stencil_ref_value(0),
            m_draw_layer(0),
            m_room_space_distance(0.0f),
            m_draw_id(0),
            m_camera_id(0),
            m_original_index(0)
    {

    }
    AbstractRenderCommand_sort(AbstractRenderCommand const & p_copy, const int p_index)
        : m_stencil_ref_value(p_copy.m_stencil_func.GetStencilReferenceValue()),
          m_draw_layer(p_copy.m_object_uniforms.m_draw_layer),
          m_room_space_distance(p_copy.m_object_uniforms.m_room_space_position_and_distance.w()),
          m_draw_id(p_copy.m_draw_id),
          m_camera_id(p_copy.m_camera_id),
          m_original_index(p_index)
    {

    }

    ~AbstractRenderCommand_sort()
    {

    }

    // Data to enable sorting
    uint32_t m_stencil_ref_value;
    int32_t m_draw_layer;
    float m_room_space_distance;
    uint32_t m_draw_id;
    uint32_t m_camera_id;
    int m_original_index;
};

class VirtualCamera
{
public:
    VirtualCamera();
    VirtualCamera(QMatrix4x4 p_viewMatrix, QVector4D p_viewport
        , float p_aspectRatio, float p_fov, float p_nearClip, float p_farClip);

    VirtualCamera(QMatrix4x4 p_viewMatrix, QVector4D p_viewport
        , QMatrix4x4 p_projectionMatrix);

    VirtualCamera(QVector3D p_position, QQuaternion p_orientation, QVector3D p_scale, QVector4D p_viewport
        , float p_aspectRatio, float p_fov, float p_nearClip, float p_farClip);
    ~VirtualCamera();

    QVector3D GetPosition() const;
    void SetPosition(QVector3D p_position);

    QQuaternion GetOrientation() const;
    void SetOrientation(QQuaternion p_orientation);

    QVector3D GetScale() const;
    void SetScale(QVector3D p_scale);

    float GetFOV() const;
    void SetFOV(float p_fov);

    float GetAspectRatio() const;
    void SetAspectRatio(float p_fov);

    float GetNearClip() const;
    void SetNearClip(float p_nearClip);

    float GetFarClip() const;
    void SetFarClip(float p_farClip);

    QVector4D GetViewport() const;
    void SetViewport(QVector4D p_viewport);

    const QMatrix4x4& GetViewMatrix() const;

    const QMatrix4x4& GetProjectionMatrix() const;

    bool GetScopeMask(RENDERER::RENDER_SCOPE const p_scope) const;
    void SetScopeMask(RENDERER::RENDER_SCOPE const p_scope, bool const p_mask);

    bool GetLeftEye() const;
    void SetLeftEye(bool p_is_left_eye);

    static bool m_enhanced_depth_precision_used;
    static bool m_enhanced_depth_precision_supported;

private:

    void Initialize();
    void RecomputeViewMatrix();
    void RecomputeProjectionMatrix();

    QVector3D m_position;
    QQuaternion m_orientation;
    QVector3D m_scale;
    float m_fov;
    float m_aspectRatio;
    float m_nearClip;
    float m_farClip;
    QVector4D m_viewport;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_projectionMatrix;
    bool m_scope_mask[static_cast<int>(RENDERER::RENDER_SCOPE::SCOPE_COUNT)];
    bool m_is_left_eye;
};

class AssetImageData;
class MeshHandle;

class GeomVBOData
{

public:
    GeomVBOData() :
        use_skelanim(false),
        m_index_count(0)
    {

    }

    ~GeomVBOData()
    {

    }

    int GetInstanceCount() const
    {
        return m_instance_transforms.size();
    }

    int GetIndexCount() const
    {
        return m_index_count;
    }

    void ClearVertexData()
    {
        m_index_count = m_indices.size();
        m_indices.clear();
        m_positions.clear();
        m_normals.clear();
        m_tex_coords.clear();
        m_colors.clear();
        m_skel_anim_indices.clear();
        m_skel_anim_weights.clear();
    }

    QVector <QMatrix4x4> m_instance_transforms;
    QVector <uint32_t> m_indices;
    QVector <float> m_positions;
    QVector <float> m_normals;
    QVector <float> m_tex_coords;
    QVector <float> m_colors;
    QVector <uint8_t> m_skel_anim_indices;
    QVector <float> m_skel_anim_weights;

    QPointer<MeshHandle> m_mesh_handle;
    bool use_skelanim;
    int m_index_count;
};

class AbstractRenderComandShaderData
{
public:
    AbstractRenderComandShaderData(ProgramHandle* p_program,
                                   AssetShader_Frame p_frame,
                                   AssetShader_Room p_room,
                                   AssetShader_Object p_object,
                                   AssetShader_Material p_material)
        : m_program(p_program),
          m_frame(p_frame),
          m_room(p_room),
          m_object(p_object),
          m_material(p_material)
    {

    }

    ProgramHandle* m_program;
    AssetShader_Frame m_frame;
    AssetShader_Room m_room;
    AssetShader_Object m_object;
    AssetShader_Material m_material;
};

enum ShaderUniformEnum : uint8_t
{
    iLeftEye = 0,
    iResolution = 1,
    iGlobalTime = 2,
    iViewportCount = 3,

    iMiscRoomData = 4,
    iPlayerPosition = 5,
    iUseClipPlane = 6,
    iClipPlane = 7,
    iFogEnabled = 8,
    iFogMode = 9,
    iFogDensity = 10,
    iFogStart = 11,
    iFogEnd = 12,
    iFogCol = 13,
    iRoomMatrix = 14,

    iAmbient = 15,
    iDiffuse = 16,
    iSpecular = 17,
    iShininess = 18,
    iEmission = 19,
    iLightmapScale = 20,
    iTiling = 21,
    iUseTexture = 22,

    iMiscObjectData = 23,
    iConstColour = 24,
    iChromaKeyColour = 25,
    iUseFlags = 26,
    iModelMatrix = 27,
    iViewMatrix = 28,
    iProjectionMatrix = 29,
    iInverseViewMatrix = 30,
    iModelViewMatrix = 31,
    iModelViewProjectionMatrix = 32,
    iTransposeInverseModelMatrix = 33,
    iTransposeInverseModelViewMatrix = 34,
    iSkelAnimJoints = 35,

    iTexture0 = 36,
    iTexture1 = 37,
    iTexture2 = 38,
    iTexture3 = 39,
    iTexture4 = 40,
    iTexture5 = 41,
    iTexture6 = 42,
    iTexture7 = 43,
    iTexture8 = 44,
    iTexture9 = 45,

    iCubeTexture0 = 46,
    iCubeTexture1 = 47,
    iCubeTexture2 = 48,
    iCubeTexture3 = 49,
    count = 50,
};


class PerObjectSSBOData
{
public:
    PerObjectSSBOData()
    {

    }
    ~PerObjectSSBOData()
    {

    }

    bool operator==(PerObjectSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerObjectSSBOData)) == 0);
    }

    bool operator!=(PerObjectSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerObjectSSBOData)) != 0);
    }

    float m_constColour[4];
    float m_chromaKeyColour[4];
    float m_useFlags[4];
};

class PerMaterialSSBOData
{
public:
    PerMaterialSSBOData()
    {

    }
    ~PerMaterialSSBOData()
    {

    }

    bool operator==(PerMaterialSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerMaterialSSBOData)) == 0);
    }

    bool operator!=(PerMaterialSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerMaterialSSBOData)) != 0);
    }

    float m_ambient[4];
    float m_diffuse[4];
    float m_specular[4];
    float m_shininess[4];
    float m_emission[4];
    float m_tiling[4];
    float m_lightmapScale[4];
    float m_useTexture[16];
};

class PerInstanceSSBOData
{
public:
    PerInstanceSSBOData()
    {

    }
    ~PerInstanceSSBOData()
    {

    }

    int32_t m_obj_cam_mat_indices[4];
    float m_modelMatrix[16];
    float m_transposeInverseModelMatrix[16];
    float m_modelViewMatrix[16];
    float m_modelViewprojectionMatrix[16];
    float m_transposeInverseModelViewMatrix[16];
};

class PerCameraSSBOData
{
public:
    PerCameraSSBOData()
    {

    }
    ~PerCameraSSBOData()
    {

    }

    bool operator==(PerCameraSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerCameraSSBOData)) == 0);
    }

    bool operator!=(PerCameraSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerCameraSSBOData)) != 0);
    }

    float m_viewmatrix[16];
    float m_projectionMatrix[16];
    float m_inverseViewMatrix[16];
};

/* flags indicating which fields in an rgbe_header_info are valid */
#define RGBE_VALID_PROGRAMTYPE 0x01
#define RGBE_VALID_GAMMA       0x02
#define RGBE_VALID_EXPOSURE    0x04

#define RGBE_DATA_RED    0
#define RGBE_DATA_GREEN  1
#define RGBE_DATA_BLUE   2
#define RGBE_DATA_SIZE   3

struct rgbe_header_info {
  int valid;            /* indicate which fields are valid */
  char programtype[16]; /* listed at beginning of file to identify it
                         * after "#?".  defaults to "RGBE" */
  float gamma;          /* image has already been gamma corrected with
                         * given gamma.  defaults to 1.0 (no correction) */
  float exposure;       /* a value of 1.0 in an image corresponds to
             * <exposure> watts/steradian/m^2.
             * defaults to 1.0 */
};

enum rgbe_error_codes {
  rgbe_read_error,
  rgbe_write_error,
  rgbe_format_error,
  rgbe_memory_error,
};

struct Triangle3D
{
    QVector3D p[3];
};

struct Sphere3D
{
    QVector3D cent;
    float rad;

    bool isPointInside(const QVector3D & pt) const {
        float dist = (pt - cent).lengthSquared();
        return (dist < (rad * rad)) ? true : false;
    }

};

struct Plane3D
{
    Plane3D() { };
    Plane3D(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) { };

    void fromPoints(const QVector3D &_p0, const QVector3D &_p1, const QVector3D &_p2)
    {
        QVector3D v0(_p0 - _p1);
        QVector3D v1(_p2 - _p1);
        QVector3D n = QVector3D::crossProduct(v1, v0).normalized();
        a = n.x();
        b = n.y();
        c = n.z();
        //d = -(_p0.x() * a + _p0.y() * b + _p0.z() * c);
        d = -QVector3D::dotProduct(_p0, n);
    }

    void fromPointAndNormal(const QVector3D &_p, const QVector3D &_n)
    {
        QVector3D nn = _n.normalized();
        a = nn.x();
        b = nn.y();
        c = nn.z();        
        //d = -(_p.x() * a + _p.y() * b + _p.z() * c);
        d = -QVector3D::dotProduct(_p, nn);
    }

    float dot(const QVector3D & _p) const
    {
        return a * _p.x() + b * _p.y() + c * _p.z();
    }

    float dist(const QVector3D & _p) const
    {
        return a * _p.x() + b * _p.y() + c * _p.z() + d;
    }

    QVector3D reflect(const QVector3D &_vec)
    {
        float d = dist(_vec);
        return _vec + QVector3D(-a, -b, -c) * d * 2.0f;
    }

    QVector3D project(const QVector3D &_p)
    {
        float h = dist(_p);
        return QVector3D(_p.x() - a * h,
                     _p.y() - b * h,
                     _p.z() - c * h);
    }

    bool isOnPlane(const QVector3D &_p, float _threshold = 0.001f)
    {
        float d = dist(_p);
        if (d < _threshold && d > -_threshold) {
            return true;
        }
        return false;
    }

    // Calcul the intersection between this plane and a line
    // If the plane and the line are parallel, OZFALSE is returned
    bool intersectWithLine(const QVector3D &_p0, const QVector3D &_p1, float &_t)
    {
        QVector3D dir = _p1 - _p0;
        float div = dot(dir);
        if (div == 0) {
            return false;
        }

        _t = -dist(_p0) / div;
        return true;
    }

    float a, b, c, d;
};

enum ElementType {
    TYPE_GHOST,
    TYPE_IMAGE,
    TYPE_LIGHT,
    TYPE_LINK,
    TYPE_OBJECT,
    TYPE_PARAGRAPH,
    TYPE_PARTICLE,
    TYPE_SOUND,
    TYPE_TEXT,
    TYPE_VIDEO,
    TYPE_ROOM,
    TYPE_PLAYER,
    TYPE_ASSET,
    TYPE_ASSETGHOST,
    TYPE_ASSETIMAGE,
    TYPE_ASSETOBJECT,
    TYPE_ASSETRECORDING,
    TYPE_ASSETSCRIPT,
    TYPE_ASSETSHADER,
    TYPE_ASSETSOUND,
    TYPE_ASSETVIDEO,
    TYPE_ASSETWEBSURFACE,
    TYPE_ERROR
};

class MathUtil
{
public:

    MathUtil();
    ~MathUtil();
	
	static void Initialize();
	static bool InitializeGLContext();    
    static void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam);

    static float DegToRad(float f);
    static float RadToDeg(float f);

    static void CropPixelData(QByteArray source, QByteArray destination, QSize size, uchar pixelSize, QRect nsize);
    static QByteArray ScalePixelData(QByteArray data, QSize size, uchar pixelSize, QSize nsize);
    static QByteArray ScaleToWidth(QByteArray data, QSize size, uchar pixelSize, int nwidth, QSize* outSize);
    static QByteArray ScaleToHeight(QByteArray data, QSize size, uchar pixelSize, int nheight, QSize* outSize);

    static QOpenGLTexture::TextureFormat GetTextureFormat(uchar pixelSize);
    static void GetGLFormat(uchar pixelSize, GLenum* format, GLenum* type);

    static bool GetRayTriIntersect(const QVector3D & rayp, const QVector3D & rayd, const QVector3D & p0, const QVector3D & p1, const QVector3D & p2, QVector3D & ipt);
    static bool LinePlaneIntersection(const QVector3D & p0, const QVector3D & n, const QVector3D & l0, const QVector3D & l1, QVector3D & intersect);
    static bool testIntersectionLineLine(const QVector2D &_p1, const QVector2D &_p2, const QVector2D &_p3, const QVector2D &_p4, float & _t);
    static bool testIntersectionSphereLine(const Sphere3D &_sphere, const QVector3D &_pt0, const QVector3D &_pt1, int & _nbInter, float & _inter1, float & _inter2);
    static bool testIntersectionTriSphere(const Triangle3D & tri, const QVector3D &_triNormal, const Sphere3D &_sphere, const QVector3D &_sphereVel, float & _distTravel, QVector3D & _reaction);
    static bool GetConvexIntersection(const QVector <QVector3D> & s1_pts, const QVector <QVector3D> & s1_normals, const QVector <QVector3D> & s2_pts, const QVector <QVector3D> & s2_normals);
    static void GetConvexIntersection_SATtest(const QVector3D & axis, const QVector <QVector3D> & ptSet, float & minAlong, float & maxAlong);
    static bool GetConvexIntersection_Overlaps(const float min1, const float max1, const float min2, const float max2);

    static void ComputeBarycentric3D(const QVector3D p, const QVector3D a, const QVector3D b, const QVector3D c, float &u, float &v, float &w);

    static void FacePosDirsGL(const QVector3D & pos, const QVector3D & xdir, const QVector3D & ydir, const QVector3D & zdir);    

    static float GetAngleBetweenRadians(const QVector3D & v1, const QVector3D & v2);
    static float GetSignedAngleBetweenRadians(const QVector3D & v1, const QVector3D & v2);
    static QVector3D GetRotatedAxis(const float anglerad, const QVector3D & vec, const QVector3D & axis);

    static void SphereToCartesian(const float thetadeg, const float phideg, const float r, QVector3D & p);
    static void CartesianToSphere(const QVector3D & p, float & thetadeg, float & phideg, float & r);

    static void NormSphereToCartesian(const float thetadeg, const float phideg, QVector3D & p);
    static void NormCartesianToSphere(const QVector3D & p, float & thetadeg, float & phideg);

    static QVector3D Slerp(QVector3D p1, QVector3D p2, float i);
    static float CosInterp(const float p1, const float p2, const float i);
    static QVector3D CosInterp(QVector3D p1, QVector3D p2, float i);

    static QVector3D GetNormalColour(const QVector3D & n);


    static float GetVectorComponent(const QVector3D & v, const int i);
    static float distancePointToLine(const QVector3D &_point, const QVector3D &_pt0, const QVector3D &_pt1, QVector3D & _linePt);    

    static QVector3D GetOrthoVec(const QVector3D & v);

    static void ErrorLog(const QString line);
    static QStringList GetErrorLogTemp();
    static void ClearErrorLogTemp();

    static void FlushErrorLog();

    static QString GetCurrentDateTimeAsString();

    static QVector3D huecycle(double val);

    static QString GetNumber(const float f);

    static QString GetBoolAsString(const bool b);
    static QString GetStringAsString(const QString & s);
    static QString GetFloatAsString(const float f);
    static QString GetIntAsString(const int i);
    static QString GetVectorAsString(const QVector3D & v, const bool add_quotes = true);
    static QString GetVector4AsString(const QVector4D & v, const bool add_quotes = true);
    static QString GetColourAsString(const QColor & c, const bool add_quotes = true);
    static QString GetRectangleAsString(const QRectF & r, const bool add_quotes = true);
    static QString GetEnumAsString(const GLenum e, const bool add_quotes = true);
    static QString GetRectAsString(const QRectF & r, const bool add_quotes = true);
    static QString GetAABBAsString(const QPair <QVector3D, QVector3D> & v, const bool add_quotes = true);

    static bool GetStringAsBool(const QString & s);
	static QVector3D GetStringAsVector(const QString & s);    
	static QVector4D GetStringAsVector4(const QString & s);
    static QColor GetVector4AsColour(const QVector4D v);
    static QVector4D GetColourAsVector4(const QColor c);
    static QColor GetStringAsColour(const QString & s);
    static QVector3D GetStringAsDoubleVector(const QString & s);
    static QRectF GetStringAsRect(const QString & s);
    static QPair <QVector3D, QVector3D> GetStringAsAABB(const QString & s);

    static QVector3D GetVectorFromQVariant(const QVariant v);
    static QVector4D GetVector4FromQVariant(const QVariant v);
    static QVector4D GetColourFromQVariant(const QVariant v);

    static QString GetTranslatorPath();
    static QString GetApplicationPath();
    static QString GetApplicationURL();

    static QString GetPath_Util(const QString subdir);
    static QString GetCachePath();
    static QString GetWorkspacePath();
    static QString GetScreenshotPath();
    static QString GetRecordingPath();
    static QString GetAppDataPath();   

    static QByteArray Decompress(const QByteArray & compressData);

    static inline QVector3D GetRandomValue(const QVector3D & v) {
        const float fx = float(qrand() % 10000) / 10000.0f;
        const float fy = float(qrand() % 10000) / 10000.0f;
        const float fz = float(qrand() % 10000) / 10000.0f;
        return QVector3D(v.x() * fx, v.y() * fy, v.z() * fz);
    }

    static QString MD5Hash(const QString & s);
    static QString EncodeString(const QString & s);
    static QString DecodeString(const QString & s);

    static bool rgbe_error(const int rgbe_error_code, const QString & msg);
    static bool RGBE_ReadHeader(QBuffer & buffer, int *width, int *height, rgbe_header_info *info);
    static bool RGBE_ReadPixels(QBuffer & buffer, float *data, int numpixels);
    static bool RGBE_ReadPixels_RLE(QBuffer & buffer, float *data, int scanline_width, int num_scanlines);

    static void PushModelMatrix();
    static void PopModelMatrix();
    static void LoadModelIdentity();
    static void LoadModelMatrix(const QMatrix4x4 m);
    static void MultModelMatrix(const QMatrix4x4 m);
    static QMatrix4x4 & ModelMatrix();

    static void LoadRoomMatrix(const QMatrix4x4 m);
    static QMatrix4x4 & RoomMatrix();

    static void LoadProjectionIdentity();
    static void LoadProjectionMatrix(const QMatrix4x4 m);
    static QMatrix4x4 & ProjectionMatrix();

    static void LoadViewIdentity();
    static void LoadViewMatrix(const QMatrix4x4 m);
    static QMatrix4x4 & ViewMatrix();

    static QMatrix4x4 InterpolateMatrices(const QMatrix4x4 & m0, const QMatrix4x4 & m1, const float t);

    static QMatrix4x4 getCurrentModelViewProjectionMatrix();

    static void MatrixToEulerAngles(const QMatrix4x4 & m, float & heading, float & attitude, float & bank);
    static void EulerAnglesToMatrix(const float heading, const float attitude, const float bank, QMatrix4x4 & m);   

    static void UnProject(float x, float y, float z, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble * new_x, GLdouble * new_y, GLdouble * new_z);

    static QString StripOutFilename(const QString s);

    static QOpenGLTexture * CreateTextureGL(QImage & img, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp);
    static QOpenGLTexture * CreateTextureGL(const QSize s, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp);   

    static float GetSoundLevel(const QByteArray & b);

    static QMatrix4x4 GetRotationMatrixFromEuler(const QVector3D rotation, const QString rotation_order);

    static QVariantList & GetPartyModeData();

    static quint64 hash(const QString & str);

    static QString GetSaveTimestampFilename();

    static ElementType AssetTypeFromFilename(const QString filename);

    static QByteArray LoadAssetFile(const QString path);

    /* standard conversion from rgbe to float pixels */
    /* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
    /*       in the range [0,1] to map back into the range [0,1].            */
    static inline void rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4]) {
        float f;
        if (rgbe[3]) {   /*nonzero pixel*/
            f = ldexp(1.0,rgbe[3]-(int)(128+8));
            *red = rgbe[0] * f;
            *green = rgbe[1] * f;
            *blue = rgbe[2] * f;
        }
        else {
            *red = *green = *blue = 0.0;
        }
    }

    static QStringList img_extensions;
    static QStringList sound_extensions;
    static QStringList vid_extensions;
    static QStringList geom_extensions;
    static QStringList domain_extensions;

    static float _180_OVER_PI;
    static float _PI_OVER_180;
    static float _PI;
    static float _2_PI;
    static float _PI_OVER_2;
    static float _PI_OVER_4;

    static QOpenGLExtraFunctions * glFuncs; 

    static QList <QMatrix4x4> modelmatrix_stack;
    static QMatrix4x4 viewmatrix;
    static QMatrix4x4 projectionmatrix;
    static QMatrix4x4 m_roomMatrix;

    static QByteArray loadFile(const QString fname);
    static void printShaderError(const GLuint p_shader);
    static bool loadGLShaderFromFile(GLuint* const p_program, const QString vertName, const QString fragName);        
    static bool m_linear_framebuffer;
    static bool m_do_equi;
    static QString m_last_screenshot_path;
    static QString GetLastScreenshotPath();
    static void SetLastScreenshotPath(QString p_last_screenshot_path);

    static QString room_delete_code;   

private:

    static QStringList error_log_msgs;
    static QStringList error_log_msgs_temp;
    static QVariantList partymode_data;

};



#endif // MATHUTIL_H
