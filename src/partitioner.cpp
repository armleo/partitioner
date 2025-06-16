#include "partitioner.hpp"
#include <QPainter>
#include <algorithm>


DotWidget::DotWidget(const std::unordered_set<Instance>& instances, QWidget* parent)
    : QWidget(parent), instances(instances) {}

void DotWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);

    // Find min/max for scaling
    float minX = instances.empty() ? 0 : (*instances.begin()).getX();
    float maxX = minX;
    float minY = instances.empty() ? 0 : (*instances.begin()).getY();
    float maxY = minY;
    for (const auto& inst : instances) {
        if (inst.getX() < minX) minX = inst.getX();
        if (inst.getX() > maxX) maxX = inst.getX();
        if (inst.getY() < minY) minY = inst.getY();
        if (inst.getY() > maxY) maxY = inst.getY();
    }
    float scaleX = (maxX - minX) > 0 ? (width() - 20) / (maxX - minX) : 1.0f;
    float scaleY = (maxY - minY) > 0 ? (height() - 20) / (maxY - minY) : 1.0f;

    for (const auto& inst : instances) {
        int px = 10 + static_cast<int>((inst.getX() - minX) * scaleX);
        int py = 10 + static_cast<int>((inst.getY() - minY) * scaleY);
        painter.drawEllipse(QPoint(px, py), 2, 2);
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
    for (const auto& it : grid.grid) {
        for(const auto& inst : it.second) {
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
