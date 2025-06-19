#include "viewer.hpp"

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

