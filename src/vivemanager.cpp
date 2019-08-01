#include "vivemanager.h"

ViveManager::ViveManager() :
    enabled(false),
    m_pHMD(NULL),
    m_pCompositor(NULL),
    m_iTrackedControllerCount(0),
    m_iTrackedControllerCount_Last(-1),
    m_iValidPoseCount(0),
    m_iValidPoseCount_Last(-1),
    m_strPoseClasses(""),    
    tracking_origin(vr::TrackingUniverseStanding)
{            
    m_using_openVR = true;
    memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));

    m_eye_view_matrices.resize(2);
    m_eye_projection_matrices.resize(4);

    controller_ind_left = vr::k_unMaxTrackedDeviceCount;
    controller_ind_right = vr::k_unMaxTrackedDeviceCount;
}

ViveManager::~ViveManager()
{
    Shutdown();
}

bool ViveManager::Initialize()
{
    qDebug() << "ViveManager::Initialize()";
    if (!vr::VR_IsHmdPresent()) {
        qDebug() << "ViveRenderer::Initialize() - VR_IsHmdPresent() returned false";
        return false;
    }

    //vr::HmdError peError = vr::HmdError_None;
    vr::EVRInitError peError = vr::EVRInitError::VRInitError_None;
    m_pHMD = vr::VR_Init(&peError, vr::EVRApplicationType::VRApplication_Scene);

    if (m_pHMD == NULL) {
        qDebug() << "ViveRenderer::Initialize() - VR_Init() error: " << vr::VR_GetVRInitErrorAsEnglishDescription(peError);
        return false;
    }

    const QString serial_number_string = QString(GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String));
    //GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ManufacturerName_String); //used to check this for "vive"
    if (serial_number_string.toLower() == "windowsholographic") {
        SetHMDString("wmxr");
    }
    else {
        SetHMDString("vive");
    }

//    qDebug() << "ViveRenderer::Initialize() - Captured input focus?" << m_pHMD->CaptureInputFocus();
    m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

    // Add 4 render target size per-eye so that we have 8 pixels of buffer between the viewports
    // This helps avoid filtering artifacts
    m_nRenderWidth += 4;
    viewport_width_modifier = (float(m_nRenderWidth - 4) / float(m_nRenderWidth));

    m_eye_viewports.resize(2);
    m_eye_viewports[0] = QVector4D(0, 0, m_nRenderWidth - 4, m_nRenderHeight);
    m_eye_viewports[1] = QVector4D(m_nRenderWidth + 4, 0, m_nRenderWidth - 4, m_nRenderHeight);

    enabled = true;

    depth_invert_matrix.setColumn(0, QVector4D(1, 0, 0, 0));
    depth_invert_matrix.setColumn(1, QVector4D(0, 1, 0, 0));
    depth_invert_matrix.setColumn(2, QVector4D(0, 0, -1, 0));
    depth_invert_matrix.setColumn(3, QVector4D(0, 0, 1, 1));

    return true;
}

void ViveManager::InitializeGL()
{
    if (!m_pHMD || !enabled) {
        return;
    }

    //vr::HmdError peError = vr::HmdError_None;
    vr::EVRInitError peError = vr::EVRInitError::VRInitError_None;
    m_pCompositor = (vr::IVRCompositor*)vr::VR_GetGenericInterface(vr::IVRCompositor_Version, &peError);

    if (peError != vr::EVRInitError::VRInitError_None || m_pCompositor == NULL) {
        m_pCompositor = NULL;
        qDebug() << "ViveRenderer::InitializeGL() - Compositor initialization failed, error: %s\n" << vr::VR_GetVRInitErrorAsEnglishDescription(peError);
        return;
    }
    else {
        qDebug() << "ViveRenderer::InitializeGL() - Compositor initialization successful!";
    }

    /*uint32_t unSize = m_pCompositor->GetLastError(NULL, 0);
    if (unSize > 1)
    {
        char* buffer = new char[unSize];
        m_pCompositor->GetLastError(buffer, unSize);
        qDebug() << "ViveRenderer::InitializeCompositor() - Compositor - " << buffer;
        delete [] buffer;
        return;
    }*/

    //currently targeting standing
    //m_pCompositor->SetTrackingSpace(vr::TrackingUniverseSeated);
    m_pCompositor->SetTrackingSpace(tracking_origin);
//    m_pCompositor->ShowMirrorWindow();
}

