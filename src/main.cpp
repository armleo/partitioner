#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <instance.hpp>
#include <instanceGrid.hpp>
#include "partitioner.hpp"


int main(int argc, char** argv) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;



    InstanceGrid instgrid(10.0);

    //instgrid.generateRandomInstancesToFile("outfile.txt", 1000000, BoundingBox(Point2D(0.0f, 0.0f), Point2D(100.0f, 200.0f)), 8);
    instgrid.generateGaussianClustersToFile("outfile.txt", 2000, 10, BoundingBox(Point2D(0.0f, 0.0f), Point2D(100.0f, 200.0f)),
                                                 10.0f, 8);
    instgrid.readInstancesFromFile("outfile.txt");
    
    auto width = 800;
    auto height = 600;
    QApplication app(argc, argv);

    std::cout << instgrid.getInstanceCount() << std::endl;
    
    std::vector<Partitioner*> partitionersList;

    for(int i = 0; i < 4; i++) {
        
        auto partitioner = new Partitioner(instgrid, 1000);
        partitionersList.push_back(partitioner);

        //partitioner.partitionNearby();

        auto t1 = high_resolution_clock::now();
        if (i == 0) partitioner->partition();
        if (i == 1) partitioner->partitionMerging();
        if (i == 2) partitioner->partitionLocalized();
        if (i == 3) partitioner->partitionNearby();
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;

        auto partitions = partitioner->getPartitions();
        std::cout << "Algo " << i << " MISSED: " << partitioner->countGridInstancesMissedInPartitions() << " PARTITIONS: " << partitions.size() << " AVERAGE: " << partitioner->getPartitionAverageBitSize() << " UNBALANCED: " << partitioner->getViolatingBitLimitPartitionCount() << " ROUTE_LEN: " << partitioner->getPartitionsTotalRoutingLength() << "   RUNTIME: " << ms_double.count() << "ms" << std::endl;
        
    }

    
    int i = 0;
    for(auto partitioner : partitionersList) {
        auto partitions = partitioner->getPartitions();
        DotWidget* widget = new DotWidget(instgrid, partitions);
        widget->resize(width, height);
        QString header = QString("Dots %1").arg(i);
        widget->setWindowTitle(header);
        widget->show();
        ++i;
    }
    // TODO: Delete partionersList instances and DotWidget
    return app.exec();
    
    
}
