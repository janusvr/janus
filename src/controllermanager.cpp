#include "controllermanager.h"

QPointer <AssetObject> ControllerManager::vive_obj;
QVector <QPointer <AssetObject> > ControllerManager::touch_obj;
QVector <QPointer <AssetObject> > ControllerManager::wmxr_obj;
QPointer <AssetObject> ControllerManager::laser_obj;

ControllerManager::ControllerManager() :
    haptics(true)
{    
    using_gamepad = false;    

    //load the various controller models        
    if (vive_obj.isNull()) {
        vive_obj = new AssetObject();
        vive_obj->SetSrc(MathUtil::GetApplicationURL(), QString("assets/controllers/vive/ViveController.dae.gz"));
        vive_obj->Load();
    }

    if (touch_obj.isEmpty()) {
        touch_obj.resize(2);

        touch_obj[0] = new AssetObject();
        touch_obj[0]->SetSrc(MathUtil::GetApplicationURL(), QString("assets/controllers/oculus/TouchLeft.dae.gz"));
        touch_obj[0]->Load();

        touch_obj[1] = new AssetObject();
        touch_obj[1]->SetSrc(MathUtil::GetApplicationURL(), QString("assets/controllers/oculus/TouchRight.dae.gz"));
        touch_obj[1]->Load();
    }

    if (wmxr_obj.isEmpty()) {
        wmxr_obj.resize(2);

        //index 0 is right hand
        wmxr_obj[0] = new AssetObject();
        wmxr_obj[0]->SetSrc(MathUtil::GetApplicationURL(), QString("assets/controllers/wmxr/Left.dae.gz"));
        wmxr_obj[0]->Load();

        wmxr_obj[1] = new AssetObject();
        wmxr_obj[1]->SetSrc(MathUtil::GetApplicationURL(), QString("assets/controllers/wmxr/Right.dae.gz"));
        wmxr_obj[1]->Load();
    }

    //laser
    if (laser_obj.isNull()) {
        laser_obj = new AssetObject();
        laser_obj->SetSrc(MathUtil::GetApplicationURL(), QString("assets/controllers/cursor/IonCannon.dae"));
        laser_obj->Load();
    }
}

ControllerManager::~ControllerManager()
{    
    if (vive_obj) {
        delete vive_obj;
    }
    for (int i=0; i<touch_obj.size(); ++i) {
        if (touch_obj[i]) {
            delete touch_obj[i];
        }
    }  
    for (int i=0; i<wmxr_obj.size(); ++i) {
        if (wmxr_obj[i]) {
            delete wmxr_obj[i];
        }
    }
    if (laser_obj) {
        delete laser_obj;
    }
}

void ControllerManager::SetHMDManager(QPointer <AbstractHMDManager> m)
{
    hmd_manager = m;
}

QPointer <AbstractHMDManager> ControllerManager::GetHMDManager()
{
    return hmd_manager;
}

void ControllerManager::DrawGhostGL(QPointer <AssetShader> shader, const int i, const QString hmd_type, const QMatrix4x4 c)
{            
    MathUtil::PushModelMatrix();
    MathUtil::MultModelMatrix(c);

    shader->SetUseLighting(true);
    shader->UpdateObjectUniforms();

    if (QString::compare(hmd_type, "vive") == 0) {
        if (vive_obj) {
            vive_obj->DrawGL(shader);
        }
    }
    else if (QString::compare(hmd_type, "rift") == 0) {
        if (touch_obj[i]) {
            touch_obj[i]->DrawGL(shader);
        }
    }   
    else if (QString::compare(hmd_type, "wmxr") == 0) {
        if (wmxr_obj[i]) {
            wmxr_obj[i]->DrawGL(shader);
        }
    }

    MathUtil::PopModelMatrix();
}

