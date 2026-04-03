#include "wallpapercard.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE

WallpaperCard::WallpaperCard(const WallpaperInfo &info, QWidget *parent)
    : DFrame(parent)
    , m_info(info)
{
    setFixedWidth(CARD_WIDTH);
    setCursor(Qt::PointingHandCursor);
    setupUI();
    loadThumbnail();
}

void WallpaperCard::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 8);
    layout->setSpacing(4);

    // --- Thumbnail ---
    m_thumbnail = new DLabel(this);
    m_thumbnail->setFixedSize(CARD_WIDTH, THUMB_HEIGHT);
    m_thumbnail->setAlignment(Qt::AlignCenter);
    m_thumbnail->setScaledContents(false);

    // Type badge overlaid on thumbnail
    m_typeBadge = new DLabel(m_thumbnail);
    m_typeBadge->setText(m_info.type);
    m_typeBadge->setAlignment(Qt::AlignCenter);
    m_typeBadge->setFixedSize(56, 20);
    m_typeBadge->move(8, 8);
    m_typeBadge->setStyleSheet(QString(
        "background-color: %1;"
        "color: white;"
        "border-radius: 4px;"
        "font-size: 10px;"
        "font-weight: bold;"
        "padding: 1px 4px;"
    ).arg(typeBadgeColor()));

    layout->addWidget(m_thumbnail);

    // --- Title ---
    m_title = new DLabel(this);
    m_title->setWordWrap(false);
    m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_title->setMaximumWidth(CARD_WIDTH - 16);
    QFont titleFont = m_title->font();
    titleFont.setPointSize(9);
    titleFont.setBold(true);
    m_title->setFont(titleFont);

    QFontMetrics fm(titleFont);
    m_title->setText(fm.elidedText(m_info.title, Qt::ElideRight, CARD_WIDTH - 16));
    layout->addWidget(m_title);
    layout->setContentsMargins(8, 0, 8, 8);

    // --- Meta (resolution + size warning) ---
    QStringList metaParts;
    if (!m_info.resolution.isEmpty())
        metaParts << m_info.resolution;

    if (m_info.folderSize >= LARGE_SIZE_THRESHOLD)
        metaParts << QString("⚠ %1").arg(formatSize(m_info.folderSize));

    m_meta = new DLabel(this);
    m_meta->setText(metaParts.join("  ·  "));
    m_meta->setAlignment(Qt::AlignLeft);
    QFont metaFont = m_meta->font();
    metaFont.setPointSize(8);
    m_meta->setFont(metaFont);
    m_meta->setForegroundRole(QPalette::PlaceholderText);
    layout->addWidget(m_meta);
}

void WallpaperCard::loadThumbnail()
{
    if (m_info.previewPath.isEmpty()) {
        m_thumbnail->setText("No Preview");
        return;
    }

    if (m_info.previewPath.endsWith(".gif", Qt::CaseInsensitive)) {
        m_movie = new QMovie(m_info.previewPath, QByteArray(), this);
        m_movie->setScaledSize(QSize(CARD_WIDTH, THUMB_HEIGHT));
        m_thumbnail->setMovie(m_movie);
        m_movie->start();
        return;
    }

    QPixmap px(m_info.previewPath);
    if (px.isNull()) {
        m_thumbnail->setText("No Preview");
        return;
    }

    px = px.scaled(CARD_WIDTH, THUMB_HEIGHT,
                   Qt::KeepAspectRatioByExpanding,
                   Qt::SmoothTransformation);

    if (px.width() > CARD_WIDTH || px.height() > THUMB_HEIGHT) {
        int x = (px.width()  - CARD_WIDTH)  / 2;
        int y = (px.height() - THUMB_HEIGHT) / 2;
        px = px.copy(x, y, CARD_WIDTH, THUMB_HEIGHT);
    }

    m_thumbnail->setPixmap(px);
}

void WallpaperCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        DFrame::mousePressEvent(event);
        return;
    }

    if (m_info.folderSize >= LARGE_SIZE_THRESHOLD) {
        auto ret = QMessageBox::question(
            this,
            tr("Large Wallpaper"),
            tr("This wallpaper is %1. It may use more memory and GPU resources. Apply anyway?")
                .arg(formatSize(m_info.folderSize)),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (ret != QMessageBox::Yes)
            return;
    }

    emit applyRequested(m_info);
}

void WallpaperCard::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    DFrame::enterEvent(event);
}

void WallpaperCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    DFrame::leaveEvent(event);
}

void WallpaperCard::paintEvent(QPaintEvent *event)
{
    DFrame::paintEvent(event);

    if (m_hovered) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect(), 8, 8);
        p.setPen(QPen(QColor(0, 120, 255, 180), 2));
        p.drawPath(path);
    }
}

QString WallpaperCard::typeBadgeColor() const
{
    if (m_info.type == "Video")  return "#2196F3";
    if (m_info.type == "Scene")  return "#9C27B0";
    if (m_info.type == "Web")    return "#4CAF50";
    return "#607D8B";
}

QString WallpaperCard::formatSize(qint64 bytes) const
{
    if (bytes >= 1024LL * 1024 * 1024)
        return QString("%1 GB").arg(bytes / (1024.0 * 1024 * 1024), 0, 'f', 1);
    if (bytes >= 1024 * 1024)
        return QString("%1 MB").arg(bytes / (1024.0 * 1024), 0, 'f', 0);
    return QString("%1 KB").arg(bytes / 1024);
}
