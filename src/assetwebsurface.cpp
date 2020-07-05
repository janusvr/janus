#include "assetwebsurface.h"

AssetWebSurface::AssetWebSurface() :
    tex_alpha(false),
    load_started(false),
    loaded(true),
    progress(0.0f),
    m_texture_handle(nullptr),
    update_texture(false)
{
    //qDebug() << "AssetWebSurface::AssetWebSurface" << this;        
    webview = new QWebEngineView();    
    connect(webview, SIGNAL(loadStarted()), this, SLOT(LoadStarted()));
    connect(webview, SIGNAL(loadProgress(int)), this, SLOT(LoadProgress(int)));
    connect(webview, SIGNAL(loadFinished(bool)), this, SLOT(LoadFinished(bool)));

    SetSize(1000,800);
    image = QImage(webview->size(), QImage::Format_RGB32);

    connect(&timer, SIGNAL(timeout()), this, SLOT(UpdateImage()));
}

AssetWebSurface::~AssetWebSurface()
{
    disconnect(&timer);
    timer.stop();

    //qDebug() << "AssetWebSurface::~AssetWebSurface()" << this;
    if (webview) {
        webview->hide();
        webview->deleteLater();
    }
}

void AssetWebSurface::SetTextureAlpha(const bool b)
{
    tex_alpha = b;
}

void AssetWebSurface::SetProperties(const QVariantMap & d)
{
    Asset::SetProperties(d);

    int w = props->GetWidth();
    int h = props->GetHeight();
    if (d.contains("width")) {
        w = d["width"].toInt();
    }
    if (d.contains("height")) {
        h = d["height"].toInt();
    }

    SetSize(w, h);
}

void AssetWebSurface::SetSrc(const QString & base, const QString & u)
{
    //qDebug() << "AssetWebSurface::SetSrc()" << base << u;
    Asset::SetSrc(base, u);
    if (original_url.isEmpty()) {
        original_url = props->GetSrcURL();
    }    
}

void AssetWebSurface::Load()
{
    //qDebug() << "AssetWebSurface::Load()" << props->GetSrcURL();
    if (webview) {        
        webview->load(props->GetSrcURL());
    }
}

void AssetWebSurface::Reload()
{
    if (webview) {
        webview->reload();
    }
}

void AssetWebSurface::Unload()
{
    if (webview) {
        delete webview;
        webview = NULL;
    }
}

bool AssetWebSurface::GetLoadStarted()
{
    return load_started;
}

bool AssetWebSurface::GetLoaded() const
{
    return loaded;
}

float AssetWebSurface::GetProgress() const
{
    return progress;
}

bool AssetWebSurface::GetFinished() const
{
    return loaded;
}

void AssetWebSurface::SetURL(const QString & u)
{
    //qDebug() << "AssetWebSurface::SetURL()" << u;
    QUrl url(u);
    if (webview && webview->url() != url) {
        //webview->setHtml(QString(), QUrl()); //32.9: fixes a bug relating to being at HTTPS link
        webview->setUrl(url);
        if (original_url.isEmpty()) {
            original_url = u;
        }     
    }
}

QString AssetWebSurface::GetURL() const
{
    //qDebug() << "AssetWebSurface::GetURL()" << u;
    if (webview) {
        return webview->url().toString();
    }
    else {
        return QString();
    }
}

void AssetWebSurface::SetOriginalURL(const QString & s)
{
    original_url = QUrl(s);
}

QString AssetWebSurface::GetOriginalURL() const
{
    return original_url.toString();
}

QUrl AssetWebSurface::GetLinkClicked(const int cursor_index)
{
    if (webview == NULL) {
        return QUrl("");
    }

    if (!hit_test_result[cursor_index].isNull()) {
        if (!hit_test_result[cursor_index].linkUrl().isEmpty()) {
            return hit_test_result[cursor_index].linkUrl();
        }
        else if (!hit_test_result[cursor_index].mediaUrl().isEmpty()) {
            return hit_test_result[cursor_index].mediaUrl();
        }
        else if (!hit_test_result[cursor_index].imageUrl().isEmpty()) {
            return hit_test_result[cursor_index].imageUrl();
        }
        else {
            return QUrl("");
        }
    }
    else {
        return QUrl("");
    }
}

void AssetWebSurface::mousePressEvent(QMouseEvent * e, const int )
{
    //qDebug() << "AssetWebSurface::mousePressEvent" << this << e->pos();    
    if (webview) {
        QMouseEvent * e2 = new QMouseEvent(QEvent::MouseButtonPress, e->pos(), e->button(), e->buttons(), e->modifiers());
        SendWebViewMouseEvent(e2);
    }
}

void AssetWebSurface::mouseMoveEvent(QMouseEvent * e, const int cursor_index)
{
    //qDebug() << "AssetWebSurface::mouseMoveEvent" << this << e->pos();    
    if (webview) {        
        QMouseEvent * e2 = new QMouseEvent(QEvent::MouseMove, e->pos(), e->button(), e->buttons(), e->modifiers());
        SendWebViewMouseEvent(e2);
    }
}

void AssetWebSurface::mouseReleaseEvent(QMouseEvent * e, const int cursor_index)
{
    //qDebug() << "AssetWebSurface::mouseReleaseEvent" << this << e->pos();    
    if (webview) {
        QMouseEvent * e2 = new QMouseEvent(QEvent::MouseButtonRelease, e->pos(), e->button(), e->buttons(), e->modifiers());
        SendWebViewMouseEvent(e2);
    }
}

