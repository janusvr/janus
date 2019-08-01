#include "riftmanager.h"

RiftManager::RiftManager()
{
    m_using_openVR = false;
    current_frame_index = 0;
    idealTextureSize.w = 640;
    idealTextureSize.h = 480;
    Session = NULL;

    eyeRenderTexture[0] = NULL;
    eyeRenderTexture[1] = NULL;
    eye_FBOs[0] = NULL;
    eye_FBOs[1] = NULL;   

    using_touch[0] = false;
    using_touch[1] = false;
    should_quit = false;
    should_recentre = false;

    entitled = true;

    m_eye_view_matrices.resize(2);
    m_eye_projection_matrices.resize(4);

#ifdef OCULUS_SUBMISSION_BUILD
    if (ovr_PlatformInitializeWindows(RIFT_ID) != ovrPlatformInitialize_Success) {
        qDebug() << "ovr_PlatformInitializeWindows failed!";
        entitled = false;
    }    
    ovrRequest req;
    req = ovr_Entitlement_GetIsViewerEntitled();    
#endif
}

QSize RiftManager::GetTextureSize() const
{
    return QSize(idealTextureSize.w, idealTextureSize.h);
}

bool RiftManager::Initialize()
{
    ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
    ovrResult result = ovr_Initialize(&initParams);
    if (!OVR_SUCCESS(result)) {
        ovrErrorInfo errorInfo;
        ovr_GetLastErrorInfo(&errorInfo);
        qDebug() << "ovr_Initialize() failed" << result << "" << errorInfo.ErrorString;

        return false;
    }
    else {
        qDebug() << "ovr_Initialize() success";
    }

    ovrGraphicsLuid luid;
    result = ovr_Create(&Session, &luid);
    if (!OVR_SUCCESS(result)) {
        ovrErrorInfo errorInfo;
        ovr_GetLastErrorInfo(&errorInfo);
        qDebug() << "ovr_Create() failed" << result<< "" << errorInfo.ErrorString;
        return false;
    }
    else {
        qDebug() << "ovr_Create() success";
    }

    hmdDesc = ovr_GetHmdDesc(Session);

    //ovr_ConfigureTracking not necessary with 0.8 SDK, in fact it causes rotation tracking to break for DK1
    return true;
}

void RiftManager::PostPresent()
{
}

void RiftManager::InitializeGL()
{    
    // Setup Window and Graphics
    // Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution

    // Make eye render buffers
    idealTextureSize = ovr_GetFovTextureSize(Session, ovrEyeType(0), hmdDesc.DefaultEyeFov[0], 1.0f);

    // Add 4 pixels of horizontal padding between eye views to prevent leaking between viewing during filtering
    idealTextureSize.w += 4;

    if (eyeRenderTexture[0] == NULL) {
        eyeRenderTexture[0] = new TextureBuffer(Session, idealTextureSize);
    }
    if (eye_FBOs[0] == NULL) {
        eye_FBOs[0] = new QOpenGLFramebufferObject(idealTextureSize.w * 2, idealTextureSize.h, QOpenGLFramebufferObjectFormat());
    }

    if (!eyeRenderTexture[0]->TextureChain) {
        qDebug() << "RiftManager::InitializeGL() Failed to create texture chain.";
        return;
    }
    else {
        qDebug() << "RiftManager::InitializeGL() Success creating texture chain.";
    }

    EyeRenderDesc[0] = ovr_GetRenderDesc(Session, ovrEye_Left, hmdDesc.DefaultEyeFov[ovrEye_Left]);
    EyeRenderDesc[1] = ovr_GetRenderDesc(Session, ovrEye_Right, hmdDesc.DefaultEyeFov[ovrEye_Right]);

    rift_phifov = MathUtil::RadToDeg(2.0f * atanf (hmdDesc.DefaultEyeFov[ovrEye_Left].UpTan));

    const ovrSizei fovsize = ovr_GetFovTextureSize(Session, ovrEye_Left, hmdDesc.DefaultEyeFov[ovrEye_Left], 1.0f);
    rift_aspectratio = float(fovsize.w) / float(fovsize.h);

    ld.Header.Type  = ovrLayerType_EyeFov;
    ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

    // Viewports are adjusted to leave 8 pixels of empty space between the 2 viewports
    m_eye_viewports.resize(2);
    m_eye_viewports[0] = QVector4D(0, 0, idealTextureSize.w - 4, idealTextureSize.h);
    m_eye_viewports[1] = QVector4D(idealTextureSize.w + 4, 0, idealTextureSize.w - 4, idealTextureSize.h);
}

