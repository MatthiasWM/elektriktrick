#include "mainwindow.h"
#include <QApplication>

#include "ETApp.h"
#include "ETModel.h"
#include "ETImportSTL.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();    

    //ET.loadModel("/Users/matt/dev/Electrictrick/FilamentWipe.stl");
    //ET.loadModel("/Users/matt/Metropolis_Robot_Maria/head.stl");
    //ET.loadModel("/Users/matt/Desktop/InMoov/Neck_Mechanism_For_InMoov/SkullServoFixV1.stl");
    ET.loadModel("/Users/matt/Desktop/ETCalibrate.stl");

    return a.exec();
}
