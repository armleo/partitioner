#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <instance.hpp>
#include <instanceGrid.hpp>
#include "partitioner.hpp"


int main(int argc, char** argv) {
    InstanceGrid instgrid(1.0);

    instgrid.generateRandomInstancesToFile("outfile.txt", 1000000, 0.0f, 100.0f, 0.0f, 200.0f, 8);
    instgrid.readInstancesFromFile("outfile.txt");
    
    auto width = 800;
    auto height = 600;
    QApplication app(argc, argv);
    DotWidget* widget = new DotWidget(instgrid.getCellInstances(0, 0));
    widget->resize(width, height);
    widget->setWindowTitle("Instance Grid Dots");
    widget->show();
    return app.exec();
}