void ViveManager::Shutdown()
{
    vr::VR_Shutdown();
    m_pHMD = NULL;
}

bool ViveManager::GetEnabled() const
{
    return enabled;
}

void ViveManager::SetHMDString(const QString s)
{
    hmd_string = s;
}

QString ViveManager::GetHMDString() const
{
    return hmd_string;
}

QString ViveManager::GetHMDType() const
{
    return hmd_string;
}

QSize ViveManager::GetTextureSize() const
{
    return QSize(m_nRenderWidth, m_nRenderHeight);
}

void ViveManager::EnterVR()
{

}

void ViveManager::ExitVR()
{

}

void ViveManager::Update()
{
    if (!m_pHMD || !enabled) {
        return;
    }

    if (m_pCompositor) {
        m_pCompositor->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
    }

    m_iValidPoseCount = 0;
    m_strPoseClasses = "";

    controller_ind_left = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
    controller_ind_right = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

    for (unsigned int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice) {
        if (m_rTrackedDevicePose[nDevice].bPoseIsValid) {

            last_m_rmat4DevicePose[nDevice] = m_rmat4DevicePose[nDevice];

            m_iValidPoseCount++;
            m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToQMatrix4x4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
            if (m_rDevClassChar[nDevice]==0)
            {
                switch (m_pHMD->GetTrackedDeviceClass(nDevice))
                {
                case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
                case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
                case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
                case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
                default:                                       m_rDevClassChar[nDevice] = '?'; break;
                }
            }
            m_strPoseClasses += m_rDevClassChar[nDevice];
        }
    }   

    if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
    {
        m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];  //qDebug() << m_mat4HMDPose;

        // Update the two eye view matrices, these are the complete transform from tracking origin to each eye.
        // So in OpenVR we need to mulitply these by the head pose as they only contain the offset from the head.
        vr::HmdMatrix34_t left_eye_transform = m_pHMD->GetEyeToHeadTransform(vr::EVREye::Eye_Left);
        m_eye_view_matrices[vr::EVREye::Eye_Left] = ConvertSteamVRMatrixToQMatrix4x4(left_eye_transform).inverted();
        m_eye_view_matrices[vr::EVREye::Eye_Left] = m_eye_view_matrices[vr::EVREye::Eye_Left] * m_mat4HMDPose.inverted();

        vr::HmdMatrix34_t right_eye_transform = m_pHMD->GetEyeToHeadTransform(vr::EVREye::Eye_Right);
        m_eye_view_matrices[vr::EVREye::Eye_Right] = ConvertSteamVRMatrixToQMatrix4x4(right_eye_transform).inverted();
        m_eye_view_matrices[vr::EVREye::Eye_Right] = m_eye_view_matrices[vr::EVREye::Eye_Right] * m_mat4HMDPose.inverted();

        // Update our four projeciton matrix (left and right eye, normal and avatar clip planes)
        //48.0: openvr oddity: frustum bounds require premultiplication by n and negation of b and t
        float l, r, t, b;
        m_pHMD->GetProjectionRaw(vr::Eye_Left, &l, &r, &t, &b);

        QMatrix4x4 ident;
        ident.setToIdentity();
        ident.frustum(l * m_near_clip, r * m_near_clip, b * -m_near_clip, t * -m_near_clip, m_near_clip, m_far_clip);
        m_eye_projection_matrices[vr::EVREye::Eye_Left] = ident;
        m_eye_projection_matrices[vr::EVREye::Eye_Left].setColumn(2,m_eye_projection_matrices[vr::EVREye::Eye_Left].column(2) * QVector4D(1, -1, 1, 1));

        ident.setToIdentity();
        ident.frustum(l * m_avatar_near_clip, r * m_avatar_near_clip, b * -m_avatar_near_clip, t * -m_avatar_near_clip, m_avatar_near_clip, m_avatar_far_clip);
        m_eye_projection_matrices[vr::EVREye::Eye_Left + 2] = ident;
        m_eye_projection_matrices[vr::EVREye::Eye_Left + 2].setColumn(2,m_eye_projection_matrices[vr::EVREye::Eye_Left + 2].column(2) * QVector4D(1, -1, 1, 1));

        m_pHMD->GetProjectionRaw(vr::Eye_Right, &l, &r, &t, &b);

        ident.setToIdentity();
        ident.frustum(l * m_near_clip, r * m_near_clip, b * -m_near_clip, t * -m_near_clip, m_near_clip, m_far_clip);
        m_eye_projection_matrices[vr::EVREye::Eye_Right] = ident;
        m_eye_projection_matrices[vr::EVREye::Eye_Right].setColumn(2,m_eye_projection_matrices[vr::EVREye::Eye_Right].column(2) * QVector4D(1, -1, 1, 1));

        ident.setToIdentity();
        ident.frustum(l * m_avatar_near_clip, r * m_avatar_near_clip, -b * m_avatar_near_clip, -t * m_avatar_near_clip, m_avatar_near_clip, m_avatar_far_clip);
        m_eye_projection_matrices[vr::EVREye::Eye_Right + 2] = ident;
        m_eye_projection_matrices[vr::EVREye::Eye_Right + 2].setColumn(2,m_eye_projection_matrices[vr::EVREye::Eye_Right + 2].column(2) * QVector4D(1, -1, 1, 1));

        if (RendererInterface::m_pimpl->GetIsEnhancedDepthPrecisionSupported() && RendererInterface::m_pimpl->GetIsUsingEnhancedDepthPrecision())
        {
            m_eye_projection_matrices[vr::EVREye::Eye_Left]      = depth_invert_matrix * m_eye_projection_matrices[vr::EVREye::Eye_Left];
            m_eye_projection_matrices[vr::EVREye::Eye_Left + 2]  = depth_invert_matrix * m_eye_projection_matrices[vr::EVREye::Eye_Left + 2];
            m_eye_projection_matrices[vr::EVREye::Eye_Right]     = depth_invert_matrix * m_eye_projection_matrices[vr::EVREye::Eye_Right];
            m_eye_projection_matrices[vr::EVREye::Eye_Right + 2] = depth_invert_matrix * m_eye_projection_matrices[vr::EVREye::Eye_Right + 2];
        }
    }

    if (m_pHMD->IsInputFocusCapturedByAnotherProcess()) {
        //qDebug() << "ViveRenderer::UpdateEyePoses() - Input captured by another process!";
        return;
    }

    //update controller states    
    if (controller_ind_left < vr::k_unMaxTrackedDeviceCount) {
        m_pHMD->GetControllerStateWithPose(tracking_origin, controller_ind_left, &controller_states[0], sizeof(vr::VRControllerState_t), &controller_poses[0]);
        controller_poses[0] = m_rTrackedDevicePose[controller_ind_left];
    }
    if (controller_ind_right < vr::k_unMaxTrackedDeviceCount) {
        m_pHMD->GetControllerStateWithPose(tracking_origin, controller_ind_right, &controller_states[1], sizeof(vr::VRControllerState_t), &controller_poses[1]);
        controller_poses[1] = m_rTrackedDevicePose[controller_ind_right];
    }    
}

