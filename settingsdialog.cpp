#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

DWIDGET_USE_NAMESPACE

// ── WallpaperSettings persistence ────────────────────────────────────────────

void WallpaperSettings::save(QSettings *s) const
{
    s->beginGroup("Playback");
    s->setValue("scaleMode",    scaleMode);
    s->setValue("fpsCap",       fpsCap);
    s->setValue("audioEnabled", audioEnabled);
    s->endGroup();

    s->beginGroup("Pause");
    s->setValue("pauseOnFullscreen", pauseOnFullscreen);
    s->setValue("pauseOnIdle",       pauseOnIdle);
    s->setValue("idleTimeoutSec",    idleTimeoutSec);
    s->endGroup();
}

void WallpaperSettings::load(QSettings *s)
{
    s->beginGroup("Playback");
    scaleMode    = s->value("scaleMode",    scaleMode).toInt();
    fpsCap       = s->value("fpsCap",       fpsCap).toInt();
    audioEnabled = s->value("audioEnabled", audioEnabled).toBool();
    s->endGroup();

    s->beginGroup("Pause");
    pauseOnFullscreen = s->value("pauseOnFullscreen", pauseOnFullscreen).toBool();
    pauseOnIdle       = s->value("pauseOnIdle",       pauseOnIdle).toBool();
    idleTimeoutSec    = s->value("idleTimeoutSec",    idleTimeoutSec).toInt();
    s->endGroup();
}

// ── SettingsDialog ────────────────────────────────────────────────────────────

SettingsDialog::SettingsDialog(const WallpaperSettings &current, QWidget *parent)
    : DDialog(parent)
    , m_settings(current)
{
    setWindowTitle(tr("Settings"));
    setFixedWidth(420);
    setupUI();
}

void SettingsDialog::setupUI()
{
    // ── Playback section ──────────────────────────────────────────────────────
    auto *playbackGrid = new QGridLayout();
    playbackGrid->setColumnStretch(1, 1);
    playbackGrid->setVerticalSpacing(10);
    playbackGrid->setHorizontalSpacing(16);

    // Scale mode
    playbackGrid->addWidget(new DLabel(tr("Scale mode"), this), 0, 0);
    m_scaleCombo = new DComboBox(this);
    m_scaleCombo->addItems({ tr("Fill"), tr("Fit"), tr("Stretch"), tr("Center") });
    m_scaleCombo->setCurrentIndex(m_settings.scaleMode);
    playbackGrid->addWidget(m_scaleCombo, 0, 1);

    // FPS cap
    playbackGrid->addWidget(new DLabel(tr("FPS cap"), this), 1, 0);
    m_fpsCombo = new DComboBox(this);
    m_fpsCombo->addItem("15 fps",        15);
    m_fpsCombo->addItem("24 fps",        24);
    m_fpsCombo->addItem("30 fps",        30);
    m_fpsCombo->addItem("60 fps",        60);
    m_fpsCombo->addItem(tr("Unlimited"), 0);
    // Select current value
    for (int i = 0; i < m_fpsCombo->count(); ++i) {
        if (m_fpsCombo->itemData(i).toInt() == m_settings.fpsCap) {
            m_fpsCombo->setCurrentIndex(i);
            break;
        }
    }
    playbackGrid->addWidget(m_fpsCombo, 1, 1);

    // Audio
    playbackGrid->addWidget(new DLabel(tr("Audio"), this), 2, 0);
    m_audioSwitch = new DSwitchButton(this);
    m_audioSwitch->setChecked(m_settings.audioEnabled);
    playbackGrid->addWidget(m_audioSwitch, 2, 1, Qt::AlignLeft);

    // ── Pause section ─────────────────────────────────────────────────────────
    auto *pauseGrid = new QGridLayout();
    pauseGrid->setColumnStretch(1, 1);
    pauseGrid->setVerticalSpacing(10);
    pauseGrid->setHorizontalSpacing(16);

    // Pause on fullscreen
    pauseGrid->addWidget(new DLabel(tr("Pause on fullscreen app"), this), 0, 0);
    m_fsSwitch = new DSwitchButton(this);
    m_fsSwitch->setChecked(m_settings.pauseOnFullscreen);
    pauseGrid->addWidget(m_fsSwitch, 0, 1, Qt::AlignLeft);

    // Pause on idle
    pauseGrid->addWidget(new DLabel(tr("Pause on idle"), this), 1, 0);
    m_idleSwitch = new DSwitchButton(this);
    m_idleSwitch->setChecked(m_settings.pauseOnIdle);
    pauseGrid->addWidget(m_idleSwitch, 1, 1, Qt::AlignLeft);

    // Idle timeout (only enabled when idle pause is on)
    m_idleLabel = new DLabel(tr("Idle timeout (seconds)"), this);
    m_idleLabel->setEnabled(m_settings.pauseOnIdle);
    pauseGrid->addWidget(m_idleLabel, 2, 0);

    m_idleSpin = new DSpinBox(this);
    m_idleSpin->setRange(30, 3600);
    m_idleSpin->setSingleStep(30);
    m_idleSpin->setValue(m_settings.idleTimeoutSec);
    m_idleSpin->setEnabled(m_settings.pauseOnIdle);
    pauseGrid->addWidget(m_idleSpin, 2, 1);

    // Toggle idle spin enabled state when switch changes
    connect(m_idleSwitch, &DSwitchButton::checkedChanged, this, [=](bool on) {
        m_idleSpin->setEnabled(on);
        m_idleLabel->setEnabled(on);
    });

    // ── Assemble ──────────────────────────────────────────────────────────────
    auto *root = new QVBoxLayout();
    root->setContentsMargins(16, 8, 16, 8);
    root->setSpacing(16);
    root->addWidget(makeSection(tr("Playback"), playbackGrid));
    root->addWidget(makeSection(tr("Pause"),    pauseGrid));
    root->addStretch();

    auto *container = new QWidget(this);
    container->setLayout(root);
    addContent(container);

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("Apply"),  true,  DDialog::ButtonRecommend);

    connect(this, &DDialog::buttonClicked, this, [=](int index, const QString &) {
        if (index == 1)
            onAccepted();
    });
}

QWidget *SettingsDialog::makeSection(const QString &title, QLayout *content)
{
    auto *frame = new QGroupBox(title, this);
    frame->setFlat(true);
    frame->setLayout(content);
    return frame;
}

void SettingsDialog::onAccepted()
{
    m_settings.scaleMode       = m_scaleCombo->currentIndex();
    m_settings.fpsCap          = m_fpsCombo->currentData().toInt();
    m_settings.audioEnabled    = m_audioSwitch->isChecked();
    m_settings.pauseOnFullscreen = m_fsSwitch->isChecked();
    m_settings.pauseOnIdle       = m_idleSwitch->isChecked();
    m_settings.idleTimeoutSec    = m_idleSpin->value();
}
