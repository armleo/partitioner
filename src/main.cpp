#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <instance.hpp>
#include <instanceGrid.hpp>
#include "partitioner.hpp"


int main(int argc, char** argv) {
    InstanceGrid instgrid(1.0);

    instgrid.generateRandomInstancesToFile("outfile.txt", 10000, BoundingBox(Point2D(0.0f, 0.0f), Point2D(100.0f, 200.0f)), 8);
    instgrid.readInstancesFromFile("outfile.txt");
    

    Partitioner partitioner(instgrid, 500);
    //partitioner.partitionNearby();
    partitioner.partition();
    auto partitions = partitioner.getPartitions();
    std::cout << "TOTAL DEFAULT: " << partitioner.getPartitionsTotalRoutingLength() << std::endl;

    partitioner.partitionLocalized();
    partitions = partitioner.getPartitions();
    std::cout << "TOTAL LOCALIZED: " << partitioner.getPartitionsTotalRoutingLength() << std::endl;


    partitioner.partitionNearby();
    partitions = partitioner.getPartitions();
    std::cout << "TOTAL NEARBY: " << partitioner.getPartitionsTotalRoutingLength() << std::endl;
    /*
    for(auto partition : partitions) {
        std::cout << partition.totalBitsize << std::endl;
    }
    */
    
    
    auto width = 800;
    auto height = 600;
    QApplication app(argc, argv);
    DotWidget* widget = new DotWidget(instgrid, partitions);
    widget->resize(width, height);
    widget->setWindowTitle("Instance Grid Dots");
    widget->show();
    return app.exec();
    
}
