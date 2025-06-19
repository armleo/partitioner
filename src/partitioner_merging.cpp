#include "partitioner.hpp"
#include <iostream>

void Partitioner::partitionMerging() {
    partitions.clear();

    // Calculate number of partitions (bins) to make bins as square as possible
    size_t totalBitSize = grid.getTotalBitSize();
    if (bitsizeLimit == 0) return;
    size_t numPartitions = std::max<size_t>(1, ceil(float(totalBitSize) / (bitsizeLimit - grid.getMaxBitSize())));

    // Get grid bounds
    BoundingBox bounds = grid.getBounds();
    float minX = bounds.ll.x;
    float maxX = bounds.ur.x;
    float minY = bounds.ll.y;
    float maxY = bounds.ur.y;
    float width = maxX - minX;
    float height = maxY - minY;

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

    float binW = width / bestNx;
    float binH = height / bestNy;

    // Split the design into equal (by dimensions) partitions
    for (size_t ix = 0; ix < bestNx; ++ix) {
        for (size_t iy = 0; iy < bestNy; ++iy) {
            float left = minX + ix * binW;
            float right = (ix == bestNx - 1) ? maxX : (left + binW);
            float bottom = minY + iy * binH;
            float top = (iy == bestNy - 1) ? maxY : (bottom + binH);

            BoundingBox binBox(Point2D(left, bottom), Point2D(right, top));
            auto instances = grid.getCellInstancesWithin(binBox);

            Partition part;
            part.centerLoc.x = (left + right) / 2.0f;
            part.centerLoc.y = (bottom + top) / 2.0f;
            for (const auto& inst : instances) {
                part.addInstance(inst);
            }
            // Always push the partition, even if empty
            partitions.push_back(std::move(part));
        }
    }

    // Balancing step: move instances from overflowing to underflowing partitions
    bool changed = true;
    while (changed) {
        changed = false;
        // Find overflowing and underflowing partitions
        std::vector<size_t> overIdx, underIdx;
        for (size_t i = 0; i < partitions.size(); ++i) {
            if (partitions[i].totalBitsize > bitsizeLimit)
                overIdx.push_back(i);
            else if (partitions[i].totalBitsize < bitsizeLimit - grid.getMaxBitSize())
                underIdx.push_back(i);
        }
        if (overIdx.empty() || underIdx.empty()) break;

        for (size_t oi : overIdx) {
            auto& over = partitions[oi];
            // Find the underflowing partition nearest to this one
            size_t nearestUnder = underIdx[0];
            float minDist = std::numeric_limits<float>::max();
            for (size_t ui : underIdx) {
                float d = std::hypot(
                    over.centerLoc.x - partitions[ui].centerLoc.x,
                    over.centerLoc.y - partitions[ui].centerLoc.y
                );
                if (d < minDist) {
                    minDist = d;
                    nearestUnder = ui;
                }
            }
            auto& under = partitions[nearestUnder];
            // Collect all instances in 'over' sorted by distance to 'under' center
            std::vector<const Instance*> sortedByDist;
            for (const auto& inst : over.instances) {
                sortedByDist.push_back(&inst);
            }
            std::sort(sortedByDist.begin(), sortedByDist.end(),
                [&under](const Instance* a, const Instance* b) {
                    float da = std::hypot(a->getX() - under.centerLoc.x, a->getY() - under.centerLoc.y);
                    float db = std::hypot(b->getX() - under.centerLoc.x, b->getY() - under.centerLoc.y);
                    return da < db;
                }
            );

            // Try to move the closest instance that fits
            const Instance* bestInst = nullptr;
            for (const Instance* inst : sortedByDist) {
                if (under.totalBitsize + inst->getBitsize() <= bitsizeLimit) {
                    bestInst = inst;
                    break;
                }
            }
            // Move the instance if it fits
            if (bestInst && (under.totalBitsize + bestInst->getBitsize() <= bitsizeLimit)) {
                under.addInstance(*bestInst);
                over.removeInstance(*bestInst);
                changed = true;
                break; // Recompute overflowing/underflowing after each move
            } else {
                std::cout << "Could not move due to bit size limit" << std::endl;
            }
        }
    }
}


