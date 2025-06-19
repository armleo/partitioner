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

struct RunConfig {
    std::string gridType;
    InstanceGrid* grid;
    AlgoInfo algo;
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
    coarseGrid.generateGaussianClustersToFile("outfile.txt", 1000000, 10, BoundingBox(Point2D(0.0f, 0.0f), Point2D(100.0f, 200.0f)), 10.0f, 8);
    coarseGrid.readInstancesFromFile("outfile.txt");
    fineGrid.readInstancesFromFile("outfile.txt");

    // Only run these combinations:
    std::vector<RunConfig> runs = {
        {"COARSE", &coarseGrid, {"HASHMAP",   &Partitioner::partition}},
        {"FINE",   &fineGrid,   {"LOCALIZE", &Partitioner::partitionLocalized}},
        {"COARSE", &coarseGrid, {"MERGE",    &Partitioner::partitionMerging}},
        {"COARSE", &coarseGrid, {"NEARBY",    &Partitioner::partitionNearby}}
    };

    int width = 800, height = 600;
    std::vector<DotWidget*> widgets;
    std::vector<Partitioner*> partitioners;

    for (const auto& run : runs) {
        std::cout << "Grid " << run.gridType << " instance count: " << run.grid->getInstanceCount() << std::endl;

        auto partitioner = new Partitioner(*run.grid, 1000);
        partitioners.push_back(partitioner);

        auto t1 = high_resolution_clock::now();
        (partitioner->*run.algo.method)();
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;

        auto partitions = partitioner->getPartitions();
        std::cout << "Algo " << run.algo.name
                  << " GRIDTYPE: " << run.gridType
                  << " MISSED (DNF if non zero): " << partitioner->countGridInstancesMissedInPartitions()
                  << " UNBALANCED (DNF if non zero): " << partitioner->getViolatingBitLimitPartitionCount()
                  << " PARTITIONS: " << partitions.size()
                  << " AVERAGE: " << partitioner->getPartitionAverageBitSize()
                  << " ROUTE_LEN: " << partitioner->getPartitionsTotalRoutingLength()
                  << "   RUNTIME: " << ms_double.count() << "ms" << std::endl;

        auto* widget = new DotWidget(*run.grid, partitions);
        width = int(ceil(std::max(float(width), run.grid->getBounds().ur.x)));
        height = int(ceil(std::max(float(height), run.grid->getBounds().ur.y)));
        widget->resize(width, height);
        widget->setWindowTitle(QString("Dots - %1 - %2").arg(QString::fromStdString(run.algo.name), QString::fromStdString(run.gridType)));
        widget->show();
        widgets.push_back(widget);
    }

    // TODO: Properly delete partitioners and widgets if needed
    return app.exec();
}