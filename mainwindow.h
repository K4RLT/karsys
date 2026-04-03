#pragma once
#include "enginemanager.h"

#include <DMainWindow>
#include <DSearchEdit>
#include <DSpinner>
#include <DLabel>
#include "wallpaperscanner.h"
#include "settingsdialog.h"

DWIDGET_USE_NAMESPACE

class FlowLayout;
class QScrollArea;
class WallpaperCard;
class QSettings;

class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onScanProgress(int current, int total);
    void onScanFinished(const QList<WallpaperInfo> &wallpapers);
    void onSearchChanged(const QString &text);
    void onApplyRequested(const WallpaperInfo &info);
    void onSelectFolderClicked();
    void onSettingsClicked();

private:
    void setupUI();
    void setupTitleBar();
    void startScan(const QString &steamPath);
    void populateGrid(const QList<WallpaperInfo> &wallpapers);
    void filterGrid(const QString &query);
    void applyWallpaper(const WallpaperInfo &info);
    void saveSettings();
    void loadSettings();

    DSearchEdit              *m_searchEdit   = nullptr;
    DSpinner                 *m_spinner      = nullptr;
    DLabel                   *m_statusLabel  = nullptr;
    QWidget                  *m_gridWidget   = nullptr;
    QScrollArea              *m_scrollArea   = nullptr;
    QList<WallpaperCard *>    m_cards;
    QList<WallpaperInfo>      m_allWallpapers;
    WallpaperScanner         *m_scanner      = nullptr;
    QSettings                *m_settings     = nullptr;
    QString                   m_steamPath;
    WallpaperSettings         m_wpSettings;
    EngineManager            *m_engineManager = nullptr;
};