void ControllerManager::DrawGL(QPointer <AssetShader> shader, QMatrix4x4 player_xform)
{
    if (shader.isNull()) {
        return;
    }

    if (hmd_manager == NULL) {
        return;
    }

    if (!hmd_manager->GetEnabled()) {
        return;
    }

    //controllers
    const QString hmd = hmd_manager->GetHMDType();

    for (int i=0; i<2; ++i) {
        if (s[i].active) {
            shader->SetUseLighting(true);
            shader->SetConstColour(QVector4D(1,1,1,1));
            shader->UpdateObjectUniforms();           

            MathUtil::PushModelMatrix();
            MathUtil::MultModelMatrix(player_xform);
            MathUtil::MultModelMatrix(s[i].xform);

            if (hmd == "rift") {
                if (touch_obj[i]) {
                    touch_obj[i]->DrawGL(shader);
                }
            }
            else if (hmd== "vive") {
                if (vive_obj) {
                    vive_obj->DrawGL(shader);
                }
            }           
            else if (hmd == "wmxr") {
                if (wmxr_obj[i]) {
                    wmxr_obj[i]->DrawGL(shader);
                }
            }

            if (s[i].laser_length > 0.0f) {
                MathUtil::PushModelMatrix();
                //59.9 - draw laser aimed 15 degrees downward
                if (hmd_manager->GetHMDType() != "rift") {
                    MathUtil::ModelMatrix().rotate(-15.0f, 1, 0, 0);
                }
                MathUtil::ModelMatrix().scale(0.5f, 0.5f, s[i].laser_length);

                shader->SetUseLighting(false);

                if (laser_obj) {
                    laser_obj->DrawGL(shader, QColor(0,255,0,64));
                }

                MathUtil::PopModelMatrix();

                shader->SetConstColour(QVector4D(1,1,1,1));
            }

            MathUtil::PopModelMatrix();
        }
    }   
}

void ControllerManager::Update(const bool use_gamepad)
{
    using_gamepad = false;

    //zero out things    
    for (int i=0; i<2; ++i) {
        s[i].x = 0.0f;
        s[i].y = 0.0f;
        for (int j=0; j<2; ++j) {
            s[i].t[j].value = 0.0f;
        }
    }

    if (hmd_manager && hmd_manager->GetEnabled() && hmd_manager->GetNumControllers() > 0) {
        UpdateControllers();
    }    
    else if (use_gamepad) {
        GamepadUpdate();
        if (GamepadIsConnected(GAMEPAD_0)) {
            UpdateGamepad(GAMEPAD_0);
            using_gamepad = true;
        }
        else if (GamepadIsConnected(GAMEPAD_1)) {
            UpdateGamepad(GAMEPAD_1);
            using_gamepad = true;
        }
        else if (GamepadIsConnected(GAMEPAD_2)) {
            UpdateGamepad(GAMEPAD_2);
            using_gamepad = true;
        }
        else if (GamepadIsConnected(GAMEPAD_3)) {
            UpdateGamepad(GAMEPAD_3);
            using_gamepad = true;
        }
    }

    UpdateAssets();
}

ControllerState * ControllerManager::GetStates()
{
    return &s[0];
}

bool ControllerManager::GetUsingGamepad() const
{
    return using_gamepad;
}

bool ControllerManager::GetUsingSpatiallyTrackedControllers() const
{    
    return (hmd_manager && hmd_manager->GetEnabled() && hmd_manager->GetNumControllers() > 0);
}

void ControllerManager::SetHapticsEnabled(const bool b)
{
    haptics = b;
}

