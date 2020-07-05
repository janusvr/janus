#include "performancelogger.h"

QVector <PerformanceLoggerSample> PerformanceLogger::frame_samples;
int PerformanceLogger::max_frame_samples = 180;
clock_t PerformanceLogger::last_clock = 0;

PerformanceLogger::PerformanceLogger() :
    samples_num(0),
    m_total_main_CPU_time(0),
    m_average_render_GPU_time(0),
    m_average_render_CPU_time(0),
    m_average_main_CPU_time(0)
{    
    frame_samples.resize(max_frame_samples);
    main_thread_cpu_time_samples.resize(max_frame_samples);    
    m_frame_time_timer.start();    
}

void PerformanceLogger::StartFrameSample()
{
    last_clock = clock();
}

void PerformanceLogger::EndFrameSample()
{
    uint32_t const results_index = samples_num % max_frame_samples;    
    ++samples_num;

    main_thread_cpu_time_samples[results_index] = double(m_frame_time_timer.nsecsElapsed() / 1000000);
    m_frame_time_timer.start();

    m_average_render_GPU_time = 0.0;
    m_average_render_CPU_time = 0.0;
    m_average_main_CPU_time = 0.0;

    for (int sample_index = 0; sample_index < max_frame_samples; ++sample_index)
    {
        m_average_render_GPU_time += static_cast<double>(frame_samples[sample_index].render_thread_gpu_time);
        m_average_render_CPU_time += static_cast<double>(frame_samples[sample_index].render_thread_cpu_time);
        m_average_main_CPU_time += static_cast<double>(main_thread_cpu_time_samples[sample_index]);
    }

    m_average_render_GPU_time /= max_frame_samples;
    m_average_render_CPU_time /= max_frame_samples;
    m_average_main_CPU_time /= max_frame_samples;
}

void PerformanceLogger::SetGPUTimeQueryResults(QVector<GLuint64> & v)
{
    for (int i=0; i<frame_samples.size(); ++i) {
        if (i < v.size()) {
            frame_samples[i].render_thread_gpu_time = double(v[i] / 1000000.0);
        }
        else {
            frame_samples[i].render_thread_gpu_time = 0.0;
        }
    }
}

void PerformanceLogger::SetCPUTimeQueryResults(QVector<uint64_t>& v)
{
    for (int i=0; i<frame_samples.size(); ++i) {
        if (i < v.size()) {
            frame_samples[i].render_thread_cpu_time = double(v[i] / 1000000.0);
        }
        else {
            frame_samples[i].render_thread_cpu_time = 0.0;
        }
    }
}

void PerformanceLogger::SetNumFrameSamples(const int i)
{
    max_frame_samples = i;
}

int PerformanceLogger::GetNumFrameSamples() const
{
    return max_frame_samples;
}

TextureHandle* PerformanceLogger::GetFrameSamplesTextureHandle()
{
    const unsigned int perf_height = 44;
    QPixmap pixmap(max_frame_samples, perf_height);
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter p(&pixmap);
    QVector<QRect> render_thread_gpu_time_rects;
    render_thread_gpu_time_rects.reserve(max_frame_samples);
    QVector<QRect> render_thread_cpu_time_rects;
    render_thread_cpu_time_rects.reserve(max_frame_samples);
    QVector<QRect> main_thread_cpu_time_rects;
    main_thread_cpu_time_rects.reserve(max_frame_samples);

    //qDebug() << frame_samples;
    for (int i=0; i<frame_samples.size(); ++i) {
        QRect gpu_r(i, 0, 1, frame_samples[i].render_thread_gpu_time);
        QRect cpu_r(i, 0, 1, frame_samples[i].render_thread_cpu_time);
        QRect cpu_m(i, 0, 1, main_thread_cpu_time_samples[i]);

        render_thread_gpu_time_rects.push_back(gpu_r);
        render_thread_cpu_time_rects.push_back(cpu_r);
        main_thread_cpu_time_rects.push_back(cpu_m);
    }

    p.setPen(QPen(QColor(0, 0, 255, 255)));
    p.drawRects(main_thread_cpu_time_rects.data(), max_frame_samples);

    p.setPen(QPen(QColor(255, 255, 255, 255)));
    p.drawRects(render_thread_cpu_time_rects.data(), max_frame_samples);

    p.setPen(QPen(QColor(0, 255, 0, 255)));
    p.drawRects(render_thread_gpu_time_rects.data(), max_frame_samples);

    p.setPen(QPen(QColor(255, 255 ,255, 255)));
    p.drawLine(0, 11, max_frame_samples, 11);

    p.setPen(QPen(QColor(255, 255, 255, 255)));
    p.drawLine(0, 22, max_frame_samples, 22);

    p.end();

    QImage perfTex = pixmap.toImage();
    perfTex = perfTex.convertToFormat(QImage::Format_RGBA8888_Premultiplied);

    if (!m_texture_handle)
    {
        m_texture_handle = Renderer::m_pimpl->CreateTextureQImage(perfTex, true, false, true, TextureHandle::ALPHA_TYPE::CUTOUT, TextureHandle::COLOR_SPACE::SRGB);
    }

    Renderer::m_pimpl->UpdateTextureHandleData(m_texture_handle, 0, 0, 0, perfTex.width(), perfTex.height(), GL_RGBA, GL_UNSIGNED_BYTE, (void *)perfTex.constBits(), perfTex.width() * perfTex.height() * 4);
    Renderer::m_pimpl->GenerateTextureHandleMipMap(m_texture_handle);
    return m_texture_handle;
}

double PerformanceLogger::GetAverageMainThreadCPUTime() const
{
    return m_average_main_CPU_time;
}

double PerformanceLogger::GetAverageRenderThreadCPUTime() const
{
    return m_average_render_CPU_time;
}

double PerformanceLogger::GetAverageRenderThreadGPUTime() const
{
    return m_average_render_GPU_time;
}

