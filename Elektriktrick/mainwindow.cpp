#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "glwidget.h"
#include "ETApp.h"
#include "ETModel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


#if  0
bool MainWindow::event(QEvent * e) {
    static int i = 0;
    fprintf(stderr, "MainWindow: NativeEvent %d = %d\n", i++, e->type());
    switch (e->type()) {
    case QEvent::FileOpen:
        fprintf(stderr, "MainWindow: FILE OPEN EVENT\n");
        break;
        // QEvent::Wheel
        // 197 is zoom
    default:
        break;
    }

    return QMainWindow::event(e);
}
#endif

void MainWindow::on_xyShrinkButton_clicked()
{
    ET.model()->calibrate(-0.1, 0.0, 0.0); // shrink the model by n mm's
    ui->glWidget->updateGL();
}

void MainWindow::on_xyGrowButton_clicked()
{
    ET.model()->calibrate(0.0, 0.0, -0.1); // grow the model by n mm's
    ui->glWidget->updateGL();
}

void MainWindow::on_actionSave_As_triggered()
{
    ET.model()->SaveAs();
}
