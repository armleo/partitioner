#include "partitioner.hpp"
#include <QPainter>
#include <algorithm>


// Modified DotWidget to take a vector of sets and draw each set in a different color
DotWidget::DotWidget(const std::vector<Partitioner::Partition>& partitions, QWidget* parent)
    : QWidget(parent), partitions(partitions) {}

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
    float minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool first = true;
    for (const auto& set : partitions) {
        for (const auto& inst : set.instances) {
            if (first) {
                minX = maxX = inst.getX();
                minY = maxY = inst.getY();
                first = false;
            } else {
                if (inst.getX() < minX) minX = inst.getX();
                if (inst.getX() > maxX) maxX = inst.getX();
                if (inst.getY() < minY) minY = inst.getY();
                if (inst.getY() > maxY) maxY = inst.getY();
            }
        }
    }
    float scaleX = (maxX - minX) > 0 ? (width() - 20) / (maxX - minX) : 1.0f;
    float scaleY = (maxY - minY) > 0 ? (height() - 20) / (maxY - minY) : 1.0f;

    
    // Draw each partition in a different color
    size_t idx = 0;
    for (const auto& set : partitions) {
        painter.setPen(colors[idx % colorCount]);
        painter.setBrush(colors[idx % colorCount]);
        for (const auto& inst : set.instances) {
            int px = 10 + static_cast<int>((inst.getX() - minX) * scaleX);
            int py = 10 + static_cast<int>((inst.getY() - minY) * scaleY);
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