QMatrix4x4 ViveManager::ConvertSteamVRMatrixToQMatrix4x4(const vr::HmdMatrix34_t & matPose) const
{
    QMatrix4x4 matrixObj;
    matrixObj.setColumn(0, QVector4D(matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0f));
    matrixObj.setColumn(1, QVector4D(matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0f));
    matrixObj.setColumn(2, QVector4D(matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0f));
    matrixObj.setColumn(3, QVector4D(matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f));
    return matrixObj;
}

void ViveManager::ReCentre()
{
    if (m_pHMD) {
        m_pHMD->ResetSeatedZeroPose();
    }
}

void ViveManager::BeginRenderEye(const int )
{
}

void ViveManager::EndRenderEye(const int )
{

}

void ViveManager::BeginRendering()
{
    // TODO:: In here we should render the VR mask object to the near-plane
    // to depth-fail fragments that won'd be visible after the distortion is applied
}

void ViveManager::EndRendering()
{
    if (m_pHMD && m_pCompositor && enabled) {
        //This blits the multisampled framebuffer to the framebuffer which contains a texture for the OpenVR SDK
		// Bind nothing to the framebuffer so that we can access the textures of our FBO
        MathUtil::glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        vr::VRTextureBounds_t left_bounds;
        left_bounds.uMin = 0.0f;
        left_bounds.vMin = 0.0f;
        left_bounds.uMax = 0.5f * viewport_width_modifier;
        left_bounds.vMax = 1.0f;

        vr::Texture_t texture;
        texture.handle = (void*)(uintptr_t)m_color_texture_id;
        texture.eType = vr::ETextureType::TextureType_OpenGL;
        texture.eColorSpace = vr::EColorSpace::ColorSpace_Gamma;

        vr::EVRCompositorError error_result = vr::EVRCompositorError::VRCompositorError_None;
        error_result = m_pCompositor->Submit(vr::Eye_Left, &texture, &left_bounds);

        if (error_result != vr::EVRCompositorError::VRCompositorError_None)
        {
            qDebug() << "ViveRenderer::EndRendering() - Compositor Submit failed" << error_result;
            return;
        }        

        vr::VRTextureBounds_t right_bounds;
        right_bounds.uMin = 1.0f - 0.5f * viewport_width_modifier;
        right_bounds.vMin = 0.0f;
        right_bounds.uMax = 1.0f;
        right_bounds.vMax = 1.0f;

        error_result = vr::EVRCompositorError::VRCompositorError_None;
        error_result = m_pCompositor->Submit(vr::Eye_Right, &texture, &right_bounds);

        if (error_result != vr::EVRCompositorError::VRCompositorError_None && error_result != vr::EVRCompositorError::VRCompositorError_DoNotHaveFocus)
        {
            qDebug() << "ViveRenderer::EndRendering() - Compositor Submit failed";
            return;
        }
    }

    // Spew out the controller and pose count whenever they change.
    if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last ) {
        m_iValidPoseCount_Last = m_iValidPoseCount;
        m_iTrackedControllerCount_Last = m_iTrackedControllerCount;       
    }

}

