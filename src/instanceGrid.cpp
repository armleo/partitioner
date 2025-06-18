#include "instanceGrid.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

// Constructor
InstanceGrid::InstanceGrid(float binSize)
    : binSize(binSize), bounds(BoundingBox(Point2D(0, 0), Point2D(0, 0))) {}

// Add an instance and update bounds
void InstanceGrid::addInstance(const Instance& inst) {
    if (grid.empty()) {
        bounds.ll.x = bounds.ur.x = inst.getX();
        bounds.ll.y = bounds.ur.y = inst.getY();
        maxBitSize = inst.getBitsize();
    } else {
        if (inst.getX() < bounds.ll.x) bounds.ll.x = inst.getX();
        if (inst.getX() > bounds.ur.x) bounds.ur.x = inst.getX();
        if (inst.getY() < bounds.ll.y) bounds.ll.y = inst.getY();
        if (inst.getY() > bounds.ur.y) bounds.ur.y = inst.getY();
        if (inst.getBitsize() > maxBitSize) maxBitSize = inst.getBitsize();
    }
    auto cell = getCell(Point2D(inst.getX(), inst.getY()));
    grid[cell].push_back(inst);
    instanceCount += 1;
}

// Get all instances in the cell containing (x, y)
const std::vector<Instance>& InstanceGrid::getCellInstances(float x, float y) const {
    static const std::vector<Instance> empty;
    auto it = grid.find(getCell(Point2D(x, y)));
    if (it != grid.end()) return it->second;
    return empty;
}

// Get all instances within the bounding box
std::vector<Instance> InstanceGrid::getCellInstancesWithin(const BoundingBox& bbox) const {
    std::vector<Instance> result;
    int cellMinX = static_cast<int>(std::floor(bbox.ll.x / binSize));
    int cellMaxX = static_cast<int>(std::floor(bbox.ur.x / binSize));
    int cellMinY = static_cast<int>(std::floor(bbox.ll.y / binSize));
    int cellMaxY = static_cast<int>(std::floor(bbox.ur.y / binSize));

    for (int cx = cellMinX; cx <= cellMaxX; ++cx) {
        for (int cy = cellMinY; cy <= cellMaxY; ++cy) {
            auto it = grid.find({cx, cy});
            if (it != grid.end()) {
                for (const auto& inst : it->second) {
                    if (inst.getX() >= bbox.ll.x && inst.getX() <= bbox.ur.x &&
                        inst.getY() >= bbox.ll.y && inst.getY() <= bbox.ur.y) {
                        result.push_back(inst);
                    }
                }
            }
        }
    }
    return result;
}

// Reads instances from a file and adds them to the grid
void InstanceGrid::readInstancesFromFile(const std::string& filename) {
    std::ifstream infile(filename);
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string name;
        float x, y;
        unsigned int bitsize;
        if (iss >> name >> x >> y >> bitsize) {
            Instance inst(name, x, y, bitsize);
            addInstance(inst);
        }
    }
}

// Generates a file with random instances within a bounding box
void InstanceGrid::generateRandomInstancesToFile(const std::string& filename, size_t count,
                                                 const BoundingBox& searchBox, size_t nameLength) {
    std::ofstream out(filename);
    if (!out) return;

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distX(searchBox.ll.x, searchBox.ur.x);
    std::uniform_real_distribution<float> distY(searchBox.ll.y, searchBox.ur.y);
    std::uniform_int_distribution<int> distBitsize(0, 8);
    std::string charset = "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<size_t> distChar(0, charset.length() - 1);

    for (size_t i = 0; i < count; ++i) {
        std::string name;
        name.reserve(nameLength);
        for (size_t j = 0; j < nameLength; ++j) {
            name += charset[distChar(rng)];
        }
        float x = distX(rng);
        float y = distY(rng);
        int bitsize = distBitsize(rng);
        out << name << " " << x << " " << y << " " << bitsize << "\n";
    }
    out.close();
}


// Generates a file with instances distributed in Gaussian clusters
void InstanceGrid::generateGaussianClustersToFile(const std::string& filename, size_t instanceCount,
                                                 size_t clusterCount, const BoundingBox& area,
                                                 float stddev, size_t nameLength) {
    std::ofstream out(filename);
    if (!out) return;

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distX(area.ll.x, area.ur.x);
    std::uniform_real_distribution<float> distY(area.ll.y, area.ur.y);
    std::uniform_int_distribution<int> distBitsize(0, 8);
    std::string charset = "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<size_t> distChar(0, charset.length() - 1);

    // Generate cluster centers
    std::vector<Point2D> centers;
    for (size_t i = 0; i < clusterCount; ++i) {
        centers.emplace_back(distX(rng), distY(rng));
    }

    // For each instance, pick a cluster and sample from Gaussian around it
    std::uniform_int_distribution<size_t> distCluster(0, clusterCount - 1);
    std::normal_distribution<float> gauss(0.0f, stddev);

    for (size_t i = 0; i < instanceCount; ++i) {
        std::string name;
        name.reserve(nameLength);
        for (size_t j = 0; j < nameLength; ++j) {
            name += charset[distChar(rng)];
        }

        int bitsize = distBitsize(rng);

        size_t clusterIdx = distCluster(rng);
        float x = centers[clusterIdx].x + gauss(rng);
        float y = centers[clusterIdx].y + gauss(rng);

        auto withinXbounds = (x < area.ur.x) && (x > area.ll.x);
        auto withinYbounds = (y < area.ur.y) && (y > area.ll.y);
        if (withinYbounds && withinXbounds) {
            out << name << " " << x << " " << y << " " << bitsize << "\n";
        }
    }
    out.close();
}

// Returns the cell coordinates for a given Point2D
std::pair<int, int> InstanceGrid::getCell(const Point2D& p) const {
    int cellX = static_cast<int>(std::floor(p.x / binSize));
    int cellY = static_cast<int>(std::floor(p.y / binSize));
    return {cellX, cellY};
}

// Accessors for grid, bounds, and binSize
std::unordered_map<std::pair<int, int>, std::vector<Instance>, PairHash>& InstanceGrid::getGrid() {
    return grid;
}

BoundingBox& InstanceGrid::getBounds() {
    return bounds;
}

float InstanceGrid::getBinSize() {
    return binSize;
}

unsigned int InstanceGrid::getMaxBitSize() {
    return maxBitSize;
}

size_t InstanceGrid::getInstanceCount() {
    return instanceCount;
}