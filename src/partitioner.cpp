#include "partitioner.hpp"
#include <algorithm>
#include <iostream>

using namespace std;


void Partitioner::Partition::addInstance(Instance inst) {
    instances.insert(inst);
    totalBitsize += inst.getBitsize();

    // Update center of weight (weighted by bitsize)
    float totalWeight = static_cast<float>(totalBitsize);
    if (instances.size() == 1) {
        centerLoc = inst.getLocation();
    } else {
        // Weighted average update
        float prevWeight = totalWeight - inst.getBitsize();
        centerLoc.x = (centerLoc.x * prevWeight + inst.getLocation().x * inst.getBitsize()) / totalWeight;
        centerLoc.y = (centerLoc.y * prevWeight + inst.getLocation().y * inst.getBitsize()) / totalWeight;
    }
}

void Partitioner::Partition::removeInstance(Instance inst) {
    auto it = instances.find(inst);
    if (it == instances.end()) return;

    unsigned int removedBitsize = it->getBitsize();
    instances.erase(it);
    totalBitsize -= removedBitsize;

    // Recalculate center of weight (weighted by bitsize)
    if (instances.empty()) {
        centerLoc = Point2D(0, 0);
        return;
    }

    float sumX = 0.0f, sumY = 0.0f;
    unsigned int sumBits = 0;
    for (const auto& i : instances) {
        sumX += i.getLocation().x * i.getBitsize();
        sumY += i.getLocation().y * i.getBitsize();
        sumBits += i.getBitsize();
    }
    centerLoc.x = sumX / sumBits;
    centerLoc.y = sumY / sumBits;
}

const float Partitioner::Partition::getTotalRoutingDistance() {
    if (instances.size() < 2) return 0.0f;
    float total = 0.0f;
    // Copy to vector for easier pairwise iteration
    std::vector<const Instance*> vec;
    for (const auto& inst : instances) vec.push_back(&inst);

    for (size_t i = 0; i < vec.size(); ++i) {
        float minDist = std::numeric_limits<float>::max();
        for (size_t j = 0; j < vec.size(); ++j) {
            if (i == j) continue;
            float dist = vec[i]->distanceTo(*vec[j]);
            if (dist < minDist) minDist = dist;
        }
        if (minDist < std::numeric_limits<float>::max())
            total += minDist;
    }
    return total;
}

size_t Partitioner::countGridInstancesMissedInPartitions() const {
    // Collect all instances from the grid
    std::unordered_set<Instance> gridInstances;
    for (const auto& cell : grid.getGrid()) {
        for (const auto& inst : cell.second) {
            gridInstances.insert(inst);
        }
    }

    // Collect all instances from all partitions
    std::unordered_set<Instance> partitionInstances;
    for (const auto& partition : partitions) {
        for (const auto& inst : partition.instances) {
            partitionInstances.insert(inst);
        }
    }

    // Count how many grid instances are missing from partitions
    size_t missed = 0;
    for (const auto& inst : gridInstances) {
        if (partitionInstances.find(inst) == partitionInstances.end()) {
            ++missed;
        }
    }
    return missed;
}
Partitioner::Partitioner(InstanceGrid& grid, unsigned int bitsizeLimit)
    : grid(grid), bitsizeLimit(bitsizeLimit) {}

float Partitioner::getPartitionAverageBitSize() {
    if (partitions.empty()) return 0;
    float total = 0.0;
    for (const auto& partition : partitions) {
        total += partition.totalBitsize;
    }
    return total / partitions.size();
}

size_t Partitioner::getViolatingBitLimitPartitionCount() {
    size_t count = 0;
    for (const auto& partition : partitions) {
        if (partition.totalBitsize > bitsizeLimit) ++count;
    }
    return count;
}



const std::vector<Partitioner::Partition>& Partitioner::getPartitions() {
    if (partitions.empty()) throw new runtime_error("No partitition generated yet");
    return partitions;
}
float Partitioner::getPartitionsTotalRoutingLength() {
    float total = 0;
    for(auto partition : partitions) {
        total += partition.getTotalRoutingDistance();
    }
    return total;
}

