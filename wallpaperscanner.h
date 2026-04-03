#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>

struct WallpaperInfo {
    QString id;
    QString title;
    QString type;       // "video", "scene", "web"
    QString author;
    QString resolution;
    QString previewPath;
    QString folderPath;
    qint64  folderSize; // bytes — for large wallpaper warning
};

class WallpaperScanner : public QObject
{
    Q_OBJECT
public:
    explicit WallpaperScanner(QObject *parent = nullptr);

    // Returns all detected Steam library root paths
    static QStringList detectSteamPaths();

    // Scans the workshop/content/431960 folder and returns wallpaper list
    QList<WallpaperInfo> scan(const QString &steamLibraryPath);

    // Returns the workshop path given a steam library root
    static QString workshopPath(const QString &steamRoot);

    // Finds the first playable video file inside a wallpaper's folder
    static QString resolveVideoPath(const QString &folderPath);

signals:
    void scanProgress(int current, int total);

private:
    WallpaperInfo parseWallpaper(const QString &folderPath);
    QString       findPreview(const QString &folderPath);
    qint64        folderSize(const QString &folderPath);
    QString       parseResolution(const QJsonObject &proj, const QString &folderPath);

    static constexpr const char *WE_APP_ID = "431960";
};
