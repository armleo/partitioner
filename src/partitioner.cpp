#include "partitioner.hpp"
#include <QPainter>

DotWidget::DotWidget(const std::vector<Instance>& instances, QWidget* parent)
    : QWidget(parent), instances(instances) {}

void DotWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);

    // Find min/max for scaling
    float minX = instances.empty() ? 0 : instances[0].getX();
    float maxX = minX;
    float minY = instances.empty() ? 0 : instances[0].getY();
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

