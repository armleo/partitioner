#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <instance.hpp>
#include <instanceGrid.hpp>
#include "partitioner.hpp"

struct AlgoInfo {
    std::string name;
    void (Partitioner::*method)();
};

int main(int argc, char** argv) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::duration_cast;

    QApplication app(argc, argv);

    // Prepare grids
    InstanceGrid coarseGrid(10.0);
    InstanceGrid fineGrid(1.0);

    // Generate and load instances
    coarseGrid.generateGaussianClustersToFile("outfile.txt", 100000, 10, BoundingBox(Point2D(0.0f, 0.0f), Point2D(100.0f, 200.0f)), 10.0f, 8);
    coarseGrid.readInstancesFromFile("outfile.txt");
    fineGrid.readInstancesFromFile("outfile.txt");

    std::vector<std::pair<std::string, InstanceGrid*>> grids = {
        {"COARSE", &coarseGrid},
        {"FINE", &fineGrid}
    };

    std::vector<AlgoInfo> algos = {
        {"RANDOM",    &Partitioner::partition},
        {"LOCALIZE",  &Partitioner::partitionLocalized},
        {"MERGE",     &Partitioner::partitionMerging},
        {"NEAREST",   &Partitioner::partitionNearby}
    };

    int width = 800, height = 600;
    std::vector<DotWidget*> widgets;
    std::vector<Partitioner*> partitioners;

    for (const auto& gridPair : grids) {
        const std::string& gridType = gridPair.first;
        InstanceGrid& grid = *gridPair.second;

        std::cout << "Grid " << gridType << " instance count: " << grid.getInstanceCount() << std::endl;

        for (const auto& algo : algos) {
            auto partitioner = new Partitioner(grid, 1000);
            partitioners.push_back(partitioner);

            auto t1 = high_resolution_clock::now();
            (partitioner->*algo.method)();
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;

            auto partitions = partitioner->getPartitions();
            std::cout << "Algo " << algo.name
                      << " GRID TYPE: " << gridType
                      << " MISSED (DNF if non zero): " << partitioner->countGridInstancesMissedInPartitions()
                      << " UNBALANCED (DNF if non zero): " << partitioner->getViolatingBitLimitPartitionCount()
                      << " PARTITIONS: " << partitions.size()
                      << " AVERAGE: " << partitioner->getPartitionAverageBitSize()
                      << " ROUTE_LEN: " << partitioner->getPartitionsTotalRoutingLength()
                      << "   RUNTIME: " << ms_double.count() << "ms" << std::endl;

            // Show widget
            auto* widget = new DotWidget(grid, partitions);
            width = int(ceil(std::max(float(width), grid.getBounds().ur.x)));
            height = int(ceil(std::max(float(height), grid.getBounds().ur.y)));
            widget->resize(width, height);
            widget->setWindowTitle(QString("Dots - %1 - %2").arg(QString::fromStdString(algo.name), QString::fromStdString(gridType)));
            widget->show();
            widgets.push_back(widget);
        }
    }

    // TODO: Properly delete partitioners and widgets if needed
    return app.exec();
}