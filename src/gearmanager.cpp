#include "gearmanager.h"
//#include "mainwindow.h"

GearManager::GearManager()
{
    ovr = nullptr;
    idealTextureSize = QSize(640,480);

    last_controller_xform = QMatrix4x4();
    controller_xform = QMatrix4x4();

    last_hmd_xform = QMatrix4x4();
    hmd_xform = QMatrix4x4();

    m_eye_view_matrices.resize(2);
    m_eye_projection_matrices.resize(4);

    m_eye_view_matrices[0] = QMatrix4x4();
    m_eye_view_matrices[1] = QMatrix4x4();

    eye_FBOs[0] = NULL;

    m_eye_projection_matrices[0] = QMatrix4x4();
    m_eye_projection_matrices[1] = QMatrix4x4();
    m_eye_projection_matrices[2] = QMatrix4x4();
    m_eye_projection_matrices[3] = QMatrix4x4();

    initialized = false;
    gl_initialized = false;
    enter_vr = false;
    exit_vr = false;

    frame_index = 0;
    fence_index = 0;
    predicted_display_time = 0;

    surfaceRef = NULL;
    surface = nullptr;
    showing_vr = false;

    controller_tracked = false;
    head_controller_tracked = false;

    head_max_thumbpad = QVector2D(0, 0);
    head_menu_pressed = false;
    head_thumbpad_pressed = false; //Trackpad clicked
    head_thumbpad_hovered = false;
    head_thumbpad = QVector2D();

    menu_pressed = false;
    max_thumbpad = QVector2D(0, 0);
    thumbpad_pressed = false; //Trackpad clicked
    thumbpad_hovered = false;
    thumbpad = QVector2D();
    trigger_pressed = false; //Trigger pressed

    last_recenter_count = 0;

    entitled = true;

    paused = false;

    QAndroidJniEnvironment jniEnv;
    java.Env = (JNIEnv *) jniEnv;
    java.Env->GetJavaVM(&java.Vm);

    QAndroidJniObject qObjAct = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    jobject objAct = qObjAct.object<jobject>();
    //m_objectRef = jniEnv->NewGlobalRef(objAct);
    java.ActivityObject = jniEnv->NewGlobalRef(objAct);

#ifdef OCULUS_SUBMISSION_BUILD
    if (ovr_PlatformInitializeAndroid(GEAR_ID, java.ActivityObject, java.Env) != ovrPlatformInitialize_Success) {
        qDebug() << "ovr_PlatformInitializeWindows failed!";
        entitled = false;
    }
    ovrRequest req;
    req = ovr_Entitlement_GetIsViewerEntitled();
#endif
}

QSize GearManager::GetTextureSize() const
{
    return idealTextureSize;
}

bool GearManager::Initialize()
{
    JNIUtil::SetupGear();

    // Initialize the API.
    ovrInitParms initParms = vrapi_DefaultInitParms(&java);
    initParms.GraphicsAPI = VRAPI_GRAPHICS_API_OPENGL_ES_3;
    initParms.ProductVersion = 1;
    initParms.MajorVersion = 1;
    initParms.MinorVersion = 14;
    initParms.PatchVersion = 0;
    if (vrapi_Initialize(&initParms) != VRAPI_INITIALIZE_SUCCESS)
    {
        qDebug() << "Failed to initialize VrApi!";
        return false;
    }

    // Get the suggested resolution to create eye texture swap chains.
    int w = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH) * 0.85;
    int h = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT) * 0.85;
    idealTextureSize = QSize(w + 4,h);

    initialized = true;

    return true;
}

GearManager::~GearManager()
{
    // Destroy the texture swap chains.
    // Make sure to delete the swapchains before the application's EGLContext is destroyed.
    if (gl_initialized){
        for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
        {
            vrapi_DestroyTextureSwapChain( colorTextureSwapChain[eye] );
        }
    }

    vrapi_Shutdown();
}

void GearManager::PostPresent()
{
}