QString RiftManager::GetHMDString() const
{
    if (Session) {
        return QString(hmdDesc.ProductName);
    }
    else {
        return "Desktop";
    }
}

QString RiftManager::GetHMDType() const
{
    return QString("rift");
}

bool RiftManager::GetEnabled() const
{
    return (Session != NULL);
}

float RiftManager::GetIPD() const
{
    return fabsf(EyeRenderDesc[0].HmdToEyePose.Position.x) * 2.0f;
}

void RiftManager::EnterVR()
{

}

void RiftManager::ExitVR()
{

}

void RiftManager::Update()
{
    // Get eye poses, feeding in correct IPD offset
    ViewOffset[0] = EyeRenderDesc[0].HmdToEyePose; //.HmdToEyeViewOffset;
    ViewOffset[1] = EyeRenderDesc[1].HmdToEyePose; //.HmdToEyeViewOffset;

    current_frame_index = RendererInterface::m_pimpl->GetFrameCounter();
    ovrResult result = ovr_WaitToBeginFrame(Session, current_frame_index);
    if (!OVR_SUCCESS(result)) {
        ///     Returns an ovrResult for which OVR_SUCCESS(result) is false upon error and true
        ///         upon success. Return values include but aren't limited to:
        ///     - ovrSuccess: command completed successfully.
        ///     - ovrError_DisplayLost: The session has become invalid (such as due to a device removal)
        ///       and the shared resources need to be released (ovr_DestroyTextureSwapChain), the session
        ///       needs to destroyed (ovr_Destroy) and recreated (ovr_Create), and new resources need to be
        ///       created (ovr_CreateTextureSwapChainXXX). The application's existing private graphics
        ///       resources do not need to be recreated unless the new ovr_Create call returns a different
        ///       GraphicsLuid.
        qDebug() << "PROBLEM ovr_WaitToBeginFrame" << result;
    }
    predicted_display_time = ovr_GetPredictedDisplayTime(Session, current_frame_index);
    hmdState = ovr_GetTrackingState(Session, predicted_display_time, ovrTrue);
    ovr_CalcEyePoses(hmdState.HeadPose.ThePose, ViewOffset, EyeRenderPose);

    //update HMD matrix
    hmd_xform = GetMatrixFromPose(hmdState.HeadPose.ThePose, true);
    hmd_xform.scale(1,1,-1);

    // m_eye_view_matrices contains the compelte transform from tracking origin to each eye.
    // This is what we use when rendering, hmd_xform is used purely for positiong the player capsule
    m_eye_view_matrices[0] = GetMatrixFromPose(EyeRenderPose[0], false).inverted();
    m_eye_view_matrices[1] = GetMatrixFromPose(EyeRenderPose[1], false).inverted();

    GetProjectionMatrix(0, true);
    GetProjectionMatrix(1, true);
    GetProjectionMatrix(0, false);
    GetProjectionMatrix(1, false);

    //query active controllers
    ovr_GetInputState(Session, ovrControllerType_Active, &inputState);
    using_touch[0] = ((inputState.ControllerType & ovrControllerType_LTouch) > 0);
    using_touch[1] = ((inputState.ControllerType & ovrControllerType_RTouch) > 0);

    //update controller matrices
    QMatrix4x4 m;
    m.translate(0,1.6f,0);
    last_controller_xform[0] = controller_xform[0];
    last_controller_xform[1] = controller_xform[1];
    controller_xform[0] = (using_touch[0] ? (m * GetMatrixFromPose(hmdState.HandPoses[0].ThePose, false)) : QMatrix4x4());
    controller_xform[1] = (using_touch[1] ? (m * GetMatrixFromPose(hmdState.HandPoses[1].ThePose, false)) : QMatrix4x4());
//    qDebug() << "using touch" << using_touch[0] << using_touch[1] << controller_transform[0] << controller_transform[1];

    ovrSessionStatus sessionStatus;
    ovr_GetSessionStatus(Session, &sessionStatus);

    if (sessionStatus.ShouldQuit) {
        should_quit = true;
    }
    if (sessionStatus.ShouldRecenter) {
        this->ReCentre();
    }
}

