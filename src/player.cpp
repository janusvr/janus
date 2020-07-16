#include "player.h"

Player::Player() :
    theta(0.0f),
    phi(90.0f),
    correction_theta(0.0f),
    walk_forward(false),
    walk_left(false),
    walk_right(false),
    walk_back(false),
    fly_up(false),
    fly_down(false),
    spin_left(false),
    spin_right(false),
    follow_mode(false),
    comfort_spinleft(false),
    comfort_spinright(false),
    flytransition(false),
    hmd_calibrated(false),
    hand_update_time(0.0f),
    cursor0_scale(0.0f),
    cursor1_scale(0.0f),
    scale_velx(1.0f),
    scale_vely(1.0f),
    jump(false),
    speaking_no_mic(false),
    delta_time(0.0f),
    typing(false),
    entering_text(false),
    recording(false),
    player_collision_radius(0.2f)
{
    props = new DOMNode();
    props->SetType(TYPE_PLAYER);

    props->SetDir(QVector3D(1,0,0));
    comfort_timer.start();

    props->setProperty("touch", QVariant::fromValue<QObject *>(CreateControllerQObject().data()));
    props->setProperty("vive", QVariant::fromValue<QObject *>(CreateControllerQObject().data()));
    props->setProperty("wmxr", QVariant::fromValue<QObject *>(CreateControllerQObject().data()));
    props->setProperty("xbox", QVariant::fromValue<QObject *>(CreateControllerQObject().data()));

    running = false;
    flying = false;
    walking = false;
    speaking = false;
}

void Player::SetProperties(QPointer<DOMNode> props)
{
    this->props = props;
    props->SetType(TYPE_PLAYER);
}

QPointer<DOMNode> Player::GetProperties()
{
    return props;
}

void Player::UpdateDir()
{
    //qDebug() << "Player::UpdateDir()" << GetV("dir");
    props->SetDir(props->GetDir()->toQVector3D().normalized());    
    MathUtil::NormCartesianToSphere(props->GetDir()->toQVector3D(), theta, phi);    
}

void Player::UpdateEyePoint()
{
    QVector3D eye_point = props->GetPos()->toQVector3D() + props->GetLocalHeadPos()->toQVector3D();
    if (props->GetHMDType() != "vive" && props->GetHMDType() != "wmxr" ) {
        eye_point += props->GetEyePos()->toQVector3D();
    }
    props->SetEyePoint(eye_point);
}

QVector3D Player::GetRightDir() const
{
    return QVector3D::crossProduct(props->GetDir()->toQVector3D(), props->GetUpDir()->toQVector3D()).normalized();
}

QMatrix4x4 Player::GetTransform() const
{
    QMatrix4x4 m;
    m.rotate(-theta, QVector3D(0,1,0));
    m.setColumn(3, props->GetPos()->toQVector3D());
    m.setRow(3, QVector4D(0,0,0,1));
    return m;
}

void Player::SetCursorActive(const bool b, const int index)
{
    if (index == 0) {
        props->SetCursor0Active(b);
    }
    else {
        props->SetCursor1Active(b);
    }
}

bool Player::GetCursorActive(const int index) const
{
    return (index == 0) ? props->GetCursor0Active() : props->GetCursor1Active();
}

void Player::SetCursorPos(const QVector3D & p, const int index)
{
    if (index == 0) {
        props->SetCursor0Pos(p);
    }
    else {
        props->SetCursor1Pos(p);
    }
}

QVector3D Player::GetCursorPos(const int index) const
{    
    return (index == 0) ? props->GetCursor0Pos()->toQVector3D() : props->GetCursor1Pos()->toQVector3D();
}

void Player::SetCursorXDir(const QVector3D & p, const int index)
{
    if (index == 0) {
        props->SetCursor0XDir(p);
    }
    else {
        props->SetCursor1XDir(p);
    }
}

