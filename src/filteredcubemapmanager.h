#ifndef FILTEREDCUBEMAPMANAGER_H
#define FILTEREDCUBEMAPMANAGER_H

#include "cmft/image.h"
#include "cmft/cubemapfilter.h"
#include "cmft/clcontext.h"
#include "cmft/print.h"

#include <QMap>
#include <QPair>
#include <QVector>
#include <QDebug>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include "mathutil.h"
#include "settingsmanager.h"

class FilteredCubemapManager
{
public:
    FilteredCubemapManager();
    ~FilteredCubemapManager();

    static FilteredCubemapManager* GetSingleton()
    {
        static FilteredCubemapManager * singleton = new FilteredCubemapManager();
        return singleton;
    }

    bool TestLoadImage();
    bool RegisterForProcessing(qint64 p_key, Cubemaps &p_cubemaps);
    PROCESSING_STATE GetProcessingState(qint64 p_key);
    void RemoveFromProcessing(qint64 p_key, bool p_delete_cubemaps);
    void RemoveTemporaryFiles(Cubemaps& p_cubemaps);
    void Update();
    void Initialize();

    void SetShutdown(const bool b);

    QMutex mutex;
    void RemoveCompletedCubemaps(Cubemaps& p_cubemaps);
    void cube_faces_to_cubemap(Cubemaps& p_cubemaps);
    void GenerateRadianceAndIrradianceMaps(qint64 p_key, Cubemaps* p_cubemaps, bool* p_processing_active);
private:

    QMap<qint64, Cubemaps> m_pending_deletions;
    QMap<qint64, Cubemaps> m_pending_cubemaps;
    QPair<qint64, Cubemaps> m_currently_processing_cubemap;
    bool m_processing_active;
    void RadianceIrradianceFilter(qint64 p_key, Cubemaps& p_cubemaps, bool* p_processing_active);
    bool shutdown;
};

#endif // FILTEREDCUBEMAPMANAGER_H
