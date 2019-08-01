#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"

class PropertiesComboBox : public QComboBox
{
public:
    PropertiesComboBox() {
        setFocusPolicy(Qt::StrongFocus);
    }

    void wheelEvent(QWheelEvent *event) {
        if (!hasFocus()) {
            event->ignore();
        }
        else {
            QComboBox::wheelEvent(event);
        }
    }
};

class PropertiesWindowIntegerValue : public QLineEdit
{
public:
    PropertiesWindowIntegerValue();

    void SetValue(const int i);
    int GetValue() const;

    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

private:
    QPoint last_mouse_pos;
};

class PropertiesWindowFloatValue : public QLineEdit
{
public:
    PropertiesWindowFloatValue();

    void SetValue(const float f);
    float GetValue() const;

    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

private:
    QPoint last_mouse_pos;
};

class PropertiesWindowVectorWidget : public QWidget
{
    Q_OBJECT
public:
    PropertiesWindowVectorWidget();

    void SetValue(const QVector3D v);
    QVector3D GetValue() const;

    bool HasFocus() const;

signals:
    void valueChanged();

public slots:

    void FloatValueChanged();

private:

    PropertiesWindowFloatValue * x;
    PropertiesWindowFloatValue * y;
    PropertiesWindowFloatValue * z;
};

class PropertiesWindow : public QScrollArea
{
    Q_OBJECT
public:
    PropertiesWindow(Game * g);

    void Update();    
    bool GetHasFocus();

public slots:

    void SetCulling(const QString s);
    void SetCollisionType(const QString s);
    void AddChildObject(const QString child_type);
    void ShowColourDialog(const bool b);
    void SelectColour(const QColor c);

    void SetSyncOnObject();

private:

    void UpdateLayout();
    void UpdateProperties();
    void UpdateRotation();

    Game * game;
    QPointer <Room> cur_room;
    QPointer <RoomObject> cur_object;

    PropertiesWindowVectorWidget * pos_widget;
    PropertiesWindowVectorWidget * rot_widget;
    PropertiesWindowVectorWidget * scale_widget;
    QCheckBox * visible_widget;
    QLineEdit * imageid_widget;
    QLineEdit * videoid_widget;
    QLineEdit * id_widget;
    QLineEdit * js_id_widget;
    QLineEdit * type_widget;
    QPushButton * col_button;
    QColorDialog * col_dialog;
    QCheckBox * lighting_checkbox;
    QCheckBox * locked_checkbox;
    PropertiesComboBox * add_child_combobox;
    PropertiesComboBox * culling_combobox;

    //sounds
    PropertiesWindowFloatValue * sound_gain;
    PropertiesWindowFloatValue * sound_doppler_factor;
    PropertiesWindowFloatValue * sound_pitch;
    QCheckBox * sound_loop;

    //lights
    PropertiesWindowFloatValue * light_intensity;
    PropertiesWindowFloatValue * light_cone_angle;
    PropertiesWindowFloatValue * light_cone_exponent;
    PropertiesWindowFloatValue * light_range;

    // Draw Priority
    PropertiesWindowIntegerValue * draw_layer;

    //particles
    QLineEdit * particle_imageid;
    PropertiesWindowIntegerValue * particle_rate;
    PropertiesWindowIntegerValue * particle_count;
    PropertiesWindowFloatValue * particle_duration;
    PropertiesWindowFloatValue * particle_fadein;
    PropertiesWindowFloatValue * particle_fadeout;
    PropertiesWindowVectorWidget * particle_vel;
    PropertiesWindowVectorWidget * particle_accel;
    PropertiesWindowVectorWidget * particle_randpos;
    PropertiesWindowVectorWidget * particle_randvel;
    PropertiesWindowVectorWidget * particle_randaccel;
    PropertiesWindowVectorWidget * particle_randcol;
    PropertiesWindowVectorWidget * particle_randscale;
    QCheckBox * particle_loop;

    //collider
    PropertiesComboBox * collision_type;
    PropertiesWindowVectorWidget * collision_pos;
    PropertiesWindowVectorWidget * collision_scale;

    //link
    QLineEdit * link_url;
    QLineEdit * link_title;

    QCheckBox * auto_play_checkbox;

    QCheckBox * play_once_checkbox;
    QComboBox * stereo_split_combobox;

    QGroupBox * object_groupbox;
    QGroupBox * xform_groupbox;
    QGroupBox * appear_groupbox;
    QGroupBox * sound_groupbox;
    QGroupBox * light_groupbox;
    QGroupBox * particle_groupbox;
    QGroupBox * collision_groupbox;
    QGroupBox * link_groupbox;
};

#endif // PROPERTIESWINDOW_H
