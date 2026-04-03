#include <DApplication>
#include <DMainWindow>
#include "mainwindow.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("K4RLT");
    app.setApplicationName("wpe-dde");
    app.setApplicationVersion("0.1.0");
    app.setApplicationDisplayName("Wallpaper Engine for Deepin");
    app.setProductIcon(QIcon::fromTheme("video-display"));
    app.loadTranslator();

    MainWindow w;
    w.show();

    return app.exec();
}