void GearManager::InitializeGL()
{
    if (!initialized) return; //Wait until initialized

    java.Vm->AttachCurrentThread(&java.Env, NULL);

    // Allocate a texture swap chain for each eye with the application's EGLContext current.
    for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++)
    {
        colorTextureSwapChain[eye] = vrapi_CreateTextureSwapChain(VRAPI_TEXTURE_TYPE_2D, VRAPI_TEXTURE_FORMAT_8888,
                                                                  idealTextureSize.width(),
                                                                  idealTextureSize.height(),
                                                                  1, true);
    }
    eye_FBOs[0] = new QOpenGLFramebufferObject(idealTextureSize.width(), idealTextureSize.height(), QOpenGLFramebufferObjectFormat());

    m_eye_viewports.resize(2);
    m_eye_viewports[0] = QVector4D(0, 0, idealTextureSize.width() - 4, idealTextureSize.height());
    m_eye_viewports[1] = QVector4D(idealTextureSize.width() + 4, 0, idealTextureSize.width() - 4, idealTextureSize.height());

    gl_initialized = true;
    //java.Vm->DetachCurrentThread();
}

QString GearManager::GetHMDString() const
{
    return QString("");
}

QString GearManager::GetHMDType() const
{
    //qDebug() << "headset" << vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_HEADSET_TYPE);
    //qDebug() << "device" << vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_DEVICE_TYPE);
    if (!initialized) return QString("");

    int device = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_DEVICE_TYPE);
    if (device >= VRAPI_DEVICE_TYPE_OCULUSGO_START && device < VRAPI_DEVICE_TYPE_OCULUSGO_END){
        return QString("go");
    }
    else{ //if (device >= VRAPI_DEVICE_TYPE_GEARVR_START && device < VRAPI_DEVICE_TYPE_GEARVR_END)
        return QString("gear");
    }
}

bool GearManager::GetEnabled() const
{
    return (initialized && gl_initialized && showing_vr);
}

float GearManager::GetIPD() const
{
    return 0.0f;
}

void GearManager::EnterVR()
{
    enter_vr = true;
    showing_vr = true;
    JNIUtil::ShowGear(true);
    //JNIUtil::HideSplash();
}

void GearManager::ExitVR()
{
    if (ovr != nullptr && showing_vr)
        exit_vr = true;
}

