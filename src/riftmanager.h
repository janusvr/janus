#ifndef RIFTRENDERER_H
#define RIFTRENDERER_H

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include "OVR_Platform.h"

#include "mathutil.h"
#include "rendererinterface.h"
#include "abstracthmdmanager.h"

struct TextureBuffer
{        
    ovrSession          Session;
    ovrTextureSwapChain  TextureChain;
    ovrSizei               texSize;

    TextureBuffer(ovrSession session, ovrSizei size) :
        Session(session),
        TextureChain(nullptr)
    {
        texSize = size;
        texSize.w *= 2;

//        ovrResult result = ovr_CreateSwapTextureSetGL(hmd, GL_RBGA8, size.w, size.h, &TextureSet);
        ovrTextureSwapChainDesc desc = {};
                    desc.Type = ovrTexture_2D;
                    desc.ArraySize = 1;
                    desc.Width = texSize.w;
                    desc.Height = texSize.h;
                    desc.MipLevels = std::log2(std::max(texSize.w, texSize.h));
                    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
                    desc.SampleCount = 1;
                    desc.StaticImage = ovrFalse;
                    desc.MiscFlags = ovrTextureMisc_AutoGenerateMips;

        ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &TextureChain);

        int length = 0;
        ovr_GetTextureSwapChainLength(session, TextureChain, &length);

        if(OVR_SUCCESS(result))
        {
            for (int i=0; i<length; ++i) {
                /*GLuint chainTexId = 0;
                ovr_GetTextureSwapChainBufferGL(Session, TextureChain, i, &chainTexId);
                //RendererInterface::m_pimpl->BindTexture(0, chainTexId); // TODO Reenable Rift rendering by moving HMD manager into Renderer
                MathUtil::glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                MathUtil::glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                MathUtil::glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                MathUtil::glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
            }
        }
        else {
            qDebug() << "TextureBuffer::TextureBuffer() - Failed to create TextureChain!";
        }       
    }

    ~TextureBuffer()
    {
        if (TextureChain)
        {
            ovr_DestroyTextureSwapChain(Session, TextureChain);
            TextureChain = nullptr;
        }
    }

    void Commit()
    {
        if (TextureChain) {
            ovr_CommitTextureSwapChain(Session, TextureChain);
        }
    }

    ovrSizei GetSize() const
    {
        return texSize;
    }
};

class RiftManager : public AbstractHMDManager
{

public:

    RiftManager();

    bool Initialize();
	void PostPresent();
	void InitializeGL();

    bool GetEnabled() const;
    QSize GetTextureSize() const;
    float GetIPD() const;
    QString GetHMDString() const;
    QString GetHMDType() const;

    void EnterVR();
    void ExitVR();
    void Update();

    QMatrix4x4 GetHMDTransform() const;
    QMatrix4x4 GetControllerTransform(const int i) const;
    QMatrix4x4 GetLastControllerTransform(const int i) const;

    int GetNumControllers() const;    
    bool GetControllerTracked(const int i);

    QVector2D GetControllerThumbpad(const int i) const;
    bool GetControllerThumbpadTouched(const int i) const;
    bool GetControllerThumbpadPressed(const int i) const;

    QVector2D GetControllerStick(const int i) const;
    bool GetControllerStickTouched(const int i) const;
    bool GetControllerStickPressed(const int i) const;

    float GetControllerTrigger(const int i) const;
    float GetControllerGrip(const int i) const;
    bool GetControllerMenuPressed(const int i);

    bool GetControllerButtonAPressed() const;
    bool GetControllerButtonBPressed() const;
    bool GetControllerButtonXPressed() const;
    bool GetControllerButtonYPressed() const;

    bool GetControllerButtonATouched() const;
    bool GetControllerButtonBTouched() const;
    bool GetControllerButtonXTouched() const;
    bool GetControllerButtonYTouched() const;

    void BeginRenderEye(const int eye);
    void EndRenderEye(const int eye);

    void BeginRendering();
    void EndRendering();

    void ReCentre();   

    void TriggerHapticPulse(const int i, const int val);

    //OVR Platform
    void Platform_ProcessMessages();
    bool Platform_GetEntitled() const;
    bool Platform_GetShouldQuit() const;

private:

    QMatrix4x4 GetProjectionMatrix(const int eye, const bool p_is_avatar);
    QMatrix4x4 GetMatrixFromPose(const ovrPosef & pose, const bool flip_z);

    ovrSession Session;
    ovrHmdDesc hmdDesc;
    ovrTrackingState hmdState;
    ovrInputState inputState;
    ovrLayerEyeFov ld;
    double predicted_display_time;
    long long current_frame_index;
    bool using_touch[2];

    float rift_aspectratio;
    float rift_phifov;

    TextureBuffer * eyeRenderTexture[2];
    QOpenGLFramebufferObject * eye_FBOs[2]; //what gets passed to Rift SDK

    ovrSizei windowSize;
    ovrSizei idealTextureSize;

    ovrEyeRenderDesc EyeRenderDesc[2];
    ovrPosef EyeRenderPose[2];
    ovrPosef ViewOffset[2];

    QMatrix4x4 hmd_xform;
    QMatrix4x4 controller_xform[2];
    QMatrix4x4 last_controller_xform[2];

    bool entitled;
    bool should_quit;
    bool should_recentre;
};

#endif // RIFTRENDERER_H
