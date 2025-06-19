#pragma once
#include <QWidget>
#include <vector>
#include <unordered_set>
#include "instance.hpp"
#include "instanceGrid.hpp"

class Partitioner {
public:
    class Partition {
        public:
            void addInstance(Instance inst);
            void removeInstance(Instance inst);
            const float getTotalRoutingDistance();

            std::unordered_set<Instance> instances;
            unsigned int totalBitsize = 0;
            Point2D centerLoc = Point2D(0, 0);
    };

    Partitioner(InstanceGrid& grid, unsigned int bitsizeLimit);

    // Performs the partitioning
    void partitionHashmap();
    void partitionLocalized();
    void partitionNearby();
    void partitionMerging();

    float getPartitionsTotalRoutingLength();
    float getPartitionAverageBitSize();
    size_t getViolatingBitLimitPartitionCount();
    size_t countGridInstancesMissedInPartitions() const;

    // Returns the created partitions
    const std::vector<Partition>& getPartitions();

private:
    InstanceGrid& grid;
    unsigned int bitsizeLimit;
    std::vector<Partition> partitions;
    
};