QMatrix4x4 RiftManager::GetHMDTransform() const
{
    return hmd_xform;
}

QMatrix4x4 RiftManager::GetControllerTransform(const int i) const
{
    return controller_xform[i];
}

QMatrix4x4 RiftManager::GetLastControllerTransform(const int i) const
{
    return last_controller_xform[i];
}

int RiftManager::GetNumControllers() const
{
    int num=0;
    for (int i=0; i<2; ++i) {
        if (using_touch[i]) {
            ++num;
        }
    }    
    return num;
}

bool RiftManager::GetControllerTracked(const int i)
{
    return using_touch[i];
}

QVector2D RiftManager::GetControllerThumbpad(const int ) const
{
    return QVector2D();
}

bool RiftManager::GetControllerThumbpadTouched(const int ) const
{
    return false;
}

bool RiftManager::GetControllerThumbpadPressed(const int ) const
{
    return false;
}

QVector2D RiftManager::GetControllerStick(const int i) const
{
    return QVector2D(inputState.Thumbstick[i].x, inputState.Thumbstick[i].y);
}

bool RiftManager::GetControllerStickTouched(const int i) const
{
    unsigned int b = ((i == 0) ? (inputState.Touches & ovrTouch_LThumb) : (inputState.Touches & ovrTouch_RThumb));
    return (b > 0);
}

bool RiftManager::GetControllerStickPressed(const int i) const
{
    unsigned int b = ((i == 0) ? (inputState.Buttons & ovrButton_LThumb) : (inputState.Buttons & ovrButton_RThumb));
    return (b > 0);
}

float RiftManager::GetControllerTrigger(const int i) const
{
    return inputState.IndexTriggerNoDeadzone[i];
}

float RiftManager::GetControllerGrip(const int i) const
{
    return inputState.HandTrigger[i];
}

bool RiftManager::GetControllerMenuPressed(const int i)
{
    if (i == 0) {
        return ((inputState.Buttons & ovrButton_Enter) > 0);
    }
    else {
        return false;
    }
}

bool RiftManager::GetControllerButtonAPressed() const
{
    return ((inputState.Buttons & ovrButton_A) > 0);
}

bool RiftManager::GetControllerButtonBPressed() const
{
    return ((inputState.Buttons & ovrButton_B) > 0);
}

bool RiftManager::GetControllerButtonXPressed() const
{
    return ((inputState.Buttons & ovrButton_X) > 0);
}

bool RiftManager::GetControllerButtonYPressed() const
{
    return ((inputState.Buttons & ovrButton_Y) > 0);
}

bool RiftManager::GetControllerButtonATouched() const
{
    return ((inputState.Touches & ovrButton_A) > 0);
}

bool RiftManager::GetControllerButtonBTouched() const
{
    return ((inputState.Touches & ovrButton_B) > 0);
}

bool RiftManager::GetControllerButtonXTouched() const
{
    return ((inputState.Touches & ovrButton_X) > 0);
}

bool RiftManager::GetControllerButtonYTouched() const
{
    return ((inputState.Touches & ovrButton_Y) > 0);
}

QMatrix4x4 RiftManager::GetMatrixFromPose(const ovrPosef & pose, const bool flip_z)
{
    QVector3D x, y, z, p;
    x = QVector3D(1,0,0);
    y = QVector3D(0,1,0);
    z = QVector3D(0,0,flip_z ? -1 : 1);
    p = QVector3D(pose.Position.x, pose.Position.y, pose.Position.z);

    const ovrQuatf & rot = pose.Orientation;
    QQuaternion qt_quat = QQuaternion(rot.w, rot.x, rot.y, rot.z);
    x = qt_quat.rotatedVector(x);
    y = qt_quat.rotatedVector(y);
    z = qt_quat.rotatedVector(z);

    QMatrix4x4 m;
    m.setColumn(0, x);
    m.setColumn(1, y);
    m.setColumn(2, z);
    m.setColumn(3, p);
    m.setRow(3, QVector4D(0,0,0,1));
    return m;
}