QVector3D Player::GetCursorXDir(const int index) const
{
    return (index == 0) ? props->GetCursor0XDir()->toQVector3D() : props->GetCursor1XDir()->toQVector3D();
}

void Player::SetCursorYDir(const QVector3D & p, const int index)
{
    if (index == 0) {
        props->SetCursor0YDir(p);
    }
    else {
        props->SetCursor1YDir(p);
    }
}

QVector3D Player::GetCursorYDir(const int index) const
{
    return (index == 0) ? props->GetCursor0YDir()->toQVector3D() : props->GetCursor1YDir()->toQVector3D();
}

void Player::SetCursorZDir(const QVector3D & p, const int index)
{
    if (index == 0) {
        props->SetCursor0ZDir(p);
    }
    else {
        props->SetCursor1ZDir(p);
    }
}

QVector3D Player::GetCursorZDir(const int index) const
{
    return (index == 0) ? props->GetCursor0ZDir()->toQVector3D() : props->GetCursor1ZDir()->toQVector3D();
}

void Player::SetCursorScale(const float f, const int index)
{
    if (index == 0) {
        cursor0_scale = f;
    }
    else {
        cursor1_scale = f;
    }
}

float Player::GetCursorScale(const int index) const
{
    return (index == 0) ? cursor0_scale : cursor1_scale;
}

float Player::ComputeCursorScale(const int index) const
{    
    if (index == 0) {
        const QVector3D v1 = props->GetCursor0Pos()->toQVector3D() - props->GetEyePoint();
        return 0.08f * QVector3D::dotProduct(v1, props->GetViewDir()->toQVector3D());
    }
    else {
        const QVector3D v1 = props->GetCursor1Pos()->toQVector3D() - props->GetEyePoint();
        return 0.08f * QVector3D::dotProduct(v1, props->GetViewDir()->toQVector3D());
    }
}

void Player::SetCursorObject(const QString & s, const int index)
{
    if (index == 0) {
        props->SetCursor0Object(s);
    }
    else {
        props->SetCursor1Object(s);
    }
}

QString Player::GetCursorObject(const int index) const
{
    return (index == 0) ? props->GetCursor0Object() : props->GetCursor1Object();
}

void Player::SpinView(const float f, const bool scale_rotation_speed)
{    
    const float rotation_speed = SettingsManager::GetRotationSpeed();    
    if (scale_rotation_speed) {
        theta += f * (rotation_speed / 60.0f);
    }
    else {
        theta += f;
    }

    if (theta > 360.0f) {
        theta -= 360.0f;
    }
    if (theta < 0.0f) {
        theta += 360.0f;
    } 
}

void Player::TiltView(const float f)
{
    const float rotation_speed = SettingsManager::GetRotationSpeed();    
    phi += f * rotation_speed / 60.0f;
    phi = qMax(phi, 5.0f);
    phi = qMin(phi, 175.0f);    
}

QVector3D Player::GetViewToWorldPoint(const QVector3D & p) const
{
    const QVector3D view_dir = props->GetViewDir()->toQVector3D();
    const QVector3D right_dir = GetRightDir();
    const QVector3D rotated_up_dir = QVector3D::crossProduct(view_dir, right_dir).normalized();
    return props->GetEyePoint() + view_dir * p.z() - right_dir * p.x() + rotated_up_dir * (-p.y());
}

QVector3D Player::GetViewToWorldDir(const QVector3D & d) const
{
    const QVector3D view_dir = props->GetViewDir()->toQVector3D();
    const QVector3D right_dir = GetRightDir();
    const QVector3D rotated_up_dir = QVector3D::crossProduct(view_dir, right_dir);
    return (view_dir * d.z() - right_dir * d.x() + rotated_up_dir * (-d.y()));
}

