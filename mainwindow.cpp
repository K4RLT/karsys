#include "mainwindow.h"
#include "wallpapercard.h"
#include "flowlayout.h"
#include "settingsdialog.h"

#include <DTitlebar>
#include <DPushButton>
#include <DFileDialog>
#include <DMessageBox>
#include <DFrame>

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QDir>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_scanner(new WallpaperScanner(this))
    , m_settings(new QSettings("K4RLT", "WPE-DDE", this))
    , m_engineManager(new EngineManager(this))
{
    setMinimumSize(900, 600);
    resize(1100, 700);
    setWindowTitle(tr("Wallpaper Engine for Deepin"));

    setupUI();
    setupTitleBar();
    loadSettings();

    connect(m_scanner, &WallpaperScanner::scanProgress,
            this, &MainWindow::onScanProgress);

    if (m_steamPath.isEmpty()) {
        QStringList paths = WallpaperScanner::detectSteamPaths();
        if (!paths.isEmpty()) {
            m_steamPath = paths.first();
            startScan(m_steamPath);
        } else {
            m_statusLabel->setText(tr("Steam library not found. Click the folder icon to select it."));
        }
    } else {
        startScan(m_steamPath);
    }
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 8, 16, 16);
    mainLayout->setSpacing(8);

    auto *topRow = new QHBoxLayout();
    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search wallpapers..."));
    m_searchEdit->setFixedHeight(36);
    connect(m_searchEdit, &DSearchEdit::textChanged,
            this, &MainWindow::onSearchChanged);

    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(24, 24);
    m_spinner->hide();

    m_statusLabel = new DLabel(this);
    m_statusLabel->setForegroundRole(QPalette::PlaceholderText);

    topRow->addWidget(m_searchEdit);
    topRow->addWidget(m_spinner);
    topRow->addWidget(m_statusLabel);
    mainLayout->addLayout(topRow);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_gridWidget = new QWidget(m_scrollArea);
    auto *flowLayout = new FlowLayout(m_gridWidget, 0, 12, 12);
    m_gridWidget->setLayout(flowLayout);

    m_scrollArea->setWidget(m_gridWidget);
    mainLayout->addWidget(m_scrollArea);
}

void MainWindow::setupTitleBar()
{
    DTitlebar *tb = titlebar();
    tb->setTitle(tr("Wallpaper Engine for Deepin"));
    tb->setIcon(QIcon::fromTheme("video-display"));

    // Settings button
    auto *settingsBtn = new DPushButton(this);
    settingsBtn->setIcon(QIcon::fromTheme("preferences-system"));
    settingsBtn->setToolTip(tr("Settings"));
    settingsBtn->setFlat(true);
    tb->addWidget(settingsBtn, Qt::AlignRight);
    connect(settingsBtn, &DPushButton::clicked,
            this, &MainWindow::onSettingsClicked);

    // Folder picker button
    auto *folderBtn = new DPushButton(this);
    folderBtn->setIcon(QIcon::fromTheme("folder-open"));
    folderBtn->setToolTip(tr("Select Steam library folder"));
    folderBtn->setFlat(true);
    tb->addWidget(folderBtn, Qt::AlignRight);
    connect(folderBtn, &DPushButton::clicked,
            this, &MainWindow::onSelectFolderClicked);
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dlg(m_wpSettings, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_wpSettings = dlg.settings();
        saveSettings();
        // TODO Phase 2: push updated settings to plugin via DBus
        qDebug() << "Settings updated — scale:" << m_wpSettings.scaleMode
                 << "fps:" << m_wpSettings.fpsCap
                 << "audio:" << m_wpSettings.audioEnabled;
    }
}

void MainWindow::startScan(const QString &steamPath)
{
    m_spinner->show();
    m_spinner->start();
    m_statusLabel->setText(tr("Scanning wallpapers..."));
    m_searchEdit->setEnabled(false);

    auto *watcher = new QFutureWatcher<QList<WallpaperInfo>>(this);
    connect(watcher, &QFutureWatcher<QList<WallpaperInfo>>::finished, this, [=]() {
        onScanFinished(watcher->result());
        watcher->deleteLater();
    });

    watcher->setFuture(QtConcurrent::run([=]() {
        return m_scanner->scan(steamPath);
    }));
}

void MainWindow::onScanProgress(int current, int total)
{
    m_statusLabel->setText(tr("Scanning... %1 / %2").arg(current).arg(total));
}

void MainWindow::onScanFinished(const QList<WallpaperInfo> &wallpapers)
{
    m_spinner->stop();
    m_spinner->hide();
    m_searchEdit->setEnabled(true);
    m_allWallpapers = wallpapers;

    if (wallpapers.isEmpty()) {
        m_statusLabel->setText(tr("No wallpapers found. Make sure Wallpaper Engine is installed on Steam."));
        return;
    }

    m_statusLabel->setText(tr("%1 wallpapers").arg(wallpapers.size()));
    populateGrid(wallpapers);
}

void MainWindow::populateGrid(const QList<WallpaperInfo> &wallpapers)
{
    for (WallpaperCard *card : m_cards)
        card->deleteLater();
    m_cards.clear();

    auto *layout = static_cast<FlowLayout *>(m_gridWidget->layout());

    for (const WallpaperInfo &info : wallpapers) {
        auto *card = new WallpaperCard(info, m_gridWidget);
        connect(card, &WallpaperCard::applyRequested,
                this, &MainWindow::onApplyRequested);
        layout->addWidget(card);
        m_cards << card;
    }
}

void MainWindow::onSearchChanged(const QString &text)
{
    filterGrid(text);
}

void MainWindow::filterGrid(const QString &query)
{
    QString q = query.trimmed().toLower();
    for (WallpaperCard *card : m_cards) {
        bool match = q.isEmpty()
                     || card->wallpaperInfo().title.toLower().contains(q)
                     || card->wallpaperInfo().type.toLower().contains(q)
                     || card->wallpaperInfo().author.toLower().contains(q);
        card->setVisible(match);
    }
}

void MainWindow::onApplyRequested(const WallpaperInfo &info)
{
    applyWallpaper(info);
}

void MainWindow::applyWallpaper(const WallpaperInfo &info)
{
    qDebug() << "Apply wallpaper:" << info.folderPath << "type:" << info.type;
    m_engineManager->apply(info, m_wpSettings);
    m_statusLabel->setText(tr("Applied: %1").arg(info.title));
}

void MainWindow::onSelectFolderClicked()
{
    QString path = DFileDialog::getExistingDirectory(
        this,
        tr("Select Steam Library Folder"),
        QDir::homePath()
    );

    if (path.isEmpty())
        return;

    if (!QDir(WallpaperScanner::workshopPath(path)).exists()) {
        DMessageBox::warning(this, tr("Invalid Folder"),
            tr("No Wallpaper Engine workshop content found in this folder.\n"
               "Please select your Steam library root (e.g. ~/.local/share/Steam)"));
        return;
    }

    m_steamPath = path;
    saveSettings();
    startScan(path);
}

void MainWindow::saveSettings()
{
    m_settings->setValue("steamPath", m_steamPath);
    m_wpSettings.save(m_settings);
}

void MainWindow::loadSettings()
{
    m_steamPath = m_settings->value("steamPath").toString();
    m_wpSettings.load(m_settings);
}
