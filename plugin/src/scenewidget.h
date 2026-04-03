#pragma once
#include "ddplugin_videowallpaper_global.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <memory>
#include "glExtra.hpp"
#include <unordered_map>

namespace wallpaper { class SceneWallpaper; }
class GlExtra;

DDP_VIDEOWALLPAPER_BEGIN_NAMESPACE

class SceneWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit SceneWidget(QWidget *parent = nullptr);
    ~SceneWidget() override;

    void loadScene(const QString &folderPath, const QString &assetsPath);
    void stop();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void initVulkan();
    std::shared_ptr<wallpaper::SceneWallpaper> m_scene;
    QString m_folderPath;
    QString m_assetsPath;
    uint m_texId = 0;
    int m_sceneW = 1920;
    int m_sceneH = 1080;
    bool m_vulkanInited = false;
    QTimer *m_timer = nullptr;
    std::unique_ptr<GlExtra> m_glex;
    std::unordered_map<int,uint> m_texMap;
};

DDP_VIDEOWALLPAPER_END_NAMESPACE
