#include "filteredcubemapmanager.h"

FilteredCubemapManager::FilteredCubemapManager()
    : m_processing_active(false),
      shutdown(false)
{
}

FilteredCubemapManager::~FilteredCubemapManager()
{
    for (Cubemaps& cubemap : m_pending_cubemaps)
    {
        RemoveTemporaryFiles(cubemap);
    }

    for (Cubemaps& cubemap : m_pending_deletions)
    {
        RemoveCompletedCubemaps(cubemap);
    }
}

bool FilteredCubemapManager::TestLoadImage()
{
    cmft::Image image;

    bool image_loaded = false;

    // bool imageLoad(Image& _image, const char* _filePath, TextureFormat::Enum _convertTo, bx::AllocatorI* _allocator)
    image_loaded = (cmft::imageLoad(image,
                                   "C:/Users/Devli/Documents/JanusVR_Win64/build-FireBox-Desktop_Qt_5_5_1_MSVC2013_64bit-Debug/debug/workspace/example1/5710610581124073051_cubemap_radiance256.dds",
                                   cmft::TextureFormat::RGBA32F)
                    || cmft::imageLoadStb(image,
                                            "C:/Users/Devli/Documents/JanusVR_Win64/build-FireBox-Desktop_Qt_5_5_1_MSVC2013_64bit-Debug/debug/workspace/example1/5710610581124073051_cubemap_radiance256.dds",
                                            cmft::TextureFormat::RGBA32F));

    return image_loaded;
}

bool FilteredCubemapManager::RegisterForProcessing(qint64 p_key, Cubemaps& p_cubemaps)
{
    mutex.lock(); //

    auto pending_iterator = m_pending_cubemaps.find(p_key);

    // If we don't already have this key then add it
    if (pending_iterator == m_pending_cubemaps.end())
    {
        m_pending_cubemaps.insert(p_key, p_cubemaps);
        qDebug() << "INFO: FilteredCubemapManager::register_for_processing" << p_key << "now pending processing";
        mutex.unlock();
        return true;
    }
    else
    {
        switch (pending_iterator.value().m_processing_state)
        {
        // If we do but we have not started processing it then replace the cube paths with the updated ones
        case PROCESSING_STATE::PENDING:
            qDebug() << "INFO: FilteredCubemapManager::register_for_processing" << p_key << "now pending processing via m_cube_maps update";
            mutex.unlock();
            return true;
            break;
        // If we are currently processing this key or it is done then move it to the pending deletions queue
        // and add the new cube faces into the pending processing list as we need to delete the old temporary
        // files before creating new ones for this key to avoid naming conflicts
        case PROCESSING_STATE::INVALID:
        case PROCESSING_STATE::PROCESSING:
        case PROCESSING_STATE::READY:
            auto deletion_itr = m_pending_deletions.find(p_key);
            if (deletion_itr == m_pending_deletions.end())
            {
                m_pending_deletions.insert(p_key, pending_iterator.value());
                qDebug() << "INFO: FilteredCubemapManager::register_for_processing" << p_key << "now pending deletion";
            }
            mutex.unlock();
            return false;
            break;
        }
    }
    mutex.unlock();
    return false;
}

PROCESSING_STATE FilteredCubemapManager::GetProcessingState(qint64 p_key)
{
    mutex.lock();
    auto itr = m_pending_cubemaps.find(p_key);
    mutex.unlock();
    return itr != m_pending_cubemaps.end() ? itr.value().m_processing_state : PROCESSING_STATE::INVALID;
}

void FilteredCubemapManager::RemoveFromProcessing(qint64 p_key, bool p_delete_cubemaps)
{
    mutex.lock();
//    qDebug() << "INFO: FilteredCubemapManager::remove_from_processing key" << p_key << " delete_cubemaps" << p_delete_cubemaps;

    const auto itr = m_pending_cubemaps.find(p_key);
    if (itr != m_pending_cubemaps.end())
    {
        if (p_delete_cubemaps == true)
        {
            m_pending_deletions.insert(p_key, itr.value());
        }

        if (itr.value().m_processing_state != PROCESSING_STATE::PROCESSING)
        {
            m_pending_cubemaps.erase(itr);
        }
    }

    mutex.unlock();
}