void Player::SetViewGL(const bool render_left_eye, const float eye_ipd, const QMatrix4x4 xform, const bool no_pitch/* = false*/)
{
    const float half_ipd = (render_left_eye ? -eye_ipd : eye_ipd) * 0.5f;

    QMatrix4x4 rot_mat;
    rot_mat.rotate(-(theta + correction_theta), 0, 1, 0);
    if (!no_pitch)
    {
        rot_mat.rotate(-(phi - 90.0f), 1, 0, 0);
    }

    QMatrix4x4 rotated_xform = rot_mat * xform;

    props->SetUpDir(rotated_xform.column(1).toVector3D());
    props->SetViewDir(-rotated_xform.column(2).toVector3D());
    props->SetLocalHeadPos(rotated_xform.column(3).toVector3D());

    UpdateEyePoint();

    QVector3D eye_point = props->GetEyePoint();
    props->SetGlobalHeadPos(eye_point);

    //view decoupled, forward dir is fixed with player's theta
    const float theta_rad = theta * MathUtil::_PI_OVER_180;
    QVector3D dir = MathUtil::GetRotatedAxis(-theta_rad, QVector3D(0,0,-1), QVector3D(0, 1, 0));
    dir.setY(props->GetViewDir()->toQVector3D().y());
    dir.normalize();
    props->SetDir(dir);

    //get the view matrix (current modelview matrix)
    QVector3D x = rotated_xform.column(0).toVector3D();
    QVector3D y = rotated_xform.column(1).toVector3D();
    QVector3D z = rotated_xform.column(2).toVector3D();
    QVector3D p = eye_point + rotated_xform.column(0).toVector3D() * half_ipd;

    QMatrix4x4 view_matrix;
    view_matrix.setRow(0, QVector4D(x.x(), x.y(), x.z(), -QVector3D::dotProduct(x, p)));
    view_matrix.setRow(1, QVector4D(y.x(), y.y(), y.z(), -QVector3D::dotProduct(y, p)));
    view_matrix.setRow(2, QVector4D(z.x(), z.y(), z.z(), -QVector3D::dotProduct(z, p)));
    view_matrix.setRow(3, QVector4D(0,0,0,1));

    //set view and model matrices
    MathUtil::LoadModelIdentity();
    MathUtil::LoadViewMatrix(view_matrix);
}

bool Player::GetWalking() const
{
    return walk_back || walk_forward || walk_left || walk_right;
}

void Player::DoSpinLeft(const bool b)
{
    if (b && !spin_left) {
        comfort_spinleft = true;
    }
    spin_left = b;
}

void Player::DoSpinRight(const bool b)
{
    if (b && !spin_right) {
        comfort_spinright = true;
    }
    spin_right = b;
}

void Player::SetFlying(const bool b)
{
    if (!flying && b) {
        flytransition = true;
        flyduration = 500.0f;
        time.start();
    }
    flying = b;
}

void Player::ResetCentre()
{
    //dir theta is the way forward (forward in reality, whatever direction dir_theta in game)
    const float viewdir_theta = atan2f(props->GetViewDir()->toQVector3D().z(),
                                       props->GetViewDir()->toQVector3D().x()) * MathUtil::_180_OVER_PI + 90.0f;
    const QVector3D dir = props->GetDir()->toQVector3D();
    const float dir_theta = atan2f(dir.z(), dir.x()) * MathUtil::_180_OVER_PI + 90.0f;

    theta = theta + viewdir_theta - dir_theta;
    correction_theta = correction_theta - viewdir_theta + dir_theta;

    //qDebug() << "Player::ResetCentre() - correction theta" << correction_theta;
}

