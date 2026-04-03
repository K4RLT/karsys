#include "scenewidget.h"
#include "SceneWallpaper.hpp"
#include "SceneWallpaperSurface.hpp"
#include "glExtra.hpp"
#include <QOpenGLContext>
#include <QDebug>
#include <unistd.h>

using namespace ddplugin_videowallpaper;

static void* gl_get_proc(const char *name) {
    auto *ctx = QOpenGLContext::currentContext();
    return ctx ? reinterpret_cast<void*>(ctx->getProcAddress(name)) : nullptr;
}

SceneWidget::SceneWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_scene(std::make_shared<wallpaper::SceneWallpaper>())
    , m_glex(std::make_unique<GlExtra>())
    , m_timer(new QTimer(this))
{
    m_scene->init();
    connect(m_timer, &QTimer::timeout, this, [this]{ update(); });
    m_timer->start(33);
}

SceneWidget::~SceneWidget() { stop(); }

void SceneWidget::loadScene(const QString &folderPath, const QString &assetsPath)
{
    m_folderPath = folderPath;
    m_assetsPath = assetsPath;
    m_scene->setPropertyString(wallpaper::PROPERTY_SOURCE, folderPath.toStdString());
    m_scene->setPropertyString(wallpaper::PROPERTY_ASSETS, assetsPath.toStdString());
    if (m_vulkanInited) m_scene->play();
}

void SceneWidget::stop()
{
    m_timer->stop();
    m_scene->pause();
}

void SceneWidget::initializeGL()
{
    initializeOpenGLFunctions();
    if (!m_glex->init(gl_get_proc)) {
        qWarning() << "SceneWidget: GlExtra init failed";
        return;
    }
    wallpaper::RenderInitInfo info;
    info.offscreen        = true;
    info.offscreen_tiling = m_glex->tiling();
    info.uuid             = m_glex->uuid();
    info.width            = (uint16_t)(width() * devicePixelRatio());
    info.height           = (uint16_t)(height() * devicePixelRatio());
    info.redraw_callback  = [this]() { QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection); };
    m_scene->initVulkan(info);
    m_vulkanInited = true;
    if (!m_folderPath.isEmpty()) m_scene->play();
}

void SceneWidget::resizeGL(int w, int h)
{
    m_sceneW = w * devicePixelRatio();
    m_sceneH = h * devicePixelRatio();
}

void SceneWidget::paintGL()
{
    if (!m_vulkanInited || !m_scene->inited()) return;
    auto *swp = m_scene->exSwapchain();
    if (!swp) return;
    wallpaper::ExHandle *exh = swp->eatFrame();
    if (!exh) return;
    int id = exh->id();
    if (m_texMap.find(id) == m_texMap.end()) {
        uint gltex = m_glex->genExTexture(*exh);
        ::close(exh->fd);
        m_texMap[id] = gltex;
    }
    uint texId = m_texMap[id];
    glBindTexture(GL_TEXTURE_2D, texId);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(-1,-1);
    glTexCoord2f(1,0); glVertex2f( 1,-1);
    glTexCoord2f(1,1); glVertex2f( 1, 1);
    glTexCoord2f(0,1); glVertex2f(-1, 1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