void FilteredCubemapManager::RadianceIrradianceFilter(qint64 p_key, Cubemaps& p_cubemaps, bool* p_processing_active)
{  
    qDebug() << "INFO: FilteredCubemapManager::radiance_irradiance_filter key" << p_key;    
    mutex.lock();
    (*p_processing_active) = true;
    p_cubemaps.m_processing_state = PROCESSING_STATE::PROCESSING;
    mutex.unlock();

    QtConcurrent::run(this, &FilteredCubemapManager::GenerateRadianceAndIrradianceMaps, p_key, &p_cubemaps, p_processing_active);
}

void FilteredCubemapManager::GenerateRadianceAndIrradianceMaps(qint64 p_key, Cubemaps* p_cubemaps, bool* p_processing_active)
{
    Q_UNUSED(p_key);

// STAGE 2: Generate radiance and irradiance cubemaps with CMFT then save to local DDS files for use in room.
    // Load dds files from memory into cmft
    cmft::Image image_face_list_radiance[6];
    bool image_loaded = false;
    cmft::TextureFormat::Enum cmftFormat = (p_cubemaps->m_channel_size == 8) ? cmft::TextureFormat::BGR8 : cmft::TextureFormat::RGBA16F;

    image_loaded = cmft::imageLoad(image_face_list_radiance[0], p_cubemaps->m_dds_data[0].data(), p_cubemaps->m_dds_data[0].size(), cmftFormat)
        && cmft::imageLoad(image_face_list_radiance[1], p_cubemaps->m_dds_data[1].data(), p_cubemaps->m_dds_data[1].size(), cmftFormat)
        && cmft::imageLoad(image_face_list_radiance[2], p_cubemaps->m_dds_data[2].data(), p_cubemaps->m_dds_data[2].size(), cmftFormat)
        && cmft::imageLoad(image_face_list_radiance[3], p_cubemaps->m_dds_data[3].data(), p_cubemaps->m_dds_data[3].size(), cmftFormat)
        && cmft::imageLoad(image_face_list_radiance[4], p_cubemaps->m_dds_data[4].data(), p_cubemaps->m_dds_data[4].size(), cmftFormat)
        && cmft::imageLoad(image_face_list_radiance[5], p_cubemaps->m_dds_data[5].data(), p_cubemaps->m_dds_data[5].size(), cmftFormat);

//    qDebug() << "FilteredCubemapManager::GenerateRadianceAndIrradianceMaps image_loaded" << image_loaded;

    // Create cmft cubemap from face images
    cmft::Image radiance_image;
    cmft::Image irradiance_image;
    if (image_loaded == true)
    {
        cmft::imageCubemapFromFaceList(radiance_image, image_face_list_radiance);
        cmft::imageCubemapFromFaceList(irradiance_image, image_face_list_radiance);
    }

// IRRADIANCE FILTERING
    // Resize if requested.
    if (irradiance_image.m_width != 64)
    {
        cmft::imageResize(irradiance_image, 64, 64);
    }

    // Convert to linear if we are an sRGB image
    if (p_cubemaps->m_channel_size == 8)
    {
        cmft::imageApplyGamma(irradiance_image, 2.2f);
    }

    // Perform Irradiance filter on irradiance_image
    cmft::imageIrradianceFilterSh(irradiance_image, 64);

    // Generate mip map chain
    cmft::imageGenerateMipMapChain(irradiance_image);

    // Convert to sRGB if we ouputting to an 8-bit image
    // this is done after mipmapping so that mipmapping is not performed in sRGB space
    if (p_cubemaps->m_channel_size == 8)
    {
        cmft::imageApplyGamma(irradiance_image, 0.45f);
    }

// RADIANCE FILTERING
    // Resize to 256 to keep processing time in check
    if (radiance_image.m_width != 256)
    {
        cmft::imageResize(radiance_image, 256, 256);
    }

    // Convert to linear if we are an sRGB image
    if (p_cubemaps->m_channel_size == 8)
    {
        cmft::imageApplyGamma(radiance_image, 2.2f);
    }

    // Perform Radiance filter on image
    cmft::imageRadianceFilter(radiance_image
                      , 256
                      , cmft::LightingModel::BlinnBrdf
                      , true
                      , 9
                      , 10
                      , 3
                      , cmft::EdgeFixup::None
                      , (QThread::idealThreadCount() < 3) ? (1) : (QThread::idealThreadCount() - 2));

    // Generate mip map chain
    cmft::imageGenerateMipMapChain(radiance_image);

    // Convert to sRGB if we ouputting to an 8-bit image
    // this is done after mipmapping so that mipmapping is not performed in sRGB space
    if (p_cubemaps->m_channel_size == 8)
    {
        cmft::imageApplyGamma(radiance_image, 0.45f);
    }

    // Save filtered cubemaps to cached dds files so they can be reloaded and converted into texturehandles
    cmft::imageSave(radiance_image, p_cubemaps->m_cube_maps[1].toStdString().c_str(),
                    cmft::ImageFileType::DDS, cmft::OutputType::Cubemap, cmftFormat,
                    true);

    cmft::imageSave(irradiance_image, p_cubemaps->m_cube_maps[0].toStdString().c_str(),
                    cmft::ImageFileType::DDS, cmft::OutputType::Cubemap, cmftFormat,
                    true);

    // Clean up gli allocated data
    cmft::imageUnload(radiance_image);
    cmft::imageUnload(irradiance_image);

    p_cubemaps->clear_DDS_data();

    mutex.lock();
    p_cubemaps->m_processing_state = PROCESSING_STATE::READY;
    *p_processing_active = false;
    mutex.unlock();
//    qDebug() << "INFO: FilteredCubemapManager::radiance_irradiance_filter completed key" << p_key << p_cubemaps->m_cube_maps[0].toStdString().c_str() << p_cubemaps->m_cube_maps[1].toStdString().c_str();
}

