#ifndef ASSETWEBSURFACE_H
#define ASSETWEBSURFACE_H

#include <QtGui>
#include <QtWebEngine>
#include <QtWebEngineWidgets>

#include "asset.h"
#include "mathutil.h"
#include "cookiejar.h"
#include "renderer.h"
#include "rendererinterface.h"
#include "performancelogger.h"
#include "assetimage.h"

struct WebHitTestResult
{
    QUrl link_url;
    QUrl media_url;
    QUrl image_url;

    QRect bounding_rect;

    bool editable;
    bool selected;
    bool is_null;

    QUrl linkUrl() const
    {
        return link_url;
    }

    QUrl mediaUrl() const
    {
        return media_url;
    }

    QUrl imageUrl() const
    {
        return image_url;
    }

    bool isContentEditable() const
    {
        return editable;
    }

    bool isContentSelected() const
    {
        return selected;
    }

    QRect boundingRect() const
    {
        return bounding_rect;
    }

    bool isNull() const
    {
        return is_null;
    }
};

class AssetWebSurface : public Asset
{
    Q_OBJECT

public:

    AssetWebSurface();
    ~AssetWebSurface();

    void SetProperties(const QVariantMap & d);

    void SetTextureAlpha(const bool b);

    void SetSrc(const QString & base, const QString & url_str);

    void SetURL(const QString & u);
    QString GetURL() const;

    void SetOriginalURL(const QString & u);
    QString GetOriginalURL() const;

    void Load();
    void Reload();
    void Unload();

    bool GetLoadStarted();
    bool GetLoaded() const;
    float GetProgress() const;
    bool GetFinished() const;

    void mousePressEvent(QMouseEvent * e, const int cursor_index);
    void mouseMoveEvent(QMouseEvent * e, const int cursor_index);
    void mouseReleaseEvent(QMouseEvent * e, const int cursor_index);
    void wheelEvent(QWheelEvent * e);

    void keyPressEvent(QKeyEvent * e);
    void keyReleaseEvent(QKeyEvent * e);

    void GoForward();
    void GoBack();

    void SetSize(const int w, const int h);

    TextureHandle* GetTextureHandle() const;
    void UpdateGL();

    QUrl GetLinkClicked(const int cursor_index);
    WebHitTestResult GetHitTestResult(const int cursor_index) const;

    QPointer <QWebEngineView> GetWebView();

    QRect GetHitTestResultRect(const int cursor_index) const;
    bool GetTextEditing();

    void SetFocus(const bool b);
    bool GetFocus() const;

public slots: 

    void LoadStarted();
    void LoadProgress(int p);
    void LoadFinished(bool b);

    void UpdateImage();

private:   

    void SendWebViewMouseEvent(QMouseEvent * e);
    void SendWebViewKeyEvent(QKeyEvent * e);

    QUrl original_url;

    QPointer <QWebEngineView> webview;
    QImage image;

    bool tex_alpha;
	QPointer<TextureHandle> m_texture_handle;

    WebHitTestResult hit_test_result[2];

    bool load_started;
    bool loaded;
    float progress;    

    bool update_texture;

    QTimer timer;
};

#endif // ASSETWEBSURFACE_H
