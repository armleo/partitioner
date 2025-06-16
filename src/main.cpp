#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <randomInstanceGenerator.hpp>
#include <instance.hpp>


int main(int argc, char** argv) {
    /*
    std::ofstream outfile;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> rnd(0.0, 10000.0);

    outfile.open ("outfile.txt");
    for (size_t i = 0; i < 10000; i ++) {
        outfile << "instance" << i << " " << round(rnd(gen)) * 0.01 << " " << round(rnd(gen)) * 0.01 << std::endl;
    }
    
    
    QApplication app(argc, argv);
    QWidget window;
    window.resize(320, 240);
    window.show();
    window.setWindowTitle(
        QApplication::translate("toplevel", "Top-level widget"));
    return app.exec();
    */

    RandomInstanceGenerator instgen(0.0f, 100.0f, 0.0f, 200.0f);
    instgen.generateToFile("outfile.txt", 1000000, 8);

    Instance a("a", 10, 10.1), b("b", 11, 12);

    std::cout << a.distanceTo(b) << std::endl;
    return 0;
}