void FilteredCubemapManager::RemoveTemporaryFiles(Cubemaps& p_cubemaps)
{
    for (int i = 0; i < p_cubemaps.m_cube_maps.size(); ++i)
    {
        QFile file(p_cubemaps.m_cube_maps[i]);
        if (file.exists())
        {
            file.remove();
        }
    }
}

void FilteredCubemapManager::RemoveCompletedCubemaps(Cubemaps& p_cubemaps)
{
    for (int i = 0; i < p_cubemaps.m_cube_maps.size(); ++i)
    {
        QFile file(p_cubemaps.m_cube_maps[i] + ".dds");
        if (file.exists())
        {
            file.remove();
        }
    }
}

void FilteredCubemapManager::Update()
{
    if (SettingsManager::GetUpdateCMFT()) {
        while (!shutdown)
        {
            if (m_processing_active == false && m_pending_cubemaps.size() != 0)
            {
                auto itr = m_pending_cubemaps.begin();
                while (itr != m_pending_cubemaps.end())
                {
                    if (itr.value().m_processing_state == PROCESSING_STATE::PENDING)
                    {
                        intptr_t key = itr.key();
                        Cubemaps& cubemap_to_process = itr.value();
                        m_processing_active = true;
                        RadianceIrradianceFilter(key, cubemap_to_process, &m_processing_active);
                        break;
                    }
                    else
                    {
                       itr++;
                    }
                }
            }

            // Delete any files generated for cubemaps that are no longer needed
            // This can be because the Room signaled that it finished loading the radiance/irradiance maps,
            // or because the Room has requested new cubemaps while it was already processing
            if (m_pending_deletions.size() != 0)
            {
                for (Cubemaps& cubemap_to_delete : m_pending_deletions)
                {
                    RemoveCompletedCubemaps(cubemap_to_delete);
                }
                m_pending_deletions.clear();
            }

            // 250 millisecond sleep to avoid this thread from taking up more processing than necessary
            QThread::msleep(250);
        }
    }
}

void FilteredCubemapManager::Initialize()
{
    QtConcurrent::run(this, &FilteredCubemapManager::Update);
}

void FilteredCubemapManager::SetShutdown(const bool b)
{
    shutdown = b;
}



