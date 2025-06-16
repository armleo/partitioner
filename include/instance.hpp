#pragma once
#include <string>
#include <complex>

class Instance {
public:
    Instance(const std::string& name, float x, float y);

    const std::string& getName() const;
    float getX() const;
    float getY() const;

    // Calculates Manhattan distance to another instance
    float distanceTo(const Instance& other) const;

private:
    std::string name;
    float x;
    float y;
};