#include "instance.hpp"
#include <cmath>

Instance::Instance(const std::string& name, float x, float y, unsigned int bitsize)
    : name(name), location(x, y), bitsize(bitsize) {}

Instance::Instance(const std::string& name, const Point2D& location, unsigned int bitsize)
    : name(name), location(location), bitsize(bitsize) {}

const std::string& Instance::getName() const {
    return name;
}

float Instance::getX() const {
    return location.x;
}

float Instance::getY() const {
    return location.y;
}

const Point2D& Instance::getLocation() const {
    return location;
}

unsigned int Instance::getBitsize() const {
    return bitsize;
}

float Instance::distanceTo(const Instance& other) const {
    return std::fabs(location.x - other.location.x) + std::fabs(location.y - other.location.y);
}

bool Instance::operator==(const Instance& other) const {
    return name == other.name && location.x == other.location.x &&
           location.y == other.location.y && bitsize == other.bitsize;
}

bool Instance::operator<(const Instance& other) const {
    return location.x < other.location.x;
}