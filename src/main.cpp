#include <iostream>
#include <random>
#include <fstream>
#include <QtWidgets>
#include <instance.hpp>
#include <instanceGrid.hpp>
#include "partitioner.hpp"
#include "viewer.hpp"

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
    InstanceGrid middleGrid(2.0);
    InstanceGrid fineGrid(1.0);

            std::cout << "| Algorithm | Grid    | Instances | Runtime (ms) | Route Len |\n";
            std::cout << "|-----------|---------|-----------|--------------|-----------|\n";

    for (size_t instCount = 10000; instCount < 50000; instCount *= 2) {
        instCount = 100000;
        // Generate and load instances
        std::string filename = "outfile" + std::to_string(instCount) + ".txt";
        coarseGrid.generateGaussianClustersToFile(filename, instCount, 10, BoundingBox(Point2D(0.0f, 0.0f), Point2D(100.0f, 200.0f)), 10.0f, 8);
        coarseGrid.readInstancesFromFile(filename);
        fineGrid.readInstancesFromFile(filename);
        middleGrid.readInstancesFromFile(filename);

        //std::cout << "INSTANCES: " << fineGrid.getInstanceCount() << " BITS: " << fineGrid.getTotalBitSize() << std::endl;
        

        // Only run these combinations:
        std::vector<RunConfig> runs = {
            //{"COARSE", &coarseGrid, {"HASHMAP ",    &Partitioner::partitionHashmap}},
            //{"MIDDLE", &middleGrid, {"HASHMAP ",    &Partitioner::partitionHashmap}},
            {"FINE  ", &fineGrid,   {"HASHMAP ",    &Partitioner::partitionHashmap}},
            {"FINE  ", &fineGrid,   {"LOCALIZE",    &Partitioner::partitionLocalized}},
            //{"MIDDLE", &middleGrid, {"LOCALIZE",    &Partitioner::partitionLocalized}},
            {"COARSE", &coarseGrid, {"LOCALIZE",    &Partitioner::partitionLocalized}},
            {" N/A  ", &coarseGrid, {"MERGING ",    &Partitioner::partitionMerging}},
            {" N/A  ", &coarseGrid, {"NEARBY  ",    &Partitioner::partitionNearby}}
        };

        int width = 800, height = 600;
        std::vector<DotWidget*> widgets;
        std::vector<Partitioner*> partitioners;

        for (const auto& run : runs) {
            auto partitioner = new Partitioner(*run.grid, 1000);
            partitioners.push_back(partitioner);

            auto t1 = high_resolution_clock::now();
            (partitioner->*run.algo.method)();
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;

            auto partitions = partitioner->getPartitions();
            // Print the table header once (before the loop)
            // Inside your loop, print each row:
            std::cout << "| "
                    << run.algo.name << " | "
                    << run.gridType << " | "
                    << instCount << " | "
                    << ms_double.count() << " | "
                    << partitioner->getPartitionsTotalRoutingLength() << " |\n";
            
            /*
            std::cout
                    << " MISSED (DNF if non zero): " << partitioner->countGridInstancesMissedInPartitions() << std::endl
                    << " UNBALANCED (DNF if non zero): " << partitioner->getViolatingBitLimitPartitionCount() << std::endl
                    << " PARTITIONS: " << partitions.size() << std::endl
                    << " AVERAGE: " << partitioner->getPartitionAverageBitSize() << std::endl
                    ;
            */
            auto* widget = new DotWidget(*run.grid, partitions);
            width = int(ceil(std::min(float(width), run.grid->getBounds().ur.x * 4)));
            height = int(ceil(std::min(float(height), run.grid->getBounds().ur.y * 4)));
            widget->resize(width, height);
            widget->setWindowTitle(
                QString("Dots - %1 - %2 - %3")
                    .arg(QString::fromStdString(run.algo.name))
                    .arg(QString::fromStdString(run.gridType))
                    .arg(QString::number(instCount))
            );
            widget->show();
            widgets.push_back(widget);
        }
        std::cout << "| | | | | |\n";
    }
    // TODO: Properly delete partitioners and widgets if needed
    return app.exec();
}