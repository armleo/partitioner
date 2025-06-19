#include "partitioner.hpp"

void Partitioner::partitionHashmap() {
    partitions.clear();

    Partition current;
    for (auto& it : grid.getGrid()) {
        for(auto& inst : it.second) {
            if (current.totalBitsize + inst.getBitsize() > bitsizeLimit && !current.instances.empty()) {
                partitions.push_back(current);
                current = Partition();
            }
            
            current.addInstance(inst);
        }
    }
    if (!current.instances.empty()) {
        partitions.push_back(current);
    }
}
