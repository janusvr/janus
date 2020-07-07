#include "propertieswindow.h"

PropertiesWindowIntegerValue::PropertiesWindowIntegerValue()
{
    setMinimumHeight(24);
    setMouseTracking(true);
}

void PropertiesWindowIntegerValue::mousePressEvent(QMouseEvent * e)
{
    if (this->hasFocus() && e->buttons() & Qt::LeftButton) {
        last_mouse_pos = e->pos();
    }

}

void PropertiesWindowIntegerValue::mouseMoveEvent(QMouseEvent * e)
{
    if (this->hasFocus() && e->buttons() & Qt::LeftButton) {
        const int delta =  int((e->pos().x() - float(last_mouse_pos.x())) * 0.05f);
        if (delta != 0) {
            SetValue(GetValue() + delta);
            last_mouse_pos = e->pos();
        }
    }

}

void PropertiesWindowIntegerValue::SetValue(const int i)
{
    setText(QString::number(i));
}

int PropertiesWindowIntegerValue::GetValue() const
{
    return text().toInt();
}

PropertiesWindowFloatValue::PropertiesWindowFloatValue()
{
    setMinimumHeight(24);
    setMouseTracking(true);
}

void PropertiesWindowFloatValue::mousePressEvent(QMouseEvent * e)
{
    if (this->hasFocus() && e->buttons() & Qt::LeftButton) {
        last_mouse_pos = e->pos();
    }
}

void PropertiesWindowFloatValue::mouseMoveEvent(QMouseEvent * e)
{
    if (this->hasFocus() && e->buttons() & Qt::LeftButton) {
        const float delta =  (e->pos().x() - float(last_mouse_pos.x())) * 0.01f;
        SetValue(GetValue() + delta);
    }
    last_mouse_pos = e->pos();
}

void PropertiesWindowFloatValue::SetValue(const float f)
{
    setText(QString::number(f, 'f'));
}

float PropertiesWindowFloatValue::GetValue() const
{
    return text().toFloat();
}