void RiftManager::EndRenderEye(const int )
{
}

void RiftManager::BeginRendering()
{
    ovrResult result = ovr_BeginFrame(Session, current_frame_index);
    if (!OVR_SUCCESS(result)) {
        /// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error and true
        ///         upon success. Return values include but aren't limited to:
        ///     - ovrSuccess: rendering completed successfully.
        ///     - ovrError_DisplayLost: The session has become invalid (such as due to a device removal)
        ///       and the shared resources need to be released (ovr_DestroyTextureSwapChain), the session
        ///       needs to destroyed (ovr_Destroy) and recreated (ovr_Create), and new resources need to be
        ///       created (ovr_CreateTextureSwapChainXXX). The application's existing private graphics
        ///       resources do not need to be recreated unless the new ovr_Create call returns a different
        ///       GraphicsLuid.
        ///     - ovrError_TextureSwapChainInvalid: The ovrTextureSwapChain is in an incomplete or
        ///       inconsistent state. Ensure ovr_CommitTextureSwapChain was called at least once first.
        qDebug() << "PROBLEM ovr_BeginFrame" << result;
    }
}

QMatrix4x4 RiftManager::GetProjectionMatrix(const int eye, const bool p_is_avatar)
{
    ovrMatrix4f proj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye],
                                              (p_is_avatar) ? m_avatar_near_clip : m_near_clip,
                                              (p_is_avatar) ? m_avatar_far_clip : m_far_clip,
                                               (RendererInterface::m_pimpl->GetIsUsingEnhancedDepthPrecision() == true
                                                && RendererInterface::m_pimpl->GetIsEnhancedDepthPrecisionSupported() == true)
                                                ? (ovrProjection_FarLessThanNear)
                                                : (ovrProjection_ClipRangeOpenGL)
                                              );
    QMatrix4x4 m;
    for (int i=0; i<4; ++i) {
        for (int j=0; j<4; ++j) {
            m.data()[i+j*4] = proj.M[i][j];
        }
    }

    m_eye_projection_matrices[(p_is_avatar) ? eye + 2 : eye] = m;

    return m;
}

void RiftManager::BeginRenderEye(const int )
{
}

