#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "instance.hpp"

// Hash function for std::pair<int, int>
struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class InstanceGrid {
public:
    explicit InstanceGrid(float binSize);

    void addInstance(const Instance& inst);

    const std::vector<Instance>& getCellInstances(float x, float y) const;

    void readInstancesFromFile(const std::string& filename);
    void generateRandomInstancesToFile(const std::string& filename, size_t count,
                                       float minX, float maxX, float minY, float maxY, size_t nameLength);
        
    std::unordered_map<std::pair<int, int>, std::vector<Instance>, PairHash> grid;
private:
    float binSize;
    

    std::pair<int, int> getCell(float x, float y) const;
};