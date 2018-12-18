#include "mainwindow.h"
#include "quiwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QUIWidget    qui;
    MainWindow   mwnd;

    app.setWindowIcon(QIcon(":/main.ico"));
    qui.setMainWidget(&mwnd);
    qui.setTitle("BMP Viewer");
    qui.setStyle(QUIWidget::StylePSBlack);
    qui.setSizeGripEnabled(true);
    qui.setVisible(QUIWidget::BtnMenuMax, true);
    qui.setVisible(QUIWidget::BtnMenuThemes, false);
    qui.show();

    return app.exec();
}
