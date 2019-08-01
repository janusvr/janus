#ifndef SLIDINGTABWIDGET_H
#define SLIDINGTABWIDGET_H

#include <QtCore>
#include <QtWidgets>

class SlidingTabWidget : public QTabWidget
{
public:
    SlidingTabWidget();
    ~SlidingTabWidget();

    void SetWindowSize(QSize s);

    bool GetShowing();
    bool GetSliding();

    void Show();
    void Hide();

    void Update();

    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);

private:
    int window_width;
    int window_height;

    bool showing;

    bool sliding;

    QTime slide_time;

    QPoint initial_mouse_pos;
    QPoint last_mouse_pos;
};

#endif // SLIDINGTABWIDGET_H
