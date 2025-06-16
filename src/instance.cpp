#include "instance.hpp"

Instance::Instance(const std::string& name, float x, float y)
    : name(name), x(x), y(y) {}

const std::string& Instance::getName() const {
    return name;
}

float Instance::getX() const {
    return x;
}

float Instance::getY() const {
    return y;
}

float Instance::distanceTo(const Instance& other) const {
    return std::fabs(x - other.x) + std::fabs(y - other.y);
}


