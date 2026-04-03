#include "enginemanager.h"
#include "videoapplier.h"
#include "sceneapplier.h"

EngineManager::EngineManager(QObject *parent)
    : QObject(parent)
    , m_videoApplier(new VideoApplier)
    , m_sceneApplier(new SceneApplier)
{
}

BaseApplier *EngineManager::applierFor(const QString &type) const
{
    if (type == "Video") return m_videoApplier;
    if (type == "Scene") return m_sceneApplier;
    return nullptr;
}

bool EngineManager::isSupported(const QString &type) const
{
    BaseApplier *a = applierFor(type);
    return a && a->isAvailable();
}

bool EngineManager::apply(const WallpaperInfo &info, const WallpaperSettings &settings)
{
    BaseApplier *a = applierFor(info.type);

    if (!a) {
        emit failed(tr("No engine available for type: %1").arg(info.type));
        return false;
    }
    if (!a->isAvailable()) {
        emit failed(tr("'%1' engine is not installed.").arg(a->engineName()));
        return false;
    }
    if (!a->apply(info, settings)) {
        emit failed(tr("Failed to apply wallpaper via '%1' engine.").arg(a->engineName()));
        return false;
    }

    emit applied(info);
    return true;
}
