# Phase 2 — Engine Manager Files

## Files included
- baseapplier.h       — pure interface, both engines implement this
- videoapplier.h/cpp  — writes to DConfig, triggers the desktop plugin
- sceneapplier.h/cpp  — stub for linux-wallpaperengine (Phase 3)
- enginemanager.h/cpp — picks the right engine by wallpaper type

## Drop into your project folder, then add to CMakeLists.txt:

set(SOURCES
    ...existing sources...
    enginemanager.cpp
    videoapplier.cpp
    sceneapplier.cpp
)

set(HEADERS
    ...existing headers...
    baseapplier.h
    enginemanager.h
    videoapplier.h
    sceneapplier.h
)

## Also add resolveVideoPath to wallpaperscanner:

# wallpaperscanner.h — add this static method:
static QString resolveVideoPath(const QString &folderPath);

# wallpaperscanner.cpp — implement it:
QString WallpaperScanner::resolveVideoPath(const QString &folderPath)
{
    static const QStringList kVideoExts { "*.mp4", "*.webm", "*.mkv", "*.avi", "*.mov" };
    const QFileInfoList files = QDir(folderPath).entryInfoList(kVideoExts, QDir::Files);
    return files.isEmpty() ? QString() : files.constFirst().absoluteFilePath();
}

## mainwindow.h — add:
#include "enginemanager.h"
// in private section:
EngineManager *m_engineManager { nullptr };
WallpaperSettings m_settings;
// in private slots:
void onApplyWallpaper(const WallpaperInfo &info);

## mainwindow.cpp — constructor:
m_engineManager = new EngineManager(this);
connect(m_engineManager, &EngineManager::applied, this, [this](const WallpaperInfo &info) {
    // show toast: "Now playing: " + info.title
});
connect(m_engineManager, &EngineManager::failed, this, [this](const QString &reason) {
    QMessageBox::warning(this, tr("Apply failed"), reason);
});

## mainwindow.cpp — slot:
void MainWindow::onApplyWallpaper(const WallpaperInfo &info)
{
    m_engineManager->apply(info, m_settings);
}

## wallpapercard signal (wherever your apply button/double-click is):
emit applyRequested(m_info);

## connect in MainWindow:
connect(card, &WallpaperCard::applyRequested,
        this, &MainWindow::onApplyWallpaper);

## Also patch the plugin (wallpaperengine.cpp) — two functions:

### resolveVideoUrl — add absolute path check at top:
static QUrl resolveVideoUrl(const QList<QUrl> &videos)
{
    const QString selected = WpCfg->videoPath();
    if (!selected.isEmpty() && QFileInfo::exists(selected))
        return QUrl::fromLocalFile(selected);         // <-- direct path, bypass list
    if (!selected.isEmpty()) {
        for (const QUrl &v : videos) {
            if (v.toLocalFile() == selected) return v;
        }
    }
    if (!videos.isEmpty()) {
        WpCfg->setVideoPath(videos.constFirst().toLocalFile());
        return videos.constFirst();
    }
    return QUrl();
}

### refreshSource — add direct-path bypass before the empty bail:
void WallpaperEngine::refreshSource()
{
    d->videos = d->getVideos(d->sourcePath());

    const QString directPath = WpCfg->videoPath();
    if (d->videos.isEmpty() && !directPath.isEmpty() && QFileInfo::exists(directPath)) {
        d->applyScaleMode(WpCfg->scaleMode());
        for (const VideoProxyPointer &bwp : d->widgets.values()) {
            bwp->setMpvProperty("pause", false);
            bwp->command(QVariantList { "loadfile", directPath });
        }
        return;
    }
    // ... rest of original refreshSource unchanged
}
