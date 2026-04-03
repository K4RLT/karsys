#pragma once

#include <DDialog>
#include <DLabel>
#include <DSwitchButton>
#include <DComboBox>
#include <DSpinBox>
#include <DFrame>
#include <DPushButton>

#include <QSettings>

DWIDGET_USE_NAMESPACE

// Shared settings pod — passed between MainWindow, SettingsDialog, and later DBus layer
struct WallpaperSettings {
    // Playback
    int     scaleMode       = 0;     // 0=Fill, 1=Fit, 2=Stretch, 3=Center
    int     fpsCap          = 60;    // 15 | 24 | 30 | 60 | 0=unlimited
    bool    audioEnabled    = false;

    // Pause conditions
    bool    pauseOnFullscreen = true;
    bool    pauseOnIdle       = true;
    int     idleTimeoutSec    = 300; // seconds

    void save(QSettings *s) const;
    void load(QSettings *s);
};

// ──────────────────────────────────────────────

class SettingsDialog : public DDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(const WallpaperSettings &current, QWidget *parent = nullptr);

    WallpaperSettings settings() const { return m_settings; }

private slots:
    void onAccepted();

private:
    void setupUI();
    QWidget *makeSection(const QString &title, QLayout *content);

    WallpaperSettings m_settings;

    DComboBox    *m_scaleCombo    = nullptr;
    DComboBox    *m_fpsCombo      = nullptr;
    DSwitchButton *m_audioSwitch  = nullptr;
    DSwitchButton *m_fsSwitch     = nullptr;
    DSwitchButton *m_idleSwitch   = nullptr;
    DSpinBox     *m_idleSpin      = nullptr;
    DLabel       *m_idleLabel     = nullptr;
};
