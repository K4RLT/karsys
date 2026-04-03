#pragma once
#include "baseapplier.h"

class VideoApplier : public BaseApplier
{
public:
    bool apply(const WallpaperInfo &info,
               const WallpaperSettings &settings) override;
    bool isAvailable() const override;
    QString engineName() const override { return "video"; }
};