void GearManager::Update()
{
    if (!initialized || !gl_initialized) return;

    if (ovr != nullptr && showing_vr && exit_vr){
        vrapi_LeaveVrMode(ovr);
        ovr = nullptr;

        exit_vr = false;

        JNIUtil::ShowGear(false);
        showing_vr = false;

        return;
    }

    //qDebug() << "MOUNTED" << vrapi_GetSystemStatusInt(&java, VRAPI_SYS_STATUS_MOUNTED) << "(VRAPI_TRUE = 1)" << "Paused" << !showing_vr;
    if (!enter_vr){
        // Note: it seems we only need to pause if app is single player
        int mounted = vrapi_GetSystemStatusInt(&java, VRAPI_SYS_STATUS_MOUNTED);
        /*if (mounted == VRAPI_FALSE && paused){
            return;
        }
        else*/ if (mounted == VRAPI_FALSE && !paused){
            paused = true;
            //JNIUtil::GetMainWindow()->Pause();
            //return;
        }
        else if (mounted == VRAPI_TRUE && paused){
            paused = false;
            //JNIUtil::GetMainWindow()->Resume();
            ReCentre();
            //return;
        }
    }

    //Enter/Exit VR; Exit if paused; Set ovr appropriately to nullptr if exitted
    //Need to wait until Surface is ready in Java

    if (enter_vr && showing_vr && JNIUtil::GetGearReady() && ovr == nullptr){
        ovrModeParms modeParms = vrapi_DefaultModeParms(&java);

        //Get EGL context handle from GL context
        QEGLNativeContext egl = qvariant_cast<QEGLNativeContext> (QOpenGLContext::currentContext()->nativeHandle());
        modeParms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW | VRAPI_MODE_FLAG_ALLOW_POWER_SAVE | VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;
        modeParms.Display =  (unsigned long long) egl.display();
        modeParms.ShareContext = (unsigned long long) egl.context();
        surfaceRef = java.Env->NewGlobalRef(JNIUtil::GetWindowSurface());
        surface = ANativeWindow_fromSurface(java.Env, surfaceRef);
        modeParms.WindowSurface = (unsigned long long) surface;

        enter_vr = false;

        //Set latency/threads/clock levels
        vrapi_SetPerfThread(ovr, VRAPI_PERF_THREAD_TYPE_MAIN, JNIUtil::GetMainThreadID());
        vrapi_SetPerfThread(ovr, VRAPI_PERF_THREAD_TYPE_RENDERER, gettid());
        vrapi_SetExtraLatencyMode(ovr, VRAPI_EXTRA_LATENCY_MODE_ON);
        vrapi_SetClockLevels(ovr, 0, 0); // 5/5, 3/0, 4/0, 3/3, 4/4
        vrapi_SetDisplayRefreshRate(ovr, 60.0);

        //Enable fixed foveated rendering
        int device = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_DEVICE_TYPE);
        if (device >= VRAPI_DEVICE_TYPE_OCULUSGO_START && device < VRAPI_DEVICE_TYPE_OCULUSGO_END){
        //if (vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_FOVEATION_AVAILABLE) == VRAPI_TRUE){
            qDebug() << "Enabling foveated rendering!";
            vrapi_SetPropertyInt(&java, VRAPI_FOVEATION_LEVEL, 3); // High
            vrapi_SetPropertyInt(&java, VRAPI_REORIENT_HMD_ON_CONTROLLER_RECENTER, 1); // Reorient head AND controller
        }

        ovr = vrapi_EnterVrMode(&modeParms);

        // Set the tracking transform to use, by default this is eye level.
        vrapi_SetTrackingTransform(ovr, vrapi_GetTrackingTransform(ovr, VRAPI_TRACKING_TRANSFORM_SYSTEM_CENTER_EYE_LEVEL)); // VRAPI_TRACKING_TRANSFORM_SYSTEM_CENTER_FLOOR_LEVEL

        vrapi_SetRemoteEmulation(ovr, false);

        ReCentre(); //Always recenter upon resuming VR mode
    }

    if (!showing_vr || ovr == nullptr){
        return;
    }

    frame_index++;

    // Get the HMD pose, predicted for the middle of the time period during which
    // the new eye images will be displayed. The number of frames predicted ahead
    // depends on the pipeline depth of the engine and the synthesis rate.
    // The better the prediction, the less black will be pulled in at the edges.
    predicted_display_time = vrapi_GetPredictedDisplayTime(ovr, frame_index);
    tracking = vrapi_GetPredictedTracking2(ovr, predicted_display_time);

    // Advance the simulation based on the predicted display time.
    last_hmd_xform = hmd_xform;
    hmd_xform = GetMatrixFromPose(tracking.HeadPose.Pose, true);

    QMatrix4x4 l = OVRMatrixToMatrix(tracking.Eye[VRAPI_EYE_LEFT].ViewMatrix);
    QMatrix4x4 r = OVRMatrixToMatrix(tracking.Eye[VRAPI_EYE_RIGHT].ViewMatrix);
    QMatrix4x4 v = (l+r)/2;

    m_eye_view_matrices[VRAPI_EYE_LEFT] = v;
    m_eye_view_matrices[VRAPI_EYE_RIGHT] = v;

    GetProjectionMatrix(0, true);
    GetProjectionMatrix(1, true);
    GetProjectionMatrix(0, false);
    GetProjectionMatrix(1, false);

    //Controller input
    for ( uint32_t deviceIndex = 0; ; deviceIndex++ )
    {
        ovrInputCapabilityHeader device;

        if ( vrapi_EnumerateInputDevices( ovr, deviceIndex, &device ) < 0 )
        {
            break;	// no more devices
        }

        //qDebug() << "DEVICEID" << device.DeviceID;
        //qDebug() << "DEVICETYPE" << device.Type;

        ovrDeviceID deviceID = device.DeviceID;
        if (deviceID == ovrDeviceIdType_Invalid)
        {
            //Remove from devices
            //qDebug() << "INVALID DEVICE";
            continue;
        }
        if (device.Type == ovrControllerType_Headset)
        {

            ovrInputHeadsetCapabilities headsetCaps;
            headsetCaps.Header = device;
            vrapi_GetInputDeviceCapabilities( ovr, &headsetCaps.Header );

            head_max_thumbpad = QVector2D(headsetCaps.TrackpadMaxX, headsetCaps.TrackpadMaxY);

            //qDebug() << "HEADSET" << device.DeviceID << head_max_thumbpad;

            ovrInputStateHeadset headsetInputState;
            headsetInputState.Header.ControllerType = ovrControllerType_Headset;
            ovrResult result = vrapi_GetCurrentInputState(ovr, deviceID, &headsetInputState.Header);

            if ( result != ovrSuccess )
            {
                //Remove from devices
                //qDebug() << "HEADSET CONTINUE";
                head_controller_tracked = false;
                continue;
            }
            else
            {
                head_controller_tracked = true;
                //Get headset button state
                if (!head_menu_pressed)
                    head_menu_pressed = headsetInputState.Buttons & ovrButton_Back;
                head_thumbpad_pressed = headsetInputState.TrackpadStatus; //headsetInputState.Buttons & ovrButton_Enter; //Trackpad clicked
                head_thumbpad_hovered = head_thumbpad_pressed;
                head_thumbpad = QVector2D(headsetInputState.TrackpadPosition.x, headsetInputState.TrackpadPosition.y);
                //qDebug() << "HEADSET STATE" << head_menu_pressed << head_thumbpad << head_thumbpad_hovered << head_thumbpad_pressed;
            }
        }
        else if ( device.Type == ovrControllerType_TrackedRemote )
        {
            ovrTracking remoteTracking;
            ovrResult result = vrapi_GetInputTrackingState(ovr, deviceID, predicted_display_time, &remoteTracking);
            if ( result != ovrSuccess )
            {
                //Remove from devices
                controller_tracked = false;
                continue;
            }

            last_controller_xform = controller_xform;
            controller_xform = GetMatrixFromPose(remoteTracking.HeadPose.Pose, false); //This is not a head pose; it should give pos of controller

            ovrInputTrackedRemoteCapabilities remoteCaps;
            remoteCaps.Header = device;
            vrapi_GetInputDeviceCapabilities( ovr, &remoteCaps.Header );

            max_thumbpad = QVector2D(remoteCaps.TrackpadMaxX, remoteCaps.TrackpadMaxY);

            //qDebug() << "REMOTE" << device.DeviceID << max_thumbpad;
            //qDebug() << controller_xform;

            ovrInputStateTrackedRemote remoteInputState;
            remoteInputState.Header.ControllerType = device.Type;
            result = vrapi_GetCurrentInputState(ovr, deviceID, &remoteInputState.Header);

            if ( result != ovrSuccess )
            {
                //Remove from devices
                controller_tracked = false;
                //qDebug() << "REMOTE CONTINUE";
                continue;
            }
            else {
                controller_tracked = true;

                //Get controller button state
                if (!menu_pressed)
                    menu_pressed = remoteInputState.Buttons & ovrButton_Back;
                thumbpad_pressed = remoteInputState.Buttons & ovrButton_Enter; //Trackpad clicked
                thumbpad_hovered = remoteInputState.TrackpadStatus;
                thumbpad = QVector2D(remoteInputState.TrackpadPosition.x, remoteInputState.TrackpadPosition.y);
                trigger_pressed = remoteInputState.Buttons & ovrButton_A; //Trigger pressed

                if (last_recenter_count != remoteInputState.RecenterCount){
                    ReCentre();
                    last_recenter_count = remoteInputState.RecenterCount;
                }
                //qDebug() << "REMOTE STATE" << menu_pressed << thumbpad_pressed << thumbpad_hovered << thumbpad << trigger_pressed << last_recenter_count;
            }
        }
    }
}

