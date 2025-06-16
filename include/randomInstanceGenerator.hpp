#pragma once
#include <string>
#include <random>
#include <fstream>

class RandomInstanceGenerator {
public:
    RandomInstanceGenerator(float minX, float maxX, float minY, float maxY);

    // Generates a file with 'count' instances, each line: "instancename x y"
    // instancename is a random string of length 8 by default
    void generateToFile(const std::string& filename, size_t count, size_t nameLength = 8);

private:
    std::string randomName(size_t length);
    float randomFloat(float min, float max);

    float minX, maxX, minY, maxY;
    std::mt19937 rng;
};