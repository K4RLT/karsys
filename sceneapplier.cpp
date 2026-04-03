#include "sceneapplier.h"
#include <QProcess>
#include <QDebug>
#include <QDir>

static constexpr char kApp[]  = "org.deepin.dde.file-manager";
static constexpr char kConf[] = "org.deepin.dde.file-manager.desktop.videowallpaper";

static void dset(const QString &key, const QString &value)
{
    QStringList args { "--set", "-a", kApp, "-r", kConf, "-k", key, "-v", value };
    int ret = QProcess::execute("/usr/bin/dde-dconfig", args);
    qDebug() << "dset" << key << "=" << value << "ret:" << ret;
}

bool SceneApplier::isAvailable() const { return true; }

bool SceneApplier::apply(const WallpaperInfo &info, const WallpaperSettings &settings)
{
    qDebug() << "SceneApplier::apply called for" << info.folderPath;
    if (!QDir(info.folderPath).exists()) {
        qWarning() << "SceneApplier: folder does not exist:" << info.folderPath;
        return false;
    }
    dset("wallpaperType",     "scene");
    dset("scenePath",         info.folderPath);
    dset("scaleMode",         QStringList{"fill","fit","stretch","center"}.value(settings.scaleMode, "fill"));
    dset("enableAudio",       settings.audioEnabled ? "true" : "false");
    dset("pauseOnFullscreen", settings.pauseOnFullscreen ? "true" : "false");
    dset("pauseIdleSeconds",  QString::number(settings.pauseOnIdle ? settings.idleTimeoutSec : 0));
    dset("enable",            "true");
    qDebug() << "SceneApplier: done";
    return true;
}