void Player::Update(const float move_speed)
{    
    const float delta_t = delta_time;

    const LeapHand & hand0 = hands.first;
    const LeapHand & hand1 = hands.second;

    props->SetHand0Active(hand0.is_active);
    props->SetHand1Active(hand1.is_active);

    const float angle = -MathUtil::_PI_OVER_2 - atan2f(props->GetDir()->toQVector3D().z(),
                                                       props->GetDir()->toQVector3D().x());

    if (hand0.is_active) {                             
        QMatrix4x4 m;
        m.translate(props->GetPos()->toQVector3D());
        m.rotate(angle * MathUtil::_180_OVER_PI, QVector3D(0,1,0));
        m = m * hand0.basis;
        props->SetHand0XDir(m.column(0).toVector3D());
        props->SetHand0YDir(m.column(1).toVector3D());
        props->SetHand0ZDir(m.column(2).toVector3D());
        props->SetHand0Pos(m.column(3).toVector3D());
    }
    else {
        props->SetHand0Vel(QVector3D());
        hand0_pos_old = QVector3D();
    }

    if (hand1.is_active) {        
        QMatrix4x4 m;
        m.translate(props->GetPos()->toQVector3D());
        m.rotate(angle * MathUtil::_180_OVER_PI, QVector3D(0,1,0));
        m = m * hand1.basis;
        props->SetHand1XDir(m.column(0).toVector3D());
        props->SetHand1YDir(m.column(1).toVector3D());
        props->SetHand1ZDir(m.column(2).toVector3D());
        props->SetHand1Pos(m.column(3).toVector3D());
    }
    else {
        props->SetHand1Vel(QVector3D());
        hand1_pos_old = QVector3D();
    }

    //59.6 - update hand velocity at a minimum interval (30 times per second)
    const double hand_update_time_threshold = 1.0/30.0;    
    hand_update_time += delta_t; //integrate time for hand velocity updates
    if (hand_update_time > hand_update_time_threshold) {
        const QVector3D p0 = props->GetHand0Pos()->toQVector3D();
        const QVector3D p1 = props->GetHand1Pos()->toQVector3D();
        if (hand0.is_active && hand0_pos_old != QVector3D()) {
            props->SetHand0Vel((p0 - hand0_pos_old)/hand_update_time);
        }
        if (hand1.is_active && hand1_pos_old != QVector3D()) {
            props->SetHand1Vel((p1 - hand1_pos_old)/hand_update_time);
        }
        hand0_pos_old = p0;
        hand1_pos_old = p1;
        hand_update_time = 0.0;
    }    

    //qDebug() << "Player::Update() delta_t" << delta_t;
    const float rotation_speed = SettingsManager::GetRotationSpeed();
    if (SettingsManager::GetComfortMode() && hmd_enabled) {
        if (comfort_spinleft) {
            SpinView(-rotation_speed*0.5f, true);
            comfort_spinleft = false;
        }
        if (comfort_spinright) {
            SpinView(rotation_speed*0.5f, true);
            comfort_spinright = false;
        }
    }
    else {
        if (spin_left) {
            SpinView(-delta_t*rotation_speed, true); //NOTE: was 60.0
        }
        if (spin_right) {
            SpinView(delta_t*rotation_speed, true); //NOTE: was 60.0
        }
    }

    //compute walk forward direction walk_dir
    QVector3D walk_dir = props->GetViewDir()->toQVector3D();
    if (!flying) {
        walk_dir.setY(0.0f);
    }
    walk_dir.normalize();

    //compute side to side direction c
    const QVector3D c = QVector3D::crossProduct(walk_dir, QVector3D(0, 1, 0)).normalized();

    //compute our "impulse" velocity vector to move the player
    impulse_vel = QVector3D(0,0,0);

    //59.3 - condition ensures player moves in at least 1 direction
    if (walk_forward != walk_back || walk_left != walk_right || fly_up != fly_down || flytransition) {
        if (walk_forward) {
            impulse_vel += walk_dir;
        }
        if (walk_back) {
            impulse_vel -= walk_dir;
        }
        if (walk_left) {
            impulse_vel -= c;
        }
        if (walk_right) {
            impulse_vel += c;
        }

        impulse_vel.normalize();

        const QVector3D forward = walk_dir * QVector3D::dotProduct(walk_dir, impulse_vel);
        const QVector3D side = c * QVector3D::dotProduct(c, impulse_vel);

        impulse_vel = forward * scale_vely + side * scale_velx;
        impulse_vel *= move_speed;

        if (flying) {
            if (fly_up) {
                impulse_vel.setY(move_speed);
            }
            if (fly_down) {
                impulse_vel.setY(-move_speed);
            }

            if (flytransition) {
                if (time.elapsed() >= flyduration) {
                    flytransition = false;
                }
                else {
                    impulse_vel.setY(1.0f);
                }
            }
        }
    }

    //qDebug() << "Player::Update() impulse velocity" << scale_velx << scale_vely << impulse_vel << walk_forward << walk_back << walk_left << walk_right;
    if (flying) {
        props->SetAnimID("fly");
    }
    else if (running && GetWalking()) {
        props->SetAnimID("run");
    }
    else if (walk_forward && !walk_back) {
        props->SetAnimID("walk");
    }
    else if (walk_left && !walk_right) {
        props->SetAnimID("walk_left");
    }
    else if (walk_right && !walk_left) {
        props->SetAnimID("walk_right");
    }
    else if (walk_back && !walk_forward) {
        props->SetAnimID("walk_back");
    }
    else if (speaking) {
        props->SetAnimID("speak");
    }
    else if (typing) {
        props->SetAnimID("type");
    }
    else if (fabsf(props->GetVel()->toQVector3D().y()) > 0.01f) { //57.1 - too small a threshold, and the player will jitter between idle/jump animations
        props->SetAnimID("jump");
    }
    else {
        props->SetAnimID("idle");
    }

    UpdateEyePoint();

    if (!SettingsManager::GetMousePitchEnabled()) {
        phi = 90.0f;
    }   
}

