#pragma once
#include <QWidget>
#include <vector>
#include <unordered_set>
#include "instance.hpp"
#include "instanceGrid.hpp"

class DotWidget : public QWidget {
    
public:
    explicit DotWidget(const std::unordered_set<Instance>& instances, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const std::unordered_set<Instance>& instances;
};

class Partitioner {
public:
    struct Partition {
        std::unordered_set<Instance> instances;
        unsigned int totalBitsize = 0;
        const float getTotalRoutingDistance();
    };

    Partitioner(const InstanceGrid& grid, unsigned int bitsizeLimit);

    // Performs the partitioning
    void partition();

    // Returns the created partitions
    const std::vector<Partition>& getPartitions();

private:
    const InstanceGrid& grid;
    unsigned int bitsizeLimit;
    std::vector<Partition> partitions;
};

