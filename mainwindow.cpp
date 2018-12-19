#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "IconsFontAwesome5.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

//#define BRIGHTNESS1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->widgetZoom->setVisible(false);
    ui->pushButtonSave->setVisible(false);
    ui->pushButtonSaveAs->setVisible(false);
    ui->pushButtonReset->setVisible(false);
    ui->frameControl->setVisible(false);

    initFont();

    modified = false;
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initFont()
{
    int     fontId      = QFontDatabase::addApplicationFont(":/image/Font-Awesome-5-Free-Solid-900.otf");
    QString fontName    = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont   fontAwesome = QFont(fontName);

    fontAwesome.setPixelSize(10);

    ui->toolButtonZoomIn->setFont(fontAwesome);
    ui->toolButtonZoomIn->setText(QChar(ICON_FA_PLUS));

    ui->toolButtonZoomOut->setFont(fontAwesome);
    ui->toolButtonZoomOut->setText(QChar(ICON_FA_MINUS));

    ui->toolButtonRotateLeft->setFont(fontAwesome);
    ui->toolButtonRotateLeft->setText(QChar(ICON_FA_ARROW_LEFT));

    ui->toolButtonRotateRight->setFont(fontAwesome);
    ui->toolButtonRotateRight->setText(QChar(ICON_FA_ARROW_RIGHT));

    ui->toolButtonMirrorVertical->setFont(fontAwesome);
    ui->toolButtonMirrorVertical->setText(QChar(ICON_FA_ARROWS_ALT_V));

    ui->toolButtonMirrorHorizontal->setFont(fontAwesome);
    ui->toolButtonMirrorHorizontal->setText(QChar(ICON_FA_ARROWS_ALT_H));
}


static void initImgFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog)
    {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    // 文件格式过滤
    QStringList mimeTypeFilters;
    mimeTypeFilters.append("image/bmp");
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
                                              ? QImageReader::supportedMimeTypes()  // 对话框为打开，导入可读的格式
                                              : QImageWriter::supportedMimeTypes(); // 对话框为保存，导入可写的格式
    foreach(const QByteArray& mimeTypeName, supportedMimeTypes) mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/bmp");

    if (acceptMode == QFileDialog::AcceptSave)
    {
        dialog.setDefaultSuffix("bmp");
    }
}


bool MainWindow::loadFile(const QString& fileName)
{
    QImageReader reader(fileName);

    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull())
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                    .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setImage(newImage);

    setWindowFilePath(fileName);

    ui->doubleSpinBoxZoom->setValue(1.00);
    ui->horizontalSliderBrightness->setValue(0);

    return true;
}


void MainWindow::setImage(const QImage& newImage)
{
    imageOld = newImage;
    imageNew = newImage;

    ui->labelImage->setPixmap(QPixmap::fromImage(imageOld));
    ui->labelImage->setAlignment(Qt::AlignCenter);

    ui->widgetZoom->setVisible(true);
    ui->pushButtonSave->setVisible(true);
    ui->pushButtonSaveAs->setVisible(true);
    ui->pushButtonReset->setVisible(true);
    ui->frameControl->setVisible(true);
}


bool MainWindow::saveFile(const QString& fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(imageNew))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                    .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }

    return true;
}


void MainWindow::scaleImage(double factor)
{
    QImage imageShow;

    if (!modified)
    {
        imageShow = imageOld.scaled(int(imageOld.width() * factor),
                                    int(imageOld.height() * factor),
                                    Qt::KeepAspectRatio);
    }
    else
    {
        imageShow = imageNew.scaled(int(imageNew.width() * factor),
                                    int(imageNew.height() * factor),
                                    Qt::KeepAspectRatio);
    }

    ui->labelImage->setPixmap(QPixmap::fromImage(imageShow));
}


void MainWindow::rotateImage(const QMatrix& m)
{
    modified = true;

    imageNew = imageNew.transformed(m);

    updateImage();
}


void MainWindow::mirrorImage(const bool h, const bool v)
{
    modified = true;

    imageNew = imageNew.mirrored(h, v);

    updateImage();
}


void MainWindow::adjustBrightness(int value)
{
    modified = true;

#ifdef BRIGHTNESS1
    uchar *ptrYOld = imageOld.bits();
    uchar *ptrYNew = imageNew.bits();

    uchar *ptrXOld;
    uchar *ptrXNew;

    for (int y = 0; y < imageOld.height(); ++y)
    {
        ptrXOld = ptrYOld;
        ptrXNew = ptrYNew;
        for (int x = 0; x < imageOld.width(); ++x)
        {
            // RGBA，A不处理
            ptrXNew[0] = (uchar)qBound(0, ptrXOld[0] + value, 255);
            ptrXNew[1] = (uchar)qBound(0, ptrXOld[1] + value, 255);
            ptrXNew[2] = (uchar)qBound(0, ptrXOld[2] + value, 255);
            // x轴指针偏移
            ptrXOld += 4;
            ptrXNew += 4;
        }
        // y轴指针偏移
        ptrYOld += imageOld.bytesPerLine();
        ptrYNew += imageNew.bytesPerLine();
    }
#elif BRIGHTNESS2
    QColor oldColor;
    int    r = 0, g = 0, b = 0;
    for (int y = 0; y < imageOld.height(); ++y)
    {
        for (int x = 0; x < imageOld.width(); ++x)
        {
            oldColor = QColor(imageOld.pixel(x, y));
            r        = oldColor.red() + value;
            g        = oldColor.green() + value;
            b        = oldColor.blue() + value;
            imageNew.setPixel(x, y, qRgb(r, g, b));
        }
    }
#else
    if ((value < -255) || (value > 255))
    {
        return;
    }

    int   red, green, blue;
    int   pixelCount = imageOld.width() * imageOld.height();
    uchar *pixelsOld = (uchar *)imageOld.bits();
    uchar *pixelsNew = (uchar *)imageNew.bits();
    for (int i = 0; i < pixelCount * 4; i += 4)
    {
        red              = pixelsOld[i] + value;
        pixelsNew[i]     = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
        green            = pixelsOld[i + 1] + value;
        pixelsNew[i + 1] = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
        blue             = pixelsOld[i + 2] + value;
        pixelsNew[i + 2] = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;
    }
#endif


    updateImage();
}


