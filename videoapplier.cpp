#include "videoapplier.h"
#include "wallpaperscanner.h"
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

static constexpr char kApp[]  = "org.deepin.dde.file-manager";
static constexpr char kConf[] = "org.deepin.dde.file-manager.desktop.videowallpaper";

static void dset(const QString &key, const QString &value)
{
    QStringList args { "--set", "-a", kApp, "-r", kConf, "-k", key, "-v", value };
    qDebug() << "dset:" << "/usr/bin/dde-dconfig" << args;
    int ret = QProcess::execute("/usr/bin/dde-dconfig", args);
    qDebug() << "dset result:" << ret;
}

bool VideoApplier::isAvailable() const
{
    return true; // plugin is always present
}

bool VideoApplier::apply(const WallpaperInfo &info, const WallpaperSettings &settings)
{
    qDebug() << "VideoApplier::apply called for" << info.folderPath;

    const QString videoPath = WallpaperScanner::resolveVideoPath(info.folderPath);
    qDebug() << "resolved videoPath:" << videoPath;
    if (videoPath.isEmpty()) {
        qWarning() << "VideoApplier: no video file in" << info.folderPath;
        return false;
    }

    static const QStringList kScaleModes { "fill", "fit", "stretch", "center" };

    dset("videoPath",         videoPath);
    dset("scaleMode",         kScaleModes.value(settings.scaleMode, "fill"));
    dset("enableAudio",       settings.audioEnabled ? "true" : "false");
    dset("pauseOnFullscreen", settings.pauseOnFullscreen ? "true" : "false");
    dset("pauseIdleSeconds",  QString::number(settings.pauseOnIdle ? settings.idleTimeoutSec : 0));
    dset("enable",            "true");

    qDebug() << "VideoApplier: done";
    return true;
}
