#pragma once
#include <QWidget>
#include <vector>
#include "instance.hpp"

class DotWidget : public QWidget {
    
public:
    explicit DotWidget(const std::vector<Instance>& instances, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const std::vector<Instance>& instances;
};


#include "partitioner.moc"