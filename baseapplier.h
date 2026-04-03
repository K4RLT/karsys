#pragma once
#include "wallpaperscanner.h"
#include "settingsdialog.h"

class BaseApplier
{
public:
    virtual ~BaseApplier() = default;
    virtual bool apply(const WallpaperInfo &info,
                       const WallpaperSettings &settings) = 0;
    virtual bool isAvailable() const = 0;
    virtual QString engineName() const = 0;
};