void AssetWebSurface::SendWebViewMouseEvent(QMouseEvent * e)
{
    QWidget* eventsReceiverWidget = nullptr;
    foreach(QObject* obj, webview->children())
    {
        QWidget* wgt = qobject_cast<QWidget*>(obj);
        if (wgt)
        {
            eventsReceiverWidget = wgt;
            break;
        }
    }

    if (eventsReceiverWidget != nullptr) {
        //event e must be heap allocated, and will be deallocated by QCoreApplication once processed
        QCoreApplication::postEvent(eventsReceiverWidget, e);
    }
}

void AssetWebSurface::keyPressEvent(QKeyEvent * e)
{    
    QKeyEvent * e2 = new QKeyEvent(QEvent::KeyPress, e->key(), e->modifiers(), e->text());
    SendWebViewKeyEvent(e2);
}

void AssetWebSurface::keyReleaseEvent(QKeyEvent * e)
{
    qDebug() << "AssetWebSurface::keyReleaseEvent" << e->key();
    QKeyEvent * e2 = new QKeyEvent(QEvent::KeyRelease, e->key(), e->modifiers());
    SendWebViewKeyEvent(e2);
}

void AssetWebSurface::SendWebViewKeyEvent(QKeyEvent * e)
{
    QWidget* eventsReceiverWidget = nullptr;
    foreach(QObject* obj, webview->children())
    {
        QWidget* wgt = qobject_cast<QWidget*>(obj);
        if (wgt)
        {
            eventsReceiverWidget = wgt;
            break;
        }
    }

    if (eventsReceiverWidget != nullptr) {
        //event e must be heap allocated, and will be deallocated by QCoreApplication once processed
        QCoreApplication::postEvent(eventsReceiverWidget, e);
    }
}

void AssetWebSurface::wheelEvent(QWheelEvent * e)
{
    if (webview) {
        if (e->orientation() == Qt::Vertical){
            webview->page()->runJavaScript(QString("window.scrollTo(%1, %2);").arg(0).arg(webview->page()->scrollPosition().y()-e->delta()));
        }
        if (e->orientation() == Qt::Horizontal){
            webview->page()->runJavaScript(QString("window.scrollTo(%1, %2);").arg(webview->page()->scrollPosition().x()+e->delta()).arg(0));
        }
    }
}

void AssetWebSurface::GoForward()
{
    if (webview) {
        webview->forward();
    }
}

void AssetWebSurface::GoBack()
{
    if (webview) {
        webview->back();
    }
}

void AssetWebSurface::SetSize(const int w, const int h)
{
    props->SetWidth(w);
    props->SetHeight(h);
    webview->resize(QSize(w, h));
}

TextureHandle* AssetWebSurface::GetTextureHandle() const
{
    return m_texture_handle ? m_texture_handle : AssetImage::null_image_tex_handle;
}

void AssetWebSurface::UpdateImage()
{
    if (webview->url().toString() != "about:blank") {
        if (image.size() != webview->size()) {
            image = QImage(webview->size(), QImage::Format_RGB32);
        }
        QPainter painter(&image);
        webview->page()->view()->render(&painter);
        painter.end();
        update_texture = true;
    }
}

void AssetWebSurface::LoadStarted()
{
    //qDebug() << "AssetWebSurface::LoadStarted()";
    load_started = true;
    loaded = false;
    webview->showMinimized();
}

void AssetWebSurface::LoadProgress(int p)
{
    //qDebug() << "AssetWebSurface::LoadProgress" << p;
    progress = float(p) / 100.0f;
}

void AssetWebSurface::LoadFinished(bool b)
{
    //qDebug() << "AssetWebSurface::LoadFinished()" << b;
    loaded = true;
    progress = 1.0f;   
}

void AssetWebSurface::UpdateGL()
{    
    if (loaded && !timer.isActive()) {
        timer.start(33);
    }

    if (update_texture) {
        //qDebug() << "AssetWebSurface::UpdateGL()" << webview->size();
        update_texture = false;

        if (m_texture_handle == nullptr || m_texture_handle == AssetImage::null_image_tex_handle) {
            m_texture_handle = Renderer::m_pimpl->CreateTextureQImage(image, true, true, false, TextureHandle::ALPHA_TYPE::BLENDED, TextureHandle::COLOR_SPACE::SRGB);
        }
        else {
            Renderer::m_pimpl->UpdateTextureHandleData(m_texture_handle, 0, 0, 0,
                    image.width(), image.height(), GL_BGRA, GL_UNSIGNED_BYTE,
                    (void *)image.constBits(), image.width() * image.height() * 4);
            Renderer::m_pimpl->GenerateTextureHandleMipMap(m_texture_handle);
        }
    }
}

WebHitTestResult AssetWebSurface::GetHitTestResult(const int cursor_index) const
{
    return hit_test_result[cursor_index];
}

QRect AssetWebSurface::GetHitTestResultRect(const int cursor_index) const
{
    return hit_test_result[cursor_index].boundingRect();
}

QPointer<QWebEngineView> AssetWebSurface::GetWebView()
{
    return webview;
}

bool AssetWebSurface::GetTextEditing()
{
    //return webview ? webview->getTextEditing() : false;
    return webview->hasFocus();
}

void AssetWebSurface::SetFocus(const bool b)
{    
    if (webview) {
        if (b) {
            webview->setFocus(Qt::MouseFocusReason);
        }
        else {
            webview->clearFocus();
        }
    }
}

bool AssetWebSurface::GetFocus() const
{
    return webview ? webview->hasFocus() : false;
}
