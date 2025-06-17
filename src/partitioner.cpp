#include "partitioner.hpp"
#include <QPainter>
#include <algorithm>


// Modified DotWidget to take a vector of sets and draw each set in a different color
DotWidget::DotWidget(const InstanceGrid & grid, const std::vector<Partitioner::Partition>& partitions, QWidget* parent)
    : QWidget(parent), partitions(partitions), grid(grid) {}

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
    float minX = grid.minX;
    float maxX = grid.maxX;
    float minY = grid.minY;
    float maxY = grid.maxY;

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

Partitioner::Partitioner(const InstanceGrid& grid, unsigned int bitsizeLimit)
    : grid(grid), bitsizeLimit(bitsizeLimit) {}



void Partitioner::partitionLocalized() {
    partitions.clear();
    
    Partition current;
    float x = grid.minX;
    float y = grid.minY;
    
    for (auto& it : grid.grid) {
        // TODO: Iterate by increasing X and Y
        for(auto& inst : it.second) {
            if (current.totalBitsize + inst.getBitsize() > bitsizeLimit && !current.instances.empty()) {
                partitions.push_back(current);
                current = Partition();
            }
            current.instances.emplace(inst);
            current.totalBitsize += inst.getBitsize();
        }
    }

    if (!current.instances.empty()) {
        partitions.push_back(current);
    }
}

void Partitioner::partitionNearby() {
    partitions.clear();

    // Collect all instances and mark them as unassigned
    std::unordered_set<const Instance*> unassigned;
    for (const auto& cell : grid.grid) {
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
                    current.instances.insert(*inst);
                    current.totalBitsize += inst->getBitsize();
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
    for (auto& it : grid.grid) {
        // TODO: Iterate by increasing X and Y
        for(auto& inst : it.second) {
            if (current.totalBitsize + inst.getBitsize() > bitsizeLimit && !current.instances.empty()) {
                partitions.push_back(current);
                current = Partition();
            }
            current.instances.emplace(inst);
            current.totalBitsize += inst.getBitsize();
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

