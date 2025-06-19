#include "partitioner.hpp"

void Partitioner::partitionNearby() {
    partitions.clear();

    // Collect all instances and mark them as unassigned
    std::unordered_set<const Instance*> unassigned;
    for (const auto& cell : grid.getGrid()) {
        for (const auto& inst : cell.second) {
            unassigned.insert(&inst);
        }
    }

    while (!unassigned.empty()) {
        Partition current;
        // Start with the first unassigned instance
        const Instance* seed = *unassigned.begin();
        std::vector<const Instance*> toVisit = {seed};

        while (!toVisit.empty()) {
            // Find the nearest unassigned instance to any in toVisit
            const Instance* nearest = nullptr;
            float minDist = std::numeric_limits<float>::max();
            for (const Instance* from : toVisit) {
                for (const Instance* cand : unassigned) {
                    if (from == cand) continue;
                    float dist = from->distanceTo(*cand);
                    if (dist < minDist) {
                        minDist = dist;
                        nearest = cand;
                    }
                }
            }

            // Add all in toVisit to the partition
            for (const Instance* inst : toVisit) {
                if (unassigned.count(inst) &&
                    current.totalBitsize + inst->getBitsize() <= bitsizeLimit) {
                    current.addInstance(*inst);
                    unassigned.erase(inst);
                }
            }

            // Prepare next toVisit
            toVisit.clear();
            if (nearest && current.totalBitsize + nearest->getBitsize() <= bitsizeLimit) {
                toVisit.push_back(nearest);
            }
        }

        if (!current.instances.empty()) {
            partitions.push_back(std::move(current));
        }
    }
}