void ViveManager::PostPresent()
{
	if (m_pCompositor)
	{
		m_pCompositor->PostPresentHandoff();
	}
}

QMatrix4x4 ViveManager::GetHMDTransform() const
{
    return m_mat4HMDPose;
}

QMatrix4x4 ViveManager::GetControllerTransform(const int i) const
{
    if (i == 0 && controller_ind_left < vr::k_unMaxTrackedDeviceCount) {
        return m_rmat4DevicePose[controller_ind_left];
    }
    else if (i == 1 && controller_ind_right < vr::k_unMaxTrackedDeviceCount) {
        return m_rmat4DevicePose[controller_ind_right];
    }
    return QMatrix4x4();
}

QMatrix4x4 ViveManager::GetLastControllerTransform(const int i) const
{
    if (i == 0 && controller_ind_left < vr::k_unMaxTrackedDeviceCount) {
        return last_m_rmat4DevicePose[controller_ind_left];
    }
    else if (i == 1 && controller_ind_right < vr::k_unMaxTrackedDeviceCount) {
        return last_m_rmat4DevicePose[controller_ind_right];
    }
    return QMatrix4x4();
}

float ViveManager::GetIPD() const
{
    return 0.064f;
}

QString ViveManager::GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError) const
{
    if (pHmd == NULL) {
        return "";
    }

    uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
    if( unRequiredBufferLen == 0 )
        return "";

    char *pchBuffer = new char[ unRequiredBufferLen ];
    unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
    QString sResult = pchBuffer;
    delete [] pchBuffer;
    return sResult;
}

