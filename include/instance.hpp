#pragma once
#include <string>
#include <complex>

class Instance {
public:
    Instance(const std::string& name, float x, float y, unsigned int bitsize);

    const std::string& getName() const;
    float getX() const;
    float getY() const;
    unsigned int getBitsize() const;

    // Calculates Manhattan distance to another instance
    float distanceTo(const Instance& other) const;

private:
    std::string name;
    float x;
    float y;
    unsigned int bitsize;
};