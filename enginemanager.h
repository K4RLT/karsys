#pragma once
#include "baseapplier.h"
#include "wallpaperscanner.h"
#include "settingsdialog.h"
#include <QObject>
#include <QString>

class EngineManager : public QObject
{
    Q_OBJECT
public:
    explicit EngineManager(QObject *parent = nullptr);

    // Main entry point — picks the right engine automatically by wallpaper type
    bool apply(const WallpaperInfo &info, const WallpaperSettings &settings);

    // Returns true if the backend for a given type is installed and ready
    bool isSupported(const QString &type) const;

signals:
    void applied(const WallpaperInfo &info);
    void failed(const QString &reason);

private:
    BaseApplier *applierFor(const QString &type) const;

    BaseApplier *m_videoApplier { nullptr };
    BaseApplier *m_sceneApplier { nullptr };
    // BaseApplier *m_webApplier { nullptr };  // Phase X
};