int ViveManager::GetNumControllers() const
{
    int total = 0;
    if (controller_ind_left < vr::k_unMaxTrackedDeviceCount) {
        total++;
    }
    if (controller_ind_right < vr::k_unMaxTrackedDeviceCount) {
        total++;
    }
    return total;
}

bool ViveManager::GetControllerActive(const int i)
{
    if (i == 0) {
        return (controller_ind_left < vr::k_unMaxTrackedDeviceCount);
    }
    else if (i == 1) {
        return (controller_ind_right < vr::k_unMaxTrackedDeviceCount);
    }
    else {
        return false;
    }
}

bool ViveManager::GetControllerTracked(const int i)
{
    return controller_poses[i].bPoseIsValid;
}

QVector2D ViveManager::GetControllerThumbpad(const int i) const
{
    //const unsigned int joy_ind = vr::k_eControllerAxis_TrackPad;
    const unsigned int joy_ind = vr::k_eControllerAxis_None; //59.7 - for some reason, "none" gets trackpads on Vive wands and WMXR controllers
    if (joy_ind >= 0 && joy_ind < vr::k_unControllerStateAxisCount) {
        return QVector2D(controller_states[i].rAxis[joy_ind].x, controller_states[i].rAxis[joy_ind].y);
    }
    else {
        return QVector2D();
    }
}

bool ViveManager::GetControllerThumbpadTouched(const int i) const
{
    return ((vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & controller_states[i].ulButtonTouched) > 0);
}

bool ViveManager::GetControllerThumbpadPressed(const int i) const
{
    return ((vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & controller_states[i].ulButtonPressed) > 0);
}

QVector2D ViveManager::GetControllerStick(const int i) const
{
    const unsigned int joy_ind = vr::k_eControllerAxis_Joystick;
    if (joy_ind >= 0 && joy_ind < vr::k_unControllerStateAxisCount) {
        return QVector2D(controller_states[i].rAxis[joy_ind].x, controller_states[i].rAxis[joy_ind].y);
    }
    else {
        return QVector2D();
    }
}

bool ViveManager::GetControllerStickTouched(const int ) const
{
    return false;
}

bool ViveManager::GetControllerStickPressed(const int ) const
{
    return false;
}

float ViveManager::GetControllerTrigger(const int i) const
{
    return controller_states[i].rAxis[1].x;
}

float ViveManager::GetControllerGrip(const int i) const
{
    if ((vr::ButtonMaskFromId(vr::k_EButton_Grip) & controller_states[i].ulButtonPressed) > 0) {
        return 1.0f;
    }

    return 0.0f;
}

bool ViveManager::GetControllerMenuPressed(const int i)
{
    return (vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) & controller_states[i].ulButtonPressed) > 0;
}

bool ViveManager::GetControllerButtonAPressed() const
{
    return false;
}

bool ViveManager::GetControllerButtonBPressed() const
{
    return false;
}

bool ViveManager::GetControllerButtonXPressed() const
{
    return false;
}

bool ViveManager::GetControllerButtonYPressed() const
{
    return false;
}

bool ViveManager::GetControllerButtonATouched() const
{
    return false;
}

bool ViveManager::GetControllerButtonBTouched() const
{
    return false;
}

bool ViveManager::GetControllerButtonXTouched() const
{
    return false;
}

bool ViveManager::GetControllerButtonYTouched() const
{
    return false;
}

void ViveManager::TriggerHapticPulse(const int i, const int val)
{
    if (i == 0 && controller_ind_left < vr::k_unMaxTrackedDeviceCount) {
        m_pHMD->TriggerHapticPulse(controller_ind_left, 0, qMax(0, qMin(val, 3900)));
    }
    else if (i == 1 && controller_ind_right < vr::k_unMaxTrackedDeviceCount) {
        m_pHMD->TriggerHapticPulse(controller_ind_right, 0, qMax(0, qMin(val, 3900)));
    }
}

void ViveManager::Platform_ProcessMessages()
{

}

bool ViveManager::Platform_GetEntitled() const
{
    return true;
}

bool ViveManager::Platform_GetShouldQuit() const
{
    return false;
}
