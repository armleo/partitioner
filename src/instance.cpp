#include "instance.hpp"

Instance::Instance(const std::string& name, float x, float y, unsigned int bitsize)
    : name(name), x(x), y(y), bitsize(bitsize) {}

const std::string& Instance::getName() const {
    return name;
}

float Instance::getX() const {
    return x;
}

float Instance::getY() const {
    return y;
}

unsigned int Instance::getBitsize() const {
    return bitsize;
}

float Instance::distanceTo(const Instance& other) const {
    return std::fabs(x - other.x) + std::fabs(y - other.y);
}