QPair <LeapHand, LeapHand> Player::GetHands() const
{
    return hands;
}

LeapHand & Player::GetHand(const int i)
{
    return (i == 0) ? hands.first : hands.second;
}

const LeapHand & Player::GetHand(const int i) const
{
    return (i == 0) ? hands.first : hands.second;
}

QTime & Player::GetComfortTimer()
{
    return comfort_timer;
}

QPointer <QObject> Player::CreateControllerQObject()
{
    QPointer <QObject> o = new QObject(props);
    o->setProperty("left_trigger", 0.0f);
    o->setProperty("left_shoulder", 0.0f);
    o->setProperty("left_grip", 0.0f);
    o->setProperty("left_stick_x", 0.0f);
    o->setProperty("left_stick_y", 0.0f);
    o->setProperty("left_stick_click", 0.0f);
    o->setProperty("left_trackpad_x", 0.0f);
    o->setProperty("left_trackpad_y", 0.0f);
    o->setProperty("left_trackpad_click", 0.0f);
    o->setProperty("left_menu", 0.0f);
    o->setProperty("button_a", 0.0f);
    o->setProperty("button_b", 0.0f);
    o->setProperty("right_trigger", 0.0f);
    o->setProperty("right_shoulder", 0.0f);
    o->setProperty("right_grip", 0.0f);
    o->setProperty("right_stick_x", 0.0f);
    o->setProperty("right_stick_y", 0.0f);
    o->setProperty("right_stick_click", 0.0f);
    o->setProperty("right_trackpad_x", 0.0f);
    o->setProperty("right_trackpad_y", 0.0f);
    o->setProperty("right_trackpad_click", 0.0f);
    o->setProperty("right_menu", 0.0f);
    o->setProperty("button_x", 0.0f);
    o->setProperty("button_y", 0.0f);
    o->setProperty("dpad_up", 0.0f);
    o->setProperty("dpad_down", 0.0f);
    o->setProperty("dpad_left", 0.0f);
    o->setProperty("dpad_right", 0.0f);
    o->setProperty("button_select", 0.0f);
    o->setProperty("button_start", 0.0f);
    return o;
}

bool Player::GetJump() const
{
    return jump;
}

void Player::SetJump(bool value)
{
    jump = value;
}