void ControllerManager::UpdateControllers()
{
    if (hmd_manager.isNull()) {
        return;
    }

    for (int i=0; i<2; ++i) {
        s[i].active = hmd_manager->GetControllerTracked(i);

        if (s[i].active) {
            //update xforms
            s[i].xform = hmd_manager->GetControllerTransform(i);
            s[i].xform_prev = hmd_manager->GetLastControllerTransform(i);

            //update thumb/touchpad (oculus touch only)
            if (hmd_manager->GetHMDType() == "rift") {
                s[i].x = hmd_manager->GetControllerStick(i).x() * 2.0f;
                s[i].y = hmd_manager->GetControllerStick(i).y() * 2.0f;
            }
            else if (hmd_manager->GetHMDType() == "vive") {
                s[i].x = hmd_manager->GetControllerThumbpad(i).x();
                s[i].y = hmd_manager->GetControllerThumbpad(i).y();
            }            
            else if (hmd_manager->GetHMDType() == "wmxr") {
                s[i].x = hmd_manager->GetControllerStick(i).x();
                s[i].y = hmd_manager->GetControllerStick(i).y();

                s[i].x1 = hmd_manager->GetControllerThumbpad(i).x();
                s[i].y1 = hmd_manager->GetControllerThumbpad(i).y();
//                qDebug() << "hey" << i << s[i].x1 << s[i].y1;
            }

            //update buttons (j == 0 is nav, j == 1 is interact)
            for (int j=0; j<4; ++j) {
                bool b_pressed = false;
                bool b_hover = false;

                if (hmd_manager->GetHMDType() == "rift") {
                    if (i == 0) {
                        if (j == 3) {
                            b_pressed = hmd_manager->GetControllerMenuPressed(i);
                        }
                        else {
                            b_pressed = ((j == 0) ? hmd_manager->GetControllerButtonXPressed() : ((j == 1) ? hmd_manager->GetControllerButtonYPressed() : hmd_manager->GetControllerStickPressed(i)));
                            b_hover = ((j == 0) ? hmd_manager->GetControllerButtonXTouched() : ((j == 1) ? hmd_manager->GetControllerButtonYTouched() : hmd_manager->GetControllerStickTouched(i)));
                        }
                    }
                    else if (i == 1) {
                        b_pressed = ((j == 0) ? hmd_manager->GetControllerButtonAPressed() : ((j == 1) ? hmd_manager->GetControllerButtonBPressed() : hmd_manager->GetControllerStickPressed(i)));
                        b_hover = ((j == 0) ? hmd_manager->GetControllerButtonATouched() : ((j == 1) ? hmd_manager->GetControllerButtonBTouched() : hmd_manager->GetControllerStickTouched(i)));
                    }                  
                }
                else if (hmd_manager->GetHMDType() == "vive") {
                    if (j == 0) {
                        b_pressed = hmd_manager->GetControllerThumbpadPressed(i);
                        b_hover = hmd_manager->GetControllerThumbpadTouched(i);
                    }
                    else if (j == 1) {
                        b_pressed = hmd_manager->GetControllerMenuPressed(i);
                    }
                }              
                else if (hmd_manager->GetHMDType() == "wmxr") {
                    if (j == 0) {
                        b_pressed = hmd_manager->GetControllerThumbpadPressed(i);
                        b_hover = hmd_manager->GetControllerThumbpadTouched(i);
                    }
                    else if (j == 1) {
                        b_pressed = hmd_manager->GetControllerMenuPressed(i);
                    }
                }

                if (b_pressed) {
                    s[i].b[j].value = 1.0f;
                }
                else if (b_hover) {
                    s[i].b[j].value = 0.5f;
                }
                else {
                    s[i].b[j].value = 0.0f;
                }

//                qDebug() << i << j << b_hover;
                s[i].b[j].hover = b_hover;

                if (b_pressed && !s[i].b[j].pressed) {
                    s[i].b[j].pressed = true;
                    s[i].b[j].proc_press = true;
                }
                else if (!b_pressed && s[i].b[j].pressed) {
                    s[i].b[j].pressed = false;
                    s[i].b[j].proc_release = true;
                }
            }

            //update triggers
            for (int j=0; j<2; ++j) {
                const float t_val = ((j == 0) ? hmd_manager->GetControllerTrigger(i) : hmd_manager->GetControllerGrip(i));

                if (t_val >= 0.95f && !s[i].t[j].pressed) {
                    s[i].t[j].pressed = true;
                    s[i].t[j].proc_press = true;
                    TriggerHapticPulse(i, 64);
                }
                else if (t_val < 0.9f && s[i].t[j].pressed) {
                    s[i].t[j].pressed = false;
                    s[i].t[j].proc_release = true;
                }

                s[i].t[j].value = t_val;                
                s[i].t[j].hover = (t_val > 0.1f);
            }
        }
    }
}

