#include "wallpaperscanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDirIterator>
#include <QImageReader>
#include <QStandardPaths>

WallpaperScanner::WallpaperScanner(QObject *parent)
    : QObject(parent) {}

QStringList WallpaperScanner::detectSteamPaths()
{
    QStringList candidates = {
        QDir::homePath() + "/.local/share/Steam",
        QDir::homePath() + "/.steam/steam",
        QDir::homePath() + "/.var/app/com.valvesoftware.Steam/.local/share/Steam",
        QDir::homePath() + "/snap/steam/common/.local/share/Steam"
    };

    QStringList found;
    for (const QString &path : candidates) {
        if (QDir(workshopPath(path)).exists())
            found << path;
    }
    return found;
}

QString WallpaperScanner::workshopPath(const QString &steamRoot)
{
    return steamRoot + "/steamapps/workshop/content/" + WE_APP_ID;
}

QList<WallpaperInfo> WallpaperScanner::scan(const QString &steamLibraryPath)
{
    QList<WallpaperInfo> results;
    QDir workshopDir(workshopPath(steamLibraryPath));

    if (!workshopDir.exists())
        return results;

    QStringList entries = workshopDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    int total = entries.size();
    int current = 0;

    for (const QString &entry : entries) {
        QString fullPath = workshopDir.absoluteFilePath(entry);
        WallpaperInfo info = parseWallpaper(fullPath);
        if (!info.title.isEmpty())
            results << info;

        emit scanProgress(++current, total);
    }

    return results;
}

WallpaperInfo WallpaperScanner::parseWallpaper(const QString &folderPath)
{
    WallpaperInfo info;
    info.folderPath = folderPath;
    info.id = QFileInfo(folderPath).fileName();

    QString jsonPath = folderPath + "/project.json";
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly))
        return info;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    info.title      = obj.value("title").toString("Unknown");
    info.author     = obj.value("workshopid").toString();
    info.folderSize = folderSize(folderPath);
    info.previewPath = findPreview(folderPath);

    // Type detection
    QString type = obj.value("type").toString().toLower();
    if (type == "video")       info.type = "Video";
    else if (type == "scene")  info.type = "Scene";
    else if (type == "web")    info.type = "Web";
    else                       info.type = "Unknown";

    // Resolution
    info.resolution = parseResolution(obj, folderPath);

    return info;
}

QString WallpaperScanner::findPreview(const QString &folderPath)
{
    // Check project.json preview field first
    QString jsonPath = folderPath + "/project.json";
    QFile file(jsonPath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QString preview = doc.object().value("preview").toString();
        if (!preview.isEmpty()) {
            QString full = folderPath + "/" + preview;
            if (QFile::exists(full))
                return full;
        }
    }

    // Fallback: look for common preview file names
    QStringList names = { "preview.gif", "preview.png", "preview.jpg",
                          "preview.jpeg", "thumbnail.png", "thumbnail.jpg" };
    for (const QString &name : names) {
        QString path = folderPath + "/" + name;
        if (QFile::exists(path))
            return path;
    }
    return QString();
}

qint64 WallpaperScanner::folderSize(const QString &folderPath)
{
    qint64 total = 0;
    QDirIterator it(folderPath, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile())
            total += it.fileInfo().size();
    }
    return total;
}

QString WallpaperScanner::parseResolution(const QJsonObject &proj, const QString &folderPath)
{
    // Some project.json files have width/height
    int w = proj.value("width").toInt(0);
    int h = proj.value("height").toInt(0);
    if (w > 0 && h > 0)
        return QString("%1x%2").arg(w).arg(h);

    // For video wallpapers, read from the video file via QImageReader
    // (basic fallback, won't work for all formats)
    QStringList videoExts = { "*.mp4", "*.webm", "*.avi", "*.mkv" };
    QDir dir(folderPath);
    for (const QString &ext : videoExts) {
        QStringList files = dir.entryList({ ext }, QDir::Files);
        if (!files.isEmpty())
            return "Video"; // resolution needs ffprobe for accurate reading
    }

    return QString();
}

QString WallpaperScanner::resolveVideoPath(const QString &folderPath)
{
    static const QStringList kVideoExts { "*.mp4", "*.webm", "*.mkv", "*.avi", "*.mov" };
    const QFileInfoList files = QDir(folderPath).entryInfoList(kVideoExts, QDir::Files);
    return files.isEmpty() ? QString() : files.constFirst().absoluteFilePath();
}