PropertiesWindowVectorWidget::PropertiesWindowVectorWidget()
{
    x = new PropertiesWindowFloatValue();
    connect(x, SIGNAL(textChanged(QString)), this, SLOT(FloatValueChanged()));
    y = new PropertiesWindowFloatValue();
    connect(y, SIGNAL(textChanged(QString)), this, SLOT(FloatValueChanged()));
    z = new PropertiesWindowFloatValue();
    connect(z, SIGNAL(textChanged(QString)), this, SLOT(FloatValueChanged()));

    x->setMinimumHeight(24);
    y->setMinimumHeight(24);
    z->setMinimumHeight(24);

    QHBoxLayout * layout = new QHBoxLayout;

    layout->addWidget(new QLabel("X"));
    layout->addWidget(x);
    layout->addWidget(new QLabel("Y"));
    layout->addWidget(y);
    layout->addWidget(new QLabel("Z"));
    layout->addWidget(z);

    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void PropertiesWindowVectorWidget::SetValue(const QVector3D v)
{
    x->SetValue(v.x());
    y->SetValue(v.y());
    z->SetValue(v.z());
}

QVector3D PropertiesWindowVectorWidget::GetValue() const
{
    return QVector3D(x->GetValue(), y->GetValue(), z->GetValue());
}

bool PropertiesWindowVectorWidget::HasFocus() const
{
    return x->hasFocus() || y->hasFocus() || z->hasFocus();
}

void PropertiesWindowVectorWidget::FloatValueChanged()
{
    emit valueChanged();
}

PropertiesWindow::PropertiesWindow(Game * g) :
    game(g)
{
    visible_widget = new QCheckBox;
    connect(visible_widget, SIGNAL(stateChanged(int)), this, SLOT(SetSyncOnObject()));
    type_widget = new QLineEdit;
    connect(type_widget, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    id_widget = new QLineEdit;
    connect(id_widget, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    js_id_widget = new QLineEdit;
    connect(js_id_widget, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    pos_widget = new PropertiesWindowVectorWidget();
    connect(pos_widget, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    rot_widget = new PropertiesWindowVectorWidget();
    connect(rot_widget, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    scale_widget = new PropertiesWindowVectorWidget();
    connect(scale_widget, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));

    col_button = new QPushButton();
    col_button->setText("Select");
    col_dialog = new QColorDialog();
    col_dialog->setWindowTitle("Color Selector");
    col_dialog->setOption(QColorDialog::ShowAlphaChannel);
    lighting_checkbox = new QCheckBox();
    connect(lighting_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SetSyncOnObject()));
    locked_checkbox = new QCheckBox();
    connect(locked_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SetSyncOnObject()));

    add_child_combobox = new PropertiesComboBox();
    add_child_combobox->addItem("ghost");
    add_child_combobox->addItem("sound");
    add_child_combobox->addItem("object");
    add_child_combobox->addItem("text");
    add_child_combobox->addItem("paragraph");
    add_child_combobox->addItem("image");
    add_child_combobox->addItem("video");
    add_child_combobox->addItem("particle");
    add_child_combobox->addItem("light");
    add_child_combobox->addItem("link");

    culling_combobox = new PropertiesComboBox();
    culling_combobox->addItem(tr("back"));
    culling_combobox->addItem(tr("front"));
    culling_combobox->addItem(tr("none"));    
    connect(culling_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSyncOnObject()));

    imageid_widget = new QLineEdit;
    connect(imageid_widget, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    link_url = new QLineEdit;
    connect(link_url, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    link_title = new QLineEdit;
    connect(link_title, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    videoid_widget = new QLineEdit;
    connect(videoid_widget, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    light_intensity = new PropertiesWindowFloatValue;
    connect(light_intensity, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    light_cone_angle = new PropertiesWindowFloatValue;
    connect(light_cone_angle, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    light_cone_exponent = new PropertiesWindowFloatValue;
    connect(light_cone_exponent, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    light_range = new PropertiesWindowFloatValue;
    connect(light_range, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    draw_layer = new PropertiesWindowIntegerValue;
    connect(draw_layer, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    sound_loop = new QCheckBox;
    connect(sound_loop, SIGNAL(stateChanged(int)), this, SLOT(SetSyncOnObject()));
    sound_gain = new PropertiesWindowFloatValue;
    connect(sound_gain, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    sound_doppler_factor = new PropertiesWindowFloatValue;
    connect(sound_doppler_factor, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    sound_pitch = new PropertiesWindowFloatValue;
    connect(sound_pitch, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));

    particle_imageid = new QLineEdit;
    connect(particle_imageid, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    particle_rate = new PropertiesWindowIntegerValue;
    connect(sound_pitch, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    particle_count = new PropertiesWindowIntegerValue;
    connect(sound_pitch, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    particle_duration = new PropertiesWindowFloatValue;
    connect(sound_pitch, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    particle_fadein = new PropertiesWindowFloatValue;
    connect(sound_pitch, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    particle_fadeout = new PropertiesWindowFloatValue;
    connect(sound_pitch, SIGNAL(textChanged(QString)), this, SLOT(SetSyncOnObject()));
    particle_vel = new PropertiesWindowVectorWidget;
    connect(particle_vel, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_accel = new PropertiesWindowVectorWidget;
    connect(particle_accel, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_randpos = new PropertiesWindowVectorWidget;
    connect(particle_randpos, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_randvel = new PropertiesWindowVectorWidget;
    connect(particle_randvel, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_randaccel = new PropertiesWindowVectorWidget;
    connect(particle_randaccel, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_randcol = new PropertiesWindowVectorWidget;
    connect(particle_randcol, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_randscale = new PropertiesWindowVectorWidget;
    connect(particle_randscale, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    particle_loop = new QCheckBox;
    connect(particle_loop, SIGNAL(stateChanged(int)), this, SLOT(SetSyncOnObject()));

    collision_type = new PropertiesComboBox;
    collision_type->addItem(tr(""));
    collision_type->addItem(tr("cube"));
    connect(collision_type, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSyncOnObject()));
    collision_pos = new PropertiesWindowVectorWidget;
    connect(collision_pos, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));
    collision_scale = new PropertiesWindowVectorWidget;
    connect(collision_scale, SIGNAL(valueChanged()), this, SLOT(SetSyncOnObject()));

    //setWindowTitle("Properties");

    QVBoxLayout * layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);

    QFormLayout * object_layout = new QFormLayout();
    object_groupbox = new QGroupBox();
    object_layout->addRow("Type", type_widget);
    object_layout->addRow("js_id", js_id_widget);
    object_layout->addRow("id", id_widget);
    object_layout->addRow("draw_layer", draw_layer);
    object_layout->addRow("visible", visible_widget);
    object_layout->addRow("locked", locked_checkbox);
    object_layout->addRow("Add Child", add_child_combobox);
    object_groupbox->setLayout(object_layout);
    object_groupbox->setTitle("PROPERTIES");
    object_groupbox->setMinimumHeight(24*object_layout->rowCount()+24);
    //object_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QFormLayout * xform_layout = new QFormLayout();
    xform_groupbox = new QGroupBox();
    xform_layout->addRow("Position", pos_widget);
    xform_layout->addRow("Rotation", rot_widget);
    xform_layout->addRow("Scale", scale_widget);
    xform_groupbox->setLayout(xform_layout);
    xform_groupbox->setTitle("TRANSFORM");
    xform_groupbox->setMinimumHeight(24*xform_layout->rowCount()+24);
    //xform_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QFormLayout * appear_layout = new QFormLayout();
    appear_groupbox = new QGroupBox();
    appear_layout->addRow("Color", col_button);
    appear_layout->addRow("Lighting", lighting_checkbox);
    appear_layout->addRow("Culling", culling_combobox);
    appear_layout->addRow("image_id", imageid_widget);
    appear_layout->addRow("video_id", videoid_widget);
    appear_groupbox->setLayout(appear_layout);
    appear_groupbox->setTitle("APPEARANCE");
    appear_groupbox->setMinimumHeight(24*appear_layout->rowCount()+24);
    //appear_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QFormLayout * light_layout = new QFormLayout();
    light_layout->addRow("Intensity", light_intensity);
    light_layout->addRow("Range", light_range);
    light_layout->addRow("Cone Angle", light_cone_angle);
    light_layout->addRow("Cone Exponent", light_cone_exponent);
    light_groupbox = new QGroupBox();
    light_groupbox->setLayout(light_layout);
    light_groupbox->setTitle("LIGHT");
    light_groupbox->setVisible(false);
    light_groupbox->setMinimumHeight(24*light_layout->rowCount()+24);
    //light_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QFormLayout * link_layout = new QFormLayout();
    link_layout->addRow("URL", link_url);
    link_layout->addRow("Title", link_title);
    link_groupbox = new QGroupBox();
    link_groupbox->setLayout(link_layout);
    link_groupbox->setTitle("LINK");
    link_groupbox->setVisible(false);
    link_groupbox->setMinimumHeight(24*light_layout->rowCount()+24);

    QFormLayout * sound_layout = new QFormLayout();
    sound_layout->addRow("Loop", sound_loop);
    sound_layout->addRow("Gain", sound_gain);
    sound_layout->addRow("Pitch", sound_pitch);
    sound_layout->addRow("Doppler Factor", sound_doppler_factor);
    sound_groupbox = new QGroupBox();
    sound_groupbox->setLayout(sound_layout);
    sound_groupbox->setTitle("SOUND");
    sound_groupbox->setVisible(false);
    sound_groupbox->setMinimumHeight(24*sound_layout->rowCount()+24);
    //sound_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QFormLayout * particle_layout = new QFormLayout();
    particle_layout->addRow("image_id", particle_imageid);
    particle_layout->addRow("Loop", particle_loop);
    particle_layout->addRow("Count", particle_count);
    particle_layout->addRow("Duration", particle_duration);
    particle_layout->addRow("Fade-in", particle_fadein);
    particle_layout->addRow("Fade-out", particle_fadeout);
    particle_layout->addRow("Rate", particle_rate);
    particle_layout->addRow("Random Position", particle_randpos);
    particle_layout->addRow("Velocity", particle_vel);
    particle_layout->addRow("Random Velocity", particle_randvel);
    particle_layout->addRow("Acceleration", particle_accel);
    particle_layout->addRow("Random Acceleration", particle_randaccel);
    particle_layout->addRow("Random Color", particle_randcol);
    particle_layout->addRow("Random Scale", particle_randscale);
    particle_groupbox = new QGroupBox();
    particle_groupbox->setLayout(particle_layout);
    particle_groupbox->setTitle("PARTICLES");
    particle_groupbox->setVisible(false);
    particle_groupbox->setMinimumHeight(24*particle_layout->rowCount()+24);
    //particle_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QFormLayout * collider_layout = new QFormLayout();
    collider_layout->addRow("Type", collision_type);
    collider_layout->addRow("Position", collision_pos);
    collider_layout->addRow("Scale", collision_scale);
    collision_groupbox = new QGroupBox();
    collision_groupbox->setLayout(collider_layout);
    collision_groupbox->setTitle("COLLISION");
    collision_groupbox->setVisible(false);
    collision_groupbox->setMinimumHeight(24*collider_layout->rowCount()+24);
    //collision_groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(object_groupbox);
    layout->addWidget(xform_groupbox);
    layout->addWidget(appear_groupbox);
    layout->addWidget(light_groupbox);
    layout->addWidget(link_groupbox);
    layout->addWidget(sound_groupbox);
    layout->addWidget(particle_groupbox);
    layout->addWidget(collision_groupbox);

    layout->setSizeConstraint(QLayout::SetFixedSize);

    QWidget * w = new QWidget;
    w->setLayout(layout);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    this->setWidget(w);

    connect(add_child_combobox, SIGNAL(activated(QString)), this, SLOT(AddChildObject(QString)));
    connect(col_button, SIGNAL(clicked(bool)), this, SLOT(ShowColourDialog(bool)));
    connect(col_dialog, SIGNAL(colorSelected(QColor)), this, SLOT(SelectColour(QColor)));
    connect(culling_combobox, SIGNAL(activated(QString)), this, SLOT(SetCulling(QString)));
    connect(collision_type, SIGNAL(activated(QString)), this, SLOT(SetCollisionType(QString)));
}

void PropertiesWindow::Update()
{
    cur_room = game->GetEnvironment()->GetCurRoom();
    if (cur_room.isNull()) {
        return;
    }

    QHash <QString, QPointer <RoomObject> > & envobjects = cur_room->GetRoomObjects();
    const QString sel = game->GetSelected(0);

    if (envobjects.contains(sel) && !envobjects[sel].isNull() && envobjects[sel] != cur_object) {
        cur_object = envobjects[sel];
        UpdateLayout();
    }

    QHash <QString, QPointer <AssetObject> > & aos = cur_room->GetAssetObjects();
    if (collision_type->count() != aos.size()) {
        //repopulate collision_id dropdown
        collision_type->clear();
        collision_type->addItems(aos.keys());
    }

    UpdateProperties();
}

void PropertiesWindow::UpdateLayout()
{
    if (cur_object.isNull()) {
        return;
    }

    const ElementType t = cur_object->GetType();

    light_groupbox->setVisible(t == TYPE_LIGHT);
    link_groupbox->setVisible(t == TYPE_LINK);
    sound_groupbox->setVisible(t == TYPE_SOUND);
    particle_groupbox->setVisible(t == TYPE_PARTICLE);
    collision_groupbox->setVisible(t == TYPE_OBJECT);
}

void PropertiesWindow::UpdateProperties()
{
    if (cur_object.isNull()) {
        return;
    }

    if (!type_widget->hasFocus()) {
        type_widget->setText(cur_object->GetProperties()->GetTypeAsString());
    }
    else {
        const QString t = type_widget->text();
        if (t != cur_object->GetType()) {
            cur_object->SetType(DOMNode::StringToElementType(t));
            UpdateLayout();
        }
    }

    if (!id_widget->hasFocus()) {
        id_widget->setText(cur_object->GetProperties()->GetID());
    }
    else {
        cur_object->GetProperties()->SetID(id_widget->text());
    }

    if (!draw_layer->hasFocus()) {
        draw_layer->SetValue(cur_object->GetProperties()->GetDrawLayer());
    }
    else {
        cur_object->GetProperties()->SetDrawLayer(id_widget->text().toInt());
    }

    if (!js_id_widget->hasFocus()) {
        js_id_widget->setText(cur_object->GetProperties()->GetJSID());
    }
    else {
        const QString s = js_id_widget->text();

        if (!s.isEmpty()) {
            game->RenameJSID(cur_object->GetProperties()->GetJSID(), s);
        }
    }

    if (!visible_widget->hasFocus()) {
        visible_widget->setChecked(cur_object->GetProperties()->GetVisible());
    }
    else {
        cur_object->GetProperties()->SetVisible(visible_widget->isChecked());
    }

    if (!pos_widget->HasFocus()) {
        pos_widget->SetValue(cur_object->GetPos());
    }
    else {
        cur_object->GetProperties()->SetPos(pos_widget->GetValue());
    }

    UpdateRotation();

    if (!scale_widget->HasFocus()) {
        scale_widget->SetValue(cur_object->GetScale());
    }
    else {
        cur_object->GetProperties()->SetScale(scale_widget->GetValue());
    }

    //color
    QPalette pal = col_button->palette();
    pal.setColor(QPalette::Button, MathUtil::GetVector4AsColour(cur_object->GetProperties()->GetColour()->toQVector4D()));
    col_button->setAutoFillBackground(true);
    col_button->setPalette(pal);
    col_button->update();

    if (!lighting_checkbox->hasFocus()) {
        lighting_checkbox->setChecked(cur_object->GetProperties()->GetLighting());
    }
    else {
        cur_object->GetProperties()->SetLighting(lighting_checkbox->isChecked());
    }

    if (!locked_checkbox->hasFocus()) {
        locked_checkbox->setChecked(cur_object->GetProperties()->GetLocked());
    }
    else {
        cur_object->GetProperties()->SetLocked(locked_checkbox->isChecked());
    }

    for (int i=0; i<culling_combobox->count(); ++i) {
        if (culling_combobox->itemText(i) == cur_object->GetProperties()->GetCullFace()) {
            culling_combobox->setCurrentIndex(i);
        }
    }

    if (!imageid_widget->hasFocus()) {
        imageid_widget->setText(cur_object->GetProperties()->GetImageID());
    }
    else {
        cur_object->GetProperties()->SetImageID(imageid_widget->text());
    }

    if (!videoid_widget->hasFocus()) {
        videoid_widget->setText(cur_object->GetProperties()->GetVideoID());
    }
    else {
        cur_object->GetProperties()->SetVideoID(videoid_widget->text());
    }

    //lights
    if (light_groupbox->isVisible()) {
        if (!light_intensity->hasFocus()) {
            light_intensity->SetValue(cur_object->GetProperties()->GetLightIntensity());
        }
        else {
            cur_object->GetProperties()->SetLightIntensity(light_intensity->GetValue());
        }

        if (!light_cone_angle->hasFocus()) {
            light_cone_angle->SetValue(cur_object->GetProperties()->GetLightConeAngle());
        }
        else {
            cur_object->GetProperties()->SetLightConeAngle(light_cone_angle->GetValue());
        }

        if (!light_cone_exponent->hasFocus()) {
            light_cone_exponent->SetValue(cur_object->GetProperties()->GetLightConeExponent());
        }
        else {
            cur_object->GetProperties()->SetLightConeExponent(light_cone_exponent->GetValue());
        }

        if (!light_range->hasFocus()) {
            light_range->SetValue(cur_object->GetProperties()->GetLightRange());
        }
        else {
            cur_object->GetProperties()->SetLightRange(light_range->GetValue());
        }
    }

    //link
    if (link_groupbox->isVisible()) {
        if (!link_url->hasFocus()) {
            link_url->setText(cur_object->GetProperties()->GetURL());
        }
        else {
            cur_object->GetProperties()->SetURL(link_url->text());
        }

        if (!link_title->hasFocus()) {
            link_title->setText(cur_object->GetProperties()->GetTitle());
        }
        else {
            cur_object->GetProperties()->SetTitle(link_title->text());
        }
    }

    if (sound_groupbox->isVisible()) {
        if (!sound_loop->hasFocus()) {
            sound_loop->setChecked(cur_object->GetProperties()->GetLoop());
        }
        else {
            cur_object->GetProperties()->SetLoop(sound_loop->isChecked());
        }

        if (!sound_pitch->hasFocus()) {
            sound_pitch->SetValue(cur_object->GetProperties()->GetPitch());
        }
        else {
            cur_object->GetProperties()->SetPitch(sound_pitch->GetValue());
        }

        if (!sound_gain->hasFocus()) {
            sound_gain->SetValue(cur_object->GetProperties()->GetGain());
        }
        else {
            cur_object->GetProperties()->SetGain(sound_gain->GetValue());
        }

        if (!sound_doppler_factor->hasFocus()) {
            sound_doppler_factor->SetValue(cur_object->GetProperties()->GetDopplerFactor());
        }
        else {
            cur_object->GetProperties()->SetDopplerFactor(sound_doppler_factor->GetValue());
        }
    }

    if (particle_groupbox->isVisible()) {
        if (!particle_loop->hasFocus()) {
            particle_loop->setChecked(cur_object->GetProperties()->GetLoop());
        }
        else {
            cur_object->GetProperties()->SetLoop(particle_loop->isChecked());
        }

        if (!particle_imageid->hasFocus()) {
            particle_imageid->setText(cur_object->GetProperties()->GetImageID());
        }
        else {
            cur_object->GetProperties()->SetImageID(particle_imageid->text());
        }

        if (!particle_count->hasFocus()) {
            particle_count->SetValue(cur_object->GetProperties()->GetCount());
        }
        else {
            cur_object->GetProperties()->SetCount(particle_count->GetValue());
        }

        if (!particle_duration->hasFocus()) {
            particle_duration->SetValue(cur_object->GetProperties()->GetDuration());
        }
        else {
            cur_object->GetProperties()->SetDuration(particle_duration->GetValue());
        }

        if (!particle_fadein->hasFocus()) {
            particle_fadein->SetValue(cur_object->GetProperties()->GetFadeIn());
        }
        else {
            cur_object->GetProperties()->SetFadeIn(particle_fadein->GetValue());
        }

        if (!particle_fadeout->hasFocus()) {
            particle_fadeout->SetValue(cur_object->GetProperties()->GetFadeOut());
        }
        else {
            cur_object->GetProperties()->SetFadeOut(particle_fadeout->GetValue());
        }

        if (!particle_rate->hasFocus()) {
            particle_rate->SetValue(cur_object->GetProperties()->GetRate());
        }
        else {
            cur_object->GetProperties()->SetRate(particle_rate->GetValue());
        }

        if (!particle_randpos->HasFocus()) {
            particle_randpos->SetValue(cur_object->GetProperties()->GetRandPos()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetRandPos(particle_randpos->GetValue());
        }

        if (!particle_vel->HasFocus()) {
            particle_vel->SetValue(cur_object->GetProperties()->GetVel()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetVel(particle_vel->GetValue());
        }

        if (!particle_randvel->HasFocus()) {
            particle_randvel->SetValue(cur_object->GetProperties()->GetRandVel()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetRandVel(particle_randvel->GetValue());
        }

        if (!particle_accel->HasFocus()) {
            particle_accel->SetValue(cur_object->GetProperties()->GetAccel()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetAccel(particle_accel->GetValue());
        }

        if (!particle_randaccel->HasFocus()) {
            particle_randaccel->SetValue(cur_object->GetProperties()->GetRandAccel()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetRandAccel(particle_randaccel->GetValue());
        }

        if (!particle_randcol->HasFocus()) {
            particle_randcol->SetValue(cur_object->GetProperties()->GetRandColour()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetRandColour(particle_randcol->GetValue());
        }

        if (!particle_randscale->HasFocus()) {
            particle_randscale->SetValue(cur_object->GetProperties()->GetRandScale()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetRandScale(particle_randscale->GetValue());
        }
    }

    if (collision_groupbox->isVisible()) {
        for (int i=0; i<collision_type->count(); ++i) {
            if (collision_type->itemText(i) == cur_object->GetProperties()->GetCollisionID()) {
                collision_type->setCurrentIndex(i);
            }
        }

        if (!collision_pos->HasFocus()) {
            collision_pos->SetValue(cur_object->GetProperties()->GetCollisionPos()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetCollisionPos(collision_pos->GetValue());
        }

        if (!collision_scale->HasFocus()) {
            collision_scale->SetValue(cur_object->GetProperties()->GetCollisionScale()->toQVector3D());
        }
        else {
            cur_object->GetProperties()->SetCollisionScale(collision_scale->GetValue());
        }
    }
}

void PropertiesWindow::UpdateRotation()
{
    if (!rot_widget->HasFocus()) {
        float heading, attitude, bank;
        MathUtil::MatrixToEulerAngles(cur_object->GetAttributeModelMatrixNoScaling(), heading, attitude, bank);
        rot_widget->SetValue(QVector3D(bank, heading, attitude));
    }
    else {
        QMatrix4x4 m;
        QVector3D v = rot_widget->GetValue();
        MathUtil::EulerAnglesToMatrix(v.y(), v.z(), v.x(), m);
        m.setColumn(3, QVector4D(cur_object->GetPos(), 1));
        cur_object->SetAttributeModelMatrixNoScaling(m);
    }
}

void PropertiesWindow::ShowColourDialog(const bool)
{
    if (cur_object) {
        col_dialog->setCurrentColor(MathUtil::GetVector4AsColour(cur_object->GetProperties()->GetColour()->toQVector4D()));
        col_dialog->open();
    }
}

void PropertiesWindow::SelectColour(const QColor c)
{
    if (cur_object && c.isValid()) {
        cur_object->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
        cur_object->GetProperties()->SetSync(true);
    }
    col_dialog->close();
}

void PropertiesWindow::SetCulling(const QString s)
{
    if (cur_object) {
        cur_object->GetProperties()->SetCullFace(s);
        cur_object->GetProperties()->SetSync(true);
    }
}

void PropertiesWindow::SetCollisionType(const QString s)
{
    if (cur_object) {
        cur_object->GetProperties()->SetCollisionID(s);
        cur_object->GetProperties()->SetSync(true);

        //Do some snapping stuff
        const QVector3D bmin = cur_object->GetBBoxMin();
        const QVector3D bmax = cur_object->GetBBoxMax();

        cur_object->GetProperties()->SetCollisionPos((bmin + bmax) * 0.5f);
        cur_object->GetProperties()->SetCollisionScale(QVector3D(fabsf(bmax.x() - bmin.x()),
                                     fabsf(bmax.y() - bmin.y()),
                                     fabsf(bmax.z() - bmin.z())));
    }
}

void PropertiesWindow::AddChildObject(const QString child_type)
{
    //qDebug() << "PropertiesWindow::AddChildObject" << index;
    if (cur_object) {
        QPointer <RoomObject> o = new RoomObject();
        o->SetParentObject(cur_object);
        o->SetType(DOMNode::StringToElementType(child_type));

        const QString js_id = cur_room->AddRoomObject(o);        
        cur_object->GetProperties()->AppendChild(o->GetProperties());
        cur_object->AppendChild(o);

        game->SetSelected(cur_room, js_id, true);
    }
}

void PropertiesWindow::SetSyncOnObject()
{
    if (cur_object) {
        cur_object->GetProperties()->SetSync(true);
    }
}

bool PropertiesWindow::GetHasFocus()
{
    return id_widget->hasFocus() ||
            js_id_widget->hasFocus() ||
            type_widget->hasFocus() ||
            imageid_widget->hasFocus() ||
            videoid_widget->hasFocus() ||
            particle_imageid->hasFocus() ||
            link_url->hasFocus() ||
            link_title->hasFocus();
}