float Player::GetScaleVelY() const
{
    return scale_vely;
}

void Player::SetScaleVelY(float value)
{
    scale_vely = value;
}

float Player::GetScaleVelX() const
{
    return scale_velx;
}

void Player::SetScaleVelX(float value)
{
    scale_velx = value;
}

bool Player::GetWalkForward() const
{
    return walk_forward;
}

void Player::SetWalkForward(bool value)
{
    walk_forward = value;
}

bool Player::GetWalkLeft() const
{
    return walk_left;
}

void Player::SetWalkLeft(bool value)
{
    walk_left = value;
}

bool Player::GetWalkRight() const
{
    return walk_right;
}

void Player::SetWalkRight(bool value)
{
    walk_right = value;
}

bool Player::GetWalkBack() const
{
    return walk_back;
}

void Player::SetWalkBack(bool value)
{
    walk_back = value;
}

bool Player::GetFollowMode() const
{
    return follow_mode;
}

void Player::SetFollowMode(bool value)
{
    follow_mode = value;
}

QString Player::GetFollowModeUserID() const
{
    return follow_mode_userid;
}

void Player::SetFollowModeUserID(const QString &value)
{
    follow_mode_userid = value;
}

float Player::GetTheta() const
{
    return theta;
}

void Player::SetTheta(float value)
{
    theta = value;
}

float Player::GetPhi() const
{
    return phi;
}

void Player::SetPhi(float value)
{
    phi = value;
}

bool Player::GetFlyUp() const
{
    return fly_up;
}

void Player::SetFlyUp(bool value)
{
    fly_up = value;
}

bool Player::GetFlyDown() const
{
    return fly_down;
}

void Player::SetFlyDown(bool value)
{
    fly_down = value;
}

float Player::GetCorrectionTheta() const
{
    return correction_theta;
}

void Player::SetCorrectionTheta(float value)
{
    correction_theta = value;
}

bool Player::GetSpinLeft() const
{
    return spin_left;
}

void Player::SetSpinLeft(bool value)
{
    spin_left = value;
}

bool Player::GetSpinRight() const
{
    return spin_right;
}

void Player::SetSpinRight(bool value)
{
    spin_right = value;
}

bool Player::GetHMDCalibrated() const
{
    return hmd_calibrated;
}

void Player::SetHMDCalibrated(bool value)
{
    hmd_calibrated = value;
}

void Player::SetHMDType(const QString s)
{
    props->SetHMDType(s);
}

QString Player::GetHMDType()
{
    return props->GetHMDType();
}

void Player::SetDeltaTime(const float f)
{
    delta_time = f;
}

void Player::SetRecording(const bool b)
{
    recording = b;
}

void Player::SetRunning(const bool b)
{
    running = b;
}

bool Player::GetFlying() const
{
    return flying;
}

bool Player::GetHMDEnabled() const
{
    return hmd_enabled;
}

void Player::SetHMDEnabled(bool value)
{
    hmd_enabled = value;
}

QVector3D Player::GetImpulseVel() const
{
    return impulse_vel;
}

void Player::SetImpulseVel(const QVector3D &value)
{
    impulse_vel = value;
}

bool Player::GetSpeaking() const
{
    return speaking;
}

void Player::SetSpeaking(bool value)
{
    speaking = value;
}

bool Player::GetTyping() const
{
    return typing;
}

void Player::SetTyping(bool value)
{
    typing = value;
}

bool Player::GetEnteringText() const
{
    return entering_text;
}

void Player::SetEnteringText(bool value)
{
    entering_text = value;
}

QString Player::GetDeviceType()
{
    return props->GetDeviceType();
}

void Player::SetDeviceType(const QString &value)
{
    props->SetDeviceType(value);
}

float Player::GetPlayerCollisionRadius() const
{
    return player_collision_radius;
}

void Player::SetPlayerCollisionRadius(float value)
{
    player_collision_radius = value;
}
