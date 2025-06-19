#include "partitioner.hpp"
#include <QPainter>
#include <algorithm>
#include <iostream>

using namespace std;

// Modified DotWidget to take a vector of sets and draw each set in a different color
DotWidget::DotWidget(InstanceGrid & grid, std::vector<Partitioner::Partition> partitions, QWidget* parent)
    : QWidget(parent), partitions(std::move(partitions)), grid(grid) {}

void DotWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Color palette for up to 10 partitions, will repeat if more
    static const QColor colors[] = {
        Qt::red, Qt::blue, Qt::green, Qt::magenta, Qt::darkYellow,
        Qt::cyan, Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::black
    };
    constexpr int colorCount = sizeof(colors) / sizeof(colors[0]);

    
    // Find min/max for scaling
    float minX = grid.getBounds().ll.x;
    float maxX = grid.getBounds().ur.x;
    float minY = grid.getBounds().ll.y;
    float maxY = grid.getBounds().ur.y;

    float dataWidth = maxX - minX;
    float dataHeight = maxY - minY;

    float widgetWidth = width() - 20;
    float widgetHeight = height() - 20;

    // Keep aspect ratio
    float scale = 1.0f;
    if (dataWidth > 0 && dataHeight > 0) {
        float scaleX = widgetWidth / dataWidth;
        float scaleY = widgetHeight / dataHeight;
        scale = std::min(scaleX, scaleY);
    }

    // Centering offsets
    float offsetX = 10 + (widgetWidth - scale * dataWidth) / 2.0f;
    float offsetY = 10 + (widgetHeight - scale * dataHeight) / 2.0f;

    for (const auto& vect : grid.getGrid()) {
        for (auto inst : vect.second) {
            painter.setPen(Qt::white);
            painter.setBrush(Qt::white);
            int px = static_cast<int>(offsetX + (inst.getX() - minX) * scale);
            int py = static_cast<int>(offsetY + (inst.getY() - minY) * scale);
            painter.drawEllipse(QPoint(px, py), 2, 2);
        }
    }

    // Draw each partition in a different color
    size_t idx = 0;
    for (const auto& set : partitions) {
        painter.setPen(colors[idx % colorCount]);
        painter.setBrush(colors[idx % colorCount]);
        for (const auto& inst : set.instances) {
            int px = static_cast<int>(offsetX + (inst.getX() - minX) * scale);
            int py = static_cast<int>(offsetY + (inst.getY() - minY) * scale);
            painter.drawEllipse(QPoint(px, py), 2, 2);
        }
        ++idx;
    }

    // Draw scale text at the bottom
    painter.setPen(Qt::blue);
    QString scaleText = QString("X: [%1, %2], Y: [%3, %4]")
        .arg(minX).arg(maxX).arg(minY).arg(maxY);
    painter.drawText(10, height() - 10, scaleText);
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

size_t Partitioner::getPartitionAverageBitSize() {
    if (partitions.empty()) return 0;
    size_t total = 0;
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


    for (size_t ix = 0; ix < bestNx; ++ix) {
        for (size_t iy = 0; iy < bestNy; ++iy) {
            float left = minX + ix * binW;
            float right = (ix == bestNx - 1) ? maxX : (left + binW);
            float bottom = minY + iy * binH;
            float top = (iy == bestNy - 1) ? maxY : (bottom + binH);

            BoundingBox binBox(Point2D(left, bottom), Point2D(right, top));
            auto instances = grid.getCellInstancesWithin(binBox);

            Partition part;
            for (const auto& inst : instances) {
                part.addInstance(inst);
                
            }
            if (!part.instances.empty())
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
            else if (partitions[i].totalBitsize < bitsizeLimit)
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
                cout << "Could not move due to bit size limit" << endl;
            }
        }
    }
}

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

    float binH = height / bestNy;

    // Track visited instances to avoid duplicates
    std::unordered_set<Instance> visited;

    for (size_t iy = 0; iy < bestNy; ++iy) {
        float bottom = minY + iy * binH;
        float top = (iy == bestNy - 1) ? maxY : (bottom + binH);

        float curX = minX;
        while (curX < maxX) {
            Partition current;
            float nextX = curX;
            bool reachedLimit = false;

            while (!reachedLimit && nextX < maxX) {
                float right = (nextX + width / bestNx > maxX) ? maxX : (nextX + width / bestNx);
                BoundingBox box(Point2D(nextX, bottom), Point2D(right, top));
                auto instances = grid.getCellInstancesWithin(box);

                // Only add unvisited instances
                for (const auto& inst : instances) {
                    if (visited.count(inst) == 0) {
                        if (current.totalBitsize + inst.getBitsize() > bitsizeLimit && !current.instances.empty()) {
                            reachedLimit = true;
                            break;
                        }
                        current.addInstance(inst);
                        visited.insert(inst);
                    }
                }
                if (!reachedLimit)
                    nextX = right;
            }

            if (!current.instances.empty())
                partitions.push_back(std::move(current));

            curX = (curX == nextX) ? curX + width / bestNx : nextX; // Avoid infinite loop if nothing was added
        }
    }

    // TODO: 

}
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



void Partitioner::partition() {
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

const std::vector<Partitioner::Partition>& Partitioner::getPartitions() {
    if (partitions.empty()) Partitioner::partition();
    return partitions;
}
float Partitioner::getPartitionsTotalRoutingLength() {
    float total = 0;
    for(auto partition : partitions) {
        total += partition.getTotalRoutingDistance();
    }
    return total;
}

