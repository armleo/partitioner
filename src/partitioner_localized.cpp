#include "partitioner.hpp"

void Partitioner::partitionLocalized() {
    partitions.clear();

    // Get grid bounds
    BoundingBox bounds = grid.getBounds();
    float minX = bounds.ll.x;
    float maxX = bounds.ur.x;
    float minY = bounds.ll.y;
    float maxY = bounds.ur.y;
    float width = maxX - minX;
    float height = maxY - minY;

    // Calculate number of partitions (bins) to make bins as square as possible (like merging)
    size_t totalBitSize = grid.getTotalBitSize();
    if (bitsizeLimit == 0) return;
    size_t numPartitions = std::max<size_t>(1, ceil(float(totalBitSize) / (bitsizeLimit - grid.getMaxBitSize())));

    // Find grid of bins (nx, ny) such that nx * ny >= numPartitions and bins are as square as possible
    size_t bestNx = 1, bestNy = numPartitions;
    float bestRatio = std::numeric_limits<float>::max();
    for (size_t nx = 1; nx <= numPartitions; ++nx) {
        size_t ny = (numPartitions + nx - 1) / nx; // ceil division
        float binW = width / nx;
        float binH = height / ny;
        float ratio = std::abs(binW - binH);
        if (ratio < bestRatio) {
            bestRatio = ratio;
            bestNx = nx;
            bestNy = ny;
        }
    }

    float binW = grid.getBinSize();
    float binH = height / bestNy;

    // Track visited instances to avoid duplicates
    std::unordered_set<Instance> visited;

    Partition current;

    std::unordered_set<Instance> reminders;
    float remMinY = std::numeric_limits<float>::max();
    float remMaxY = std::numeric_limits<float>::lowest();
    float remMinX = std::numeric_limits<float>::max();
    float remMaxX = std::numeric_limits<float>::lowest();

    for (size_t iy = 0; iy < bestNy; ++iy) {
        float bottom = minY + iy * binH;
        float top = (iy == bestNy - 1) ? maxY : (bottom + binH);

        float curX = minX;
        while (curX < maxX) {
            // Gather all unvisited instances in the current window
            float right = (curX + binW > maxX) ? maxX : (curX + binW);
            BoundingBox box(Point2D(curX, bottom), Point2D(right, top));
            auto allInstances = grid.getCellInstancesWithin(box);


            for (const auto& inst : allInstances) {
                // Fill partition up to bitsizeLimit
                if((current.totalBitsize + inst.getBitsize() <= bitsizeLimit)) {
                    current.addInstance(inst);
                    visited.insert(inst);
                }
                if(current.totalBitsize >= bitsizeLimit - grid.getMaxBitSize()) {
                    partitions.push_back(current);
                    current = Partition();
                }
            }
            curX = right;
        }
        // After the inner X loop, push any remaining instances in 'current' to reminders
        for (const auto& inst : current.instances) {
            reminders.insert(inst);
            if (inst.getY() < remMinY) remMinY = inst.getY();
            if (inst.getY() > remMaxY) remMaxY = inst.getY();
            if (inst.getX() < remMinX) remMinX = inst.getX();
            if (inst.getX() > remMaxX) remMaxX = inst.getX();
        }
        current = Partition();
    }

    // If there are reminders, calculate their bounding box
    if (!reminders.empty()) {
        float gridStep = grid.getBinSize();
        float curY = remMinY;
        std::unordered_set<Instance> handled;

        while (curY < remMaxY) {
            float top = std::min(curY + gridStep, remMaxY);
            BoundingBox box(Point2D(remMinX, curY), Point2D(remMaxX, top));
            auto allInstances = grid.getCellInstancesWithin(box);

            // Only consider reminders that haven't been handled yet
            std::vector<Instance> unassigned;
            for (const auto& inst : allInstances) {
                if (reminders.count(inst) && handled.count(inst) == 0) {
                    unassigned.push_back(inst);
                }
            }
            
            for (auto inst : unassigned) {
                current.addInstance(inst);
                handled.insert(inst);
                if(current.totalBitsize >= bitsizeLimit - grid.getMaxBitSize()) {
                    partitions.push_back(current);
                    current = Partition();
                }
            }

            curY = top;
        }

        if(!current.instances.empty()) {
            partitions.push_back(current);
            current = Partition();
        }
    }
}

