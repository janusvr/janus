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
//#include <glm/mat4x4.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include "rendererinterface.h"
class MeshHandle;
class BufferHandle;

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
    static uint64_t m_frame_limiter_render_thread;
    static bool m_linear_framebuffer;
    static bool m_do_equi;
    static QString m_last_screenshot_path;
    static QString GetLastScreenshotPath();
    static void SetLastScreenshotPath(QString p_last_screenshot_path);

    static QString room_delete_code;   

private:

    static bool use_render_doc;
    static QStringList error_log_msgs;
    static QStringList error_log_msgs_temp;
    static QVariantList partymode_data;

};



#endif // MATHUTIL_H
