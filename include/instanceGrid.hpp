#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "instance.hpp"
#include "geom.hpp"

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
    std::vector<Instance> getCellInstancesWithin(const BoundingBox& bbox) const;

    void readInstancesFromFile(const std::string& filename);
    void generateRandomInstancesToFile(const std::string& filename, size_t count,
                                       const BoundingBox& searchBox, size_t nameLength);
    void generateGaussianClustersToFile(const std::string& filename, size_t instanceCount,
                                size_t clusterCount, const BoundingBox& area,
                                float stddev, size_t nameLength);

    
    std::pair<int, int> getCell(const Point2D& p) const;
    
    std::unordered_map<std::pair<int, int>, std::vector<Instance>, PairHash>& getGrid();
    BoundingBox& getBounds();
    float getBinSize();
    unsigned int getMaxBitSize();
    size_t getInstanceCount();
private:
    std::unordered_map<std::pair<int, int>, std::vector<Instance>, PairHash> grid;
    BoundingBox bounds;
    float binSize;
    unsigned int maxBitSize = 0;
    size_t instanceCount = 0;
};