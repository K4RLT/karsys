cmake_minimum_required(VERSION 3.16)
project(wpe-dde VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Concurrent
    DBus
)

find_package(Dtk6Widget REQUIRED)
find_package(Dtk6Core REQUIRED)
find_package(Dtk6Gui REQUIRED)

# ── GUI app ───────────────────────────────────────────────────────────────────

set(SOURCES
    main.cpp
    mainwindow.cpp
    wallpapercard.cpp
    wallpaperscanner.cpp
    settingsdialog.cpp
    flowlayout.cpp
    enginemanager.cpp
    videoapplier.cpp
    sceneapplier.cpp
)

set(HEADERS
    mainwindow.h
    wallpapercard.h
    wallpaperscanner.h
    settingsdialog.h
    flowlayout.h
    baseapplier.h
    enginemanager.h
    videoapplier.h
    sceneapplier.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE .)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::DBus
    Dtk6::Widget
    Dtk6::Core
    Dtk6::Gui
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION /usr/bin
)

# ── DDE desktop plugin .so ────────────────────────────────────────────────────
add_subdirectory(plugin)
