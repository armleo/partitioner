#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <instance.hpp>
#include <instanceGrid.hpp>



int main(int argc, char** argv) {
    InstanceGrid instgrid(1.0);

    instgrid.generateRandomInstancesToFile("outfile.txt", 1000000, 0.0f, 100.0f, 0.0f, 200.0f, 8);

    // We read it into the vector but it might not be read from the file but be inside memory already
    instgrid.readInstancesFromFile("outfile.txt");
    
    /*

    
    QApplication app(argc, argv);
    QWidget window;
    window.resize(320, 240);
    window.show();
    window.setWindowTitle(
        QApplication::translate("toplevel", "Top-level widget"));
    
    return app.exec();
    */
    return 0;
}
