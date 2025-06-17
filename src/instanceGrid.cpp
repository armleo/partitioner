#include <unordered_map>
#include <vector>
#include <algorithm>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <utility>
#include <fstream>
#include <random>
#include <chrono>
#include <sstream>

#include "instance.hpp"
#include "instanceGrid.hpp"


// Bins instances by integer cell coordinates (binSize x binSize)
InstanceGrid::InstanceGrid(float binSize) : binSize(binSize) {};

void InstanceGrid::addInstance(const Instance& inst) {
    if (grid.empty()) {
        minX = maxX = inst.getX();
        minY = maxY = inst.getY();
    } else {
        if (inst.getX() < minX) minX = inst.getX();
        if (inst.getX() > maxX) maxX = inst.getX();
        if (inst.getY() < minY) minY = inst.getY();
        if (inst.getY() > maxY) maxY = inst.getY();
    }

    auto cell = getCell(inst.getX(), inst.getY());
    grid[cell].push_back(inst);
};

// Returns all instances in the cell containing (x, y)
const std::vector<Instance>& InstanceGrid::getCellInstances(float x, float y) const {
    static const std::vector<Instance> empty;
    auto it = grid.find(getCell(x, y));
    if (it != grid.end()) return it->second;
    return empty;
};

/*
// Returns all cells in the bounding box
const std::vector<Instance>& InstanceGrid::getCellInstancesWithin(float minX, float minY, float maxX, float maxY) const {
    static const std::vector<Instance> empty;
    auto it = grid.find(getCell(x, y));
    if (it != grid.end()) return it->second;
    return empty;
};

*/


// Reads instances from a file and returns a vector of Instance objects
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
};

// Generates a file with random instances: "instancename x y"
void InstanceGrid::generateRandomInstancesToFile(const std::string& filename, size_t count,
                                    float minX, float maxX, float minY, float maxY, size_t nameLength) {
    std::ofstream out(filename);
    if (!out) return;

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distX(minX, maxX);
    std::uniform_real_distribution<float> distY(minY, maxY);
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

std::pair<int, int> InstanceGrid::getCell(float x, float y) const {
    int cellX = static_cast<int>(std::floor(x / binSize));
    int cellY = static_cast<int>(std::floor(y / binSize));
    return {cellX, cellY};
}



