#include "slidingtabwidget.h"

SlidingTabWidget::SlidingTabWidget()
{
    showing = false;
    sliding = false;
}

SlidingTabWidget::~SlidingTabWidget()
{

}

void SlidingTabWidget::SetWindowSize(QSize s)
{
    window_width = s.width();
    window_height = s.height();
}

bool SlidingTabWidget::GetShowing()
{
    return showing;
}

bool SlidingTabWidget::GetSliding()
{
    return sliding;
}

void SlidingTabWidget::Update()
{
    if (tabPosition() == QTabWidget::South)
    {
        int delta = -height() + tabBar()->height() + ((showing)?widget(0)->height():0);
        if (pos().y() != delta) move(0, delta);
    }
    else if (tabPosition() == QTabWidget::North)
    {
        int delta = window_height - tabBar()->height() - ((showing)?widget(0)->height():0);
        if (pos().y() != delta) move(0, delta);
    }
    else if (tabPosition() == QTabWidget::East)
    {
        int delta = -width() + tabBar()->width() + ((showing)?widget(0)->width():0);
        if (pos().x() != delta) move(delta, 0);
    }
    else if (tabPosition() == QTabWidget::West)
    {
        int delta = window_width - tabBar()->width() - ((showing)?widget(0)->width():0);
        if (pos().x() != delta) move(delta, 0);
    }
}

void SlidingTabWidget::Show()
{
    showing = true;}

void SlidingTabWidget::Hide()
{
    showing = false;
}

void SlidingTabWidget::mousePressEvent(QMouseEvent * event)
{
    if (this->tabBar()->tabAt(event->pos()) >= 0)
    {
        sliding = true;

        initial_mouse_pos = event->pos();
        last_mouse_pos = event->pos();

        slide_time.restart();
    }
}

void SlidingTabWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if (sliding)
    {
        float elapsed = (float) slide_time.elapsed() / 1000.0f;
        //Toggle on short press or when sliding in correct direction
        if (elapsed < 0.25f && sqrt(pow(event->pos().x() - initial_mouse_pos.x(), 2) + pow(event->pos().y() - initial_mouse_pos.y(), 2)) < 50) {
            showing = !showing;
        }
        else {
            float slide_speed_x = (float)(event->pos().x() - initial_mouse_pos.x())/elapsed;
            float slide_speed_y = (float)(event->pos().y() - initial_mouse_pos.y())/elapsed;

            if (abs(slide_speed_x) > 100 && tabPosition() == QTabWidget::East)
            {
                if (slide_speed_x > 0) {
                    showing = false;
                }
                else {
                    showing = true;
                }
            }
            else if (abs(slide_speed_x) > 100 && tabPosition() == QTabWidget::West)
            {
                if (slide_speed_x > 0) {
                    showing = true;
                }
                else {
                    showing = false;
                }
            }
            else if (abs(slide_speed_y) > 100 && tabPosition() == QTabWidget::South)
            {
                if (slide_speed_y > 0) {
                    showing = true;
                }
                else {
                    showing = false;
                }
            }
            else if (abs(slide_speed_y) > 100 && tabPosition() == QTabWidget::North)
            {
                if (slide_speed_y > 0) {
                    showing = false;
                }
                else {
                    showing = true;
                }
            }
         }
    }
    sliding = false;
}