void ControllerManager::UpdateGamepad(GAMEPAD_DEVICE dev)
{    
    if (!GamepadIsConnected(dev)) {
//        printf("%d) n/a\n", dev);
        return;
    }

    int lx, ly, rx, ry;
    GamepadStickXY(dev, STICK_LEFT, &lx, &ly);
    GamepadStickXY(dev, STICK_RIGHT, &rx, &ry);
    s[0].x = qMax(-1.0f, qMin(1.0f, float(lx)/32767.0f));
    s[0].y = qMax(-1.0f, qMin(1.0f, float(ly)/32767.0f));
    s[1].x = qMax(-1.0f, qMin(1.0f, float(rx)/32767.0f));
    s[1].y = qMax(-1.0f, qMin(1.0f, float(ry)/32767.0f));
//    qDebug() << lx << ly << rx << ry;
    s[0].t[0].value = GamepadTriggerLength(dev, TRIGGER_LEFT);   
    s[1].t[0].value = GamepadTriggerLength(dev, TRIGGER_RIGHT);

    bool b[2][7];
    b[0][0] = GamepadButtonDown(dev, BUTTON_Y);
    b[0][1] = GamepadButtonDown(dev, BUTTON_X);
    b[0][2] = GamepadButtonDown(dev, BUTTON_BACK);
    b[0][3] = GamepadButtonDown(dev, BUTTON_START);
    b[0][4] = GamepadButtonDown(dev, BUTTON_LEFT_SHOULDER);
    b[0][5] = GamepadButtonDown(dev, BUTTON_RIGHT_SHOULDER);
    b[0][6] = GamepadButtonDown(dev, BUTTON_LEFT_THUMB);

    b[1][0] = GamepadButtonDown(dev, BUTTON_B);
    b[1][1] = GamepadButtonDown(dev, BUTTON_A);
    b[1][2] = GamepadButtonDown(dev, BUTTON_DPAD_LEFT);
    b[1][3] = GamepadButtonDown(dev, BUTTON_DPAD_UP);
    b[1][4] = GamepadButtonDown(dev, BUTTON_DPAD_RIGHT);
    b[1][5] = GamepadButtonDown(dev, BUTTON_DPAD_DOWN);
    b[1][6] = GamepadButtonDown(dev, BUTTON_RIGHT_THUMB);

    for (int i=0; i<2; ++i) {
        for (int j=0; j<7; ++j) {
            //update buttons
            s[i].b[j].value = (s[i].b[j].pressed ? 1.0f : 0.0f);

            if (b[i][j] && !s[i].b[j].pressed) {
                s[i].b[j].pressed = true;
                s[i].b[j].proc_press = true;
            }
            else if (!b[i][j] && s[i].b[j].pressed) {
                s[i].b[j].pressed = false;
                s[i].b[j].proc_release = true;
            }
        }

        //update triggers
        for (int j=0; j<1; ++j) {
            const float t_val = s[i].t[j].value;

            if (t_val >= 0.9f && !s[i].t[j].pressed) {
                s[i].t[j].pressed = true;
                s[i].t[j].proc_press = true;
            }
            else if (t_val < 0.9f && s[i].t[j].pressed) {
                s[i].t[j].pressed = false;
                s[i].t[j].proc_release = true;
            }
        }
    }
}

void ControllerManager::TriggerHapticPulse(const int i, const int val)
{
    if (hmd_manager && hmd_manager->GetEnabled() && haptics) {
        hmd_manager->TriggerHapticPulse(i, val);
    }
}

float ControllerManager::GetAxisThreshold()
{
    return 0.2f;
}

void ControllerManager::UpdateAssets()
{
    if (vive_obj) {
        vive_obj->Update();
        vive_obj->UpdateGL();
    }
    for (int i=0; i<touch_obj.size(); ++i) {
        if (touch_obj[i]) {
            touch_obj[i]->Update();
            touch_obj[i]->UpdateGL();
        }
    }   
    for (int i=0; i<wmxr_obj.size(); ++i) {
        if (wmxr_obj[i]) {
            wmxr_obj[i]->Update();
            wmxr_obj[i]->UpdateGL();
        }
    }
    if (laser_obj) {
        laser_obj->Update();
        laser_obj->UpdateGL();
    }
}