QMatrix4x4 GearManager::GetHMDTransform() const
{
    return hmd_xform;
}

QMatrix4x4 GearManager::GetControllerTransform(const int i) const
{
    if (i == 0) {
        return controller_xform;
    }
    else if (i == 1) {
        return hmd_xform;
    }
    return QMatrix4x4();
}

QMatrix4x4 GearManager::GetLastControllerTransform(const int i) const
{
    if (i == 0) {
        return last_controller_xform;
    }
    else if (i == 1) {
        return last_hmd_xform;
    }
    return QMatrix4x4();
}

int GearManager::GetNumControllers() const
{
    if (GetHMDType() == "go") {
        if (controller_tracked) return 1; // CONTROLLER
        else return 0;
    }
    else {
        if (head_controller_tracked && controller_tracked) return 2; // HEAD and CONTROLLER
        else return 1;
    }
}

bool GearManager::GetControllerTracked(const int i)
{
    if (i == 0) {
        return controller_tracked;
    }
    else if (i == 1) {
        return head_controller_tracked;
    }
    return false;
}

QVector2D GearManager::GetControllerThumbpad(const int i) const
{
    //if (!(i == 0 && controller_tracked)) return QVector2D();

    if (i == 0) {
        if (thumbpad_hovered){
            //qDebug() << "THUMBPAD" << QVector2D(2*(thumbpad.x()/max_thumbpad.x() - 0.5),-2*(thumbpad.y()/max_thumbpad.y() - 0.5));
            return QVector2D(2*(thumbpad.x()/max_thumbpad.x() - 0.5),-2*(thumbpad.y()/max_thumbpad.y() - 0.5));
        }
    }
    else if (i == 1) {
        if (head_thumbpad_hovered){
            return QVector2D(2*(head_thumbpad.x()/head_max_thumbpad.x() - 0.5),-2*(head_thumbpad.y()/head_max_thumbpad.y() - 0.5));
        }
    }

    //qDebug() << "THUMBPAD";
    return QVector2D();
}