void RiftManager::EndRendering()
{
    if (Session == NULL || eyeRenderTexture[0] == NULL) {
        return;
    }
//    qDebug() << "RiftManager::EndRendering()";

    // Do distortion rendering, Present and flush/sync
    // Set up positional data.
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyePose[0] = ViewOffset[0];
    viewScaleDesc.HmdToEyePose[1] = ViewOffset[1];

    auto const width = idealTextureSize.w;
    auto const height = idealTextureSize.h;

    int out_Length;
    GLuint out_TexId = 0;
    ovrResult result = ovr_GetTextureSwapChainLength(Session, eyeRenderTexture[0]->TextureChain, &out_Length);
    if (!OVR_SUCCESS(result))
    {
        qDebug() << "PROBLEM ovr_GetTextureSwapChainLength";
    }

    if (eyeRenderTexture[0]->TextureChain && out_Length > 0)
    {
        int out_Index = 0;
        result = ovr_GetTextureSwapChainCurrentIndex(Session, eyeRenderTexture[0]->TextureChain, &out_Index);
        if (!OVR_SUCCESS(result))
        {
            qDebug() << "PROBLEM ovr_GetTextureSwapChainCurrentIndex";
        }

        result = ovr_GetTextureSwapChainBufferGL(Session, eyeRenderTexture[0]->TextureChain, out_Index, &out_TexId);
        if (!OVR_SUCCESS(result))
        {
            qDebug() << "PROBLEM ovr_GetTextureSwapChainBufferGL";
        }
    }

    MathUtil::glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eye_FBOs[0]->handle());
    MathUtil::glFuncs->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_TexId, 0);
    GLenum drawBuffers[1];
    drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    GLsizei drawBufferCount = 1;
    MathUtil::glFuncs->glDrawBuffers(drawBufferCount, &drawBuffers[0]);
    MathUtil::glFuncs->glBlitFramebuffer(0, 0, width * 2, height, 0, 0, width * 2, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    MathUtil::glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


    ld.ColorTexture[0] = eyeRenderTexture[0]->TextureChain;
    ld.ColorTexture[1] = 0;
    for (int eye = 0; eye < 2; ++eye)
    {
        QVector4D & current_viewport((eye == 0) ? m_eye_viewports[0] : m_eye_viewports[1]);
        ovrRecti rect;
        rect.Pos.x = int(current_viewport.x());
        rect.Pos.y = int(current_viewport.y());
        rect.Size.w = int(current_viewport.z());
        rect.Size.h = int(current_viewport.w());
        ld.Viewport[eye]     = rect;
        ld.Fov[eye]          = hmdDesc.DefaultEyeFov[eye];
        ld.RenderPose[eye]   = EyeRenderPose[eye];
    }
    eyeRenderTexture[0]->Commit();

    ovrLayerHeader* layers = &ld.Header;
    result = ovr_EndFrame(Session, current_frame_index, &viewScaleDesc, &layers, 1);
    // exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
    if (!OVR_SUCCESS(result)) {
        /// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error and true
        ///         upon success. Return values include but aren't limited to:
        ///     - ovrSuccess: rendering completed successfully.
        ///     - ovrSuccess_NotVisible: rendering completed successfully but was not displayed on the HMD,
        ///       usually because another application currently has ownership of the HMD. Applications
        ///       receiving this result should stop rendering new content, call ovr_GetSessionStatus
        ///       to detect visibility.
        ///     - ovrError_DisplayLost: The session has become invalid (such as due to a device removal)
        ///       and the shared resources need to be released (ovr_DestroyTextureSwapChain), the session
        ///       needs to destroyed (ovr_Destroy) and recreated (ovr_Create), and new resources need to be
        ///       created (ovr_CreateTextureSwapChainXXX). The application's existing private graphics
        ///       resources do not need to be recreated unless the new ovr_Create call returns a different
        ///       GraphicsLuid.
        ///     - ovrError_TextureSwapChainInvalid: The ovrTextureSwapChain is in an incomplete or
        ///       inconsistent state. Ensure ovr_CommitTextureSwapChain was called at least once first.
        qDebug() << "PROBLEM ovr_EndFrame" << result;
    }           
}

void RiftManager::ReCentre()
{
    ovr_RecenterTrackingOrigin(Session);
}

void RiftManager::TriggerHapticPulse(const int i, const int val)
{
//    ovr_SetControllerVibration(Session, (i == 0) ? ovrControllerType_LTouch : ovrControllerType_RTouch, 0.5f, float(val)/255.0f);
    ovrHapticsBuffer * buffer = new ovrHapticsBuffer;
    buffer->SamplesCount = 10;
    int * samples = new int[buffer->SamplesCount];
    for (int i=0; i<buffer->SamplesCount; ++i) {
        samples[i] = ((i % 2) == 0) ? val : -val;
    }
    buffer->Samples = (void *)samples;
    buffer->SubmitMode = ovrHapticsBufferSubmit_Enqueue;
    ovr_SubmitControllerVibration(Session, (i == 0) ? ovrControllerType_LTouch : ovrControllerType_RTouch, buffer);
}

void RiftManager::Platform_ProcessMessages()
{
#ifdef OCULUS_SUBMISSION_BUILD
    //OVR Platform SDK
    ovrMessage *message  = ovr_PopMessage();
    if (message  != nullptr) {
        switch (ovr_Message_GetType(message)) {
        case ovrMessage_Entitlement_GetIsViewerEntitled:
            if (!ovr_Message_IsError(message)) {
                // User is entitled.
                qDebug() << "OVRPlatform - Entitlement check succeeded";
//                entitled = false; //uncomment to test entitlement check
            }
            else {
                // User is NOT entitled.  Exit
                qDebug() << "OVRPlatform - Entitlement check failed";
                entitled = false;
            }
            break;

        default:
            break;

        }
    }
#endif
}

bool RiftManager::Platform_GetEntitled() const
{
    return entitled;
}

bool RiftManager::Platform_GetShouldQuit() const
{
    return should_quit;
}
