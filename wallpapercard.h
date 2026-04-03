#pragma once

#include <DFrame>
#include <DLabel>
#include <QPixmap>
#include "wallpaperscanner.h"

DWIDGET_USE_NAMESPACE

class QMovie;
class QVBoxLayout;

class WallpaperCard : public DFrame
{
    Q_OBJECT
public:
    explicit WallpaperCard(const WallpaperInfo &info, QWidget *parent = nullptr);

    const WallpaperInfo &wallpaperInfo() const { return m_info; }

signals:
    void applyRequested(const WallpaperInfo &info);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    void loadThumbnail();
    QString typeBadgeColor() const;
    QString formatSize(qint64 bytes) const;

    WallpaperInfo  m_info;
    DLabel        *m_thumbnail;
    DLabel        *m_title;
    DLabel        *m_typeBadge;
    DLabel        *m_meta;
    QMovie        *m_movie = nullptr;
    bool           m_hovered = false;

    static constexpr int CARD_WIDTH  = 220;
    static constexpr int THUMB_HEIGHT = 130;
    // Warn user if wallpaper folder exceeds this size (500 MB)
    static constexpr qint64 LARGE_SIZE_THRESHOLD = 500LL * 1024 * 1024;
};