bool GearManager::GetControllerThumbpadTouched(const int i) const
{
    if (i == 0) {
        return thumbpad_hovered;
    }
    else if (i == 1) {
        QVector2D p = GetControllerThumbpad(i);
        return head_thumbpad_hovered && sqrt(pow(p.x(),2) + pow(p.y(),2)) < 0.5f;
    }

    return false;
}

bool GearManager::GetControllerThumbpadPressed(const int i) const
{
    if (i == 0) {
        return thumbpad_pressed;
    }
    else if (i == 1) {
        QVector2D p = GetControllerThumbpad(i);
        return head_thumbpad_pressed && sqrt(pow(p.x(),2) + pow(p.y(),2)) < 0.5f;
    }

    return false;
}

QVector2D GearManager::GetControllerStick(const int) const
{
    return QVector2D();
}

bool GearManager::GetControllerStickTouched(const int) const
{
    return false;
}

bool GearManager::GetControllerStickPressed(const int) const
{
    return false;
}

float GearManager::GetControllerTrigger(const int i) const
{
    if (!(i == 0 && controller_tracked)) return 0;
    return (trigger_pressed) ? 1.0f : 0.0f;
}

float GearManager::GetControllerGrip(const int) const
{
    return 0;
}

bool GearManager::GetControllerMenuPressed(const int i)
{
    if (i == 0) {
        bool m = menu_pressed;
        if (menu_pressed) menu_pressed = false;
        return m;
    }
    else if (i == 1) {
        bool h = head_menu_pressed;
        if (head_menu_pressed) head_menu_pressed = false;
        return h;
    }

    return false;

}

bool GearManager::GetControllerButtonAPressed() const
{
    return false;
}

bool GearManager::GetControllerButtonBPressed() const
{
    return false;
}

bool GearManager::GetControllerButtonXPressed() const
{
    return false;
}

bool GearManager::GetControllerButtonYPressed() const
{
    return false;
}

bool GearManager::GetControllerButtonATouched() const
{
    return false;
}

bool GearManager::GetControllerButtonBTouched() const
{
    return false;
}

bool GearManager::GetControllerButtonXTouched() const
{
    return false;
}

bool GearManager::GetControllerButtonYTouched() const
{
    return false;
}

QMatrix4x4 GearManager::GetMatrixFromPose(const ovrPosef & pose, const bool flip_z)
{
    QVector3D x, y, z, p;
    x = QVector3D(1,0,0);
    y = QVector3D(0,1,0);
    z = QVector3D(0,0,flip_z?-1:1);
    p = QVector3D(pose.Position.x, pose.Position.y + 1.6, pose.Position.z);

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


    if (flip_z){
        m.scale(1,1,-1);
    }

    return m;
}

void GearManager::EndRenderEye(const int )
{
}

void GearManager::BeginRendering()
{

}

