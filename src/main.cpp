#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <randomInstanceGenerator.hpp>
#include <instanceListReader.hpp>
#include <instance.hpp>




int main(int argc, char** argv) {
    RandomInstanceGenerator instgen(0.0f, 100.0f, 0.0f, 200.0f);
    instgen.generateToFile("outfile.txt", 1000000, 8);
    
    // We read it into the vector but it might not be read from the file but be inside memory already
    auto insts = readInstancesFromFile("outfile.txt");
    /*
    for(auto inst : insts) {

    }


    
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
