#pragma once
#include <QWidget>
#include <vector>
#include <unordered_set>
#include "instance.hpp"
#include "instanceGrid.hpp"

class Partitioner {
public:
    struct Partition {
        std::unordered_set<Instance> instances;
        unsigned int totalBitsize = 0;
        const float getTotalRoutingDistance();
    };

    Partitioner(InstanceGrid& grid, unsigned int bitsizeLimit);

    // Performs the partitioning
    void partition();
    void partitionLocalized();
    void partitionNearby();

    float getPartitionsTotalRoutingLength();

    // Returns the created partitions
    const std::vector<Partition>& getPartitions();

private:
    InstanceGrid& grid;
    unsigned int bitsizeLimit;
    std::vector<Partition> partitions;
};


class DotWidget : public QWidget {
public:
    explicit DotWidget(InstanceGrid & grid, std::vector<Partitioner::Partition>& partitions, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<Partitioner::Partition>& partitions;
    InstanceGrid& grid;
};
