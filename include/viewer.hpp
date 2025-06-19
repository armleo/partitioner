#pragma once
#include <QWidget>
#include <QPainter>
#include <vector>
#include <unordered_set>
#include "instance.hpp"
#include "instanceGrid.hpp"
#include "partitioner.hpp"

class DotWidget : public QWidget {
public:
    explicit DotWidget(InstanceGrid & grid, std::vector<Partitioner::Partition> partitions, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<Partitioner::Partition> partitions;
    InstanceGrid& grid;
};