void MainWindow::adjustContrast(int value)
{
    modified = true;

    int   pixelCount = imageOld.width() * imageOld.height();
    uchar *pixelsOld = (uchar *)imageOld.bits();
    uchar *pixelsNew = (uchar *)imageNew.bits();

    uchar red, green, blue, nRed, nGreen, nBlue;

    if ((0 < value) && (value < 100))
    {
//        double param = 1 / (1 - value / 100.0) - 1;
        double param = value / 100.0;

        for (int i = 0; i < pixelCount * 4; i += 4)
        {
            nRed   = (pixelsOld[i]);
            nGreen = (pixelsOld[i + 1]);
            nBlue  = (pixelsOld[i + 2]);

            red   = nRed + (nRed - 127) * param;
            red   = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
            green = nGreen + (nGreen - 127) * param;
            green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
            blue  = nBlue + (nBlue - 127) * param;
            blue  = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;

            pixelsNew[i]     = red;
            pixelsNew[i + 1] = green;
            pixelsNew[i + 2] = blue;
        }
    }
    else
    {
        for (int i = 0; i < pixelCount * 4; i += 4)
        {
            nRed   = (pixelsOld[i]);
            nGreen = (pixelsOld[i + 1]);
            nBlue  = (pixelsOld[i + 2]);

            red   = nRed + (nRed - 127) * value / 100.0;
            red   = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
            green = nGreen + (nGreen - 127) * value / 100.0;
            green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
            blue  = nBlue + (nBlue - 127) * value / 100.0;
            blue  = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;

            pixelsNew[i]     = red;
            pixelsNew[i + 1] = green;
            pixelsNew[i + 2] = blue;
        }
    }

    updateImage();
}


void MainWindow::updateImage()
{
    scaleImage(ui->doubleSpinBoxZoom->value());
}


void MainWindow::on_pushButtonOpen_clicked()
{
    QFileDialog dlg(this, tr("打开图像文件"));

    initImgFileDialog(dlg, QFileDialog::AcceptOpen);

    while (dlg.exec() == QDialog::Accepted &&
           !loadFile(dlg.selectedFiles().first()))
    {
    }
}


void MainWindow::on_pushButtonSave_clicked()
{
    saveFile(windowFilePath());
}


void MainWindow::on_pushButtonSaveAs_clicked()
{
    QFileDialog dlg(this, tr("另存为"));

    initImgFileDialog(dlg, QFileDialog::AcceptSave);

    while (dlg.exec() == QDialog::Accepted &&
           !saveFile(dlg.selectedFiles().first()))
    {
    }
}


void MainWindow::on_pushButtonReset_clicked()
{
    modified = false;
    imageNew = imageOld;
    ui->doubleSpinBoxZoom->setValue(1.0);
    ui->horizontalSliderBrightness->setValue(0);
    ui->horizontalSliderContrast->setValue(0);
    updateImage();
}


void MainWindow::on_doubleSpinBoxZoom_valueChanged(double val)
{
    scaleImage(val);
}


void MainWindow::on_toolButtonZoomIn_clicked()
{
    QDoubleSpinBox *box = ui->doubleSpinBoxZoom;

    box->setValue(box->value() + 0.1);
}


void MainWindow::on_toolButtonZoomOut_clicked()
{
    QDoubleSpinBox *box = ui->doubleSpinBoxZoom;

    box->setValue(box->value() - 0.1);
}


void MainWindow::on_toolButtonRotateLeft_clicked()
{
    QMatrix m;

    m.rotate(-90);
    rotateImage(m);
}


void MainWindow::on_toolButtonRotateRight_clicked()
{
    QMatrix m;

    m.rotate(90);
    rotateImage(m);
}


void MainWindow::on_toolButtonMirrorHorizontal_clicked()
{
    const bool horizontal = true;
    const bool vertical   = false;

    mirrorImage(horizontal, vertical);
}


void MainWindow::on_toolButtonMirrorVertical_clicked()
{
    const bool horizontal = false;
    const bool vertical   = true;

    mirrorImage(horizontal, vertical);
}


void MainWindow::on_horizontalSliderBrightness_valueChanged(int value)
{
    ui->horizontalSliderContrast->setValue(0);
    adjustBrightness(value);
}


void MainWindow::on_horizontalSliderContrast_valueChanged(int value)
{
    ui->horizontalSliderBrightness->setValue(0);
    adjustContrast(value);
}
