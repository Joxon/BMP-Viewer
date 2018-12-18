#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool loadFile(const QString&);


private slots:
    void on_pushButtonOpen_clicked();

    void on_pushButtonSaveAs_clicked();

    void on_pushButtonReset_clicked();

    void on_doubleSpinBoxZoom_valueChanged(double val);

    void on_toolButtonZoomIn_clicked();

    void on_toolButtonZoomOut_clicked();

    void on_toolButtonRotateLeft_clicked();

    void on_toolButtonRotateRight_clicked();

    void on_toolButtonMirrorHorizontal_clicked();

    void on_toolButtonMirrorVertical_clicked();

    void on_horizontalSliderBrightness_sliderMoved(int position);

    void on_horizontalSliderBrightness_actionTriggered(int action);

private:
    Ui::MainWindow *ui;

    QImage imageOld;
    QImage imageNew;
    bool modified;

    void initFont();
    bool saveFile(const QString& fileName);
    void setImage(const QImage& newImage);
    void scaleImage(double factor);
    void rotateImage(const QMatrix& m);
    void mirrorImage(const bool h, const bool v);
    void adjustBrightness(int value);
    void adjustContrast(int value);

    void updateImage();
};

#endif // MAINWINDOW_H
