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
        // Start with any unassigned instance
        const Instance* currentInst = *unassigned.begin();
        current.addInstance(*currentInst);
        unassigned.erase(currentInst);

        while (current.totalBitsize < bitsizeLimit && !unassigned.empty()) {
            // Find the nearest unassigned instance to currentInst
            const Instance* nearest = nullptr;
            float minDist = std::numeric_limits<float>::max();
            for (const Instance* cand : unassigned) {
                float dist = currentInst->distanceTo(*cand);
                if (dist < minDist) {
                    minDist = dist;
                    nearest = cand;
                }
            }
            if (!nearest) break;
            if (current.totalBitsize + nearest->getBitsize() > bitsizeLimit)
                break;
            current.addInstance(*nearest);
            unassigned.erase(nearest);
            currentInst = nearest;
        }
        if (!current.instances.empty())
            partitions.push_back(std::move(current));
    }
}