QMatrix4x4 GearManager::GetProjectionMatrix(const int eye, const bool p_is_avatar)
{
    ovrMatrix4f proj = ovrMatrix4f_CreateProjectionFov(vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_X),
                                                       vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_Y),
                                                       0.0f,
                                                       0.0f,
                                                       (p_is_avatar) ? m_avatar_near_clip : m_near_clip,
                                                       (p_is_avatar) ? m_avatar_far_clip : m_far_clip);
    QMatrix4x4 m = OVRMatrixToMatrix(proj);
    /*for (int i=0; i<4; ++i) {
        for (int j=0; j<4; ++j) {
            m.data()[i+j*4] = proj.M[i][j];
        }
    }*/

    m_eye_projection_matrices[(p_is_avatar) ? eye + 2 : eye] = m;

    return m;
}

void GearManager::BeginRenderEye(const int)
{
}

void GearManager::EndRendering()
{
    if (ovr == nullptr || !showing_vr || !gl_initialized || !initialized){
        return;
    }

    // Render eye images and setup the 'ovrSubmitFrameDesc2' using 'ovrTracking2' data.
    layer = vrapi_DefaultLayerProjection2();
    layer.HeadPose = tracking.HeadPose;

    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        const int colorTextureSwapChainIndex = frame_index % vrapi_GetTextureSwapChainLength( colorTextureSwapChain[eye] );
        const unsigned int textureId = vrapi_GetTextureSwapChainHandle( colorTextureSwapChain[eye], colorTextureSwapChainIndex );

        // Render to 'textureId' using the 'ProjectionMatrix' from 'ovrTracking2'.
        MathUtil::glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eye_FBOs[0]->handle());
        MathUtil::glFuncs->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
        //GLenum drawBuffers[1];
        //drawBuffers[0] = GL_COLOR_ATTACHMENT0;
        //GLsizei drawBufferCount = 1;
        //MathUtil::glFuncs->glDrawBuffers(drawBufferCount, &drawBuffers[0]);
        MathUtil::glFuncs->glBlitFramebuffer(0, 0, idealTextureSize.width(), idealTextureSize.height(), 0, 0, idealTextureSize.width(), idealTextureSize.height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        MathUtil::glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        layer.Textures[eye].ColorSwapChain = colorTextureSwapChain[eye];
        layer.Textures[eye].SwapChainIndex = colorTextureSwapChainIndex;
        layer.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection( &tracking.Eye[eye].ProjectionMatrix );
        ovrRectf r = {};
        r.x = 0;
        r.y = 0;
        r.width = idealTextureSize.width();
        r.height = idealTextureSize.height();
        layer.Textures[eye].TextureRect = r;
    }

    /*qDebug() << "GearManager::FENCE";
    ovrFence fence = fences[fence_index];

    // Create and insert a new sync object.
    fence.Sync = MathUtil::glFuncs->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    MathUtil::glFuncs->glClientWaitSync( fence.Sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0 );

    fence_index = (frame_index + 1) % MAX_FENCES;*/

    const ovrLayerHeader2 * layers[] =
    {
        &layer.Header
    };

    ovrSubmitFrameDescription2 frameDesc = {};
    frameDesc.FrameIndex = frame_index;
    frameDesc.DisplayTime = predicted_display_time;
    frameDesc.SwapInterval = 1;
    //frameDesc.CompletionFence = (size_t)fence.Sync;
    frameDesc.CompletionFence = 0;
    frameDesc.LayerCount = 1;
    frameDesc.Layers = layers;

    // Hand over the eye images to the time warp.
    vrapi_SubmitFrame2( ovr, &frameDesc );
}

void GearManager::ReCentre()
{
    if (ovr != nullptr){
        vrapi_RecenterPose(ovr);
    }
}

void GearManager::TriggerHapticPulse(const int, const int)
{

}

void GearManager::Platform_ProcessMessages()
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

bool GearManager::Platform_GetEntitled() const
{
    return entitled;
}

bool GearManager::Platform_GetShouldQuit() const
{
    return false;
}

void GearManager::ShowQuitMenu()
{
    if (ovr == nullptr || !showing_vr || !gl_initialized || !initialized){
        return;
    }

    vrapi_ShowSystemUI(&java, VRAPI_SYS_UI_CONFIRM_QUIT_MENU);
}

QMatrix4x4 GearManager::OVRMatrixToMatrix(ovrMatrix4f m)
{
    return QMatrix4x4(
                m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3],
                m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3],
                m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
                m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]);
}
