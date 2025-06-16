#include <unordered_map>
#include <vector>
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


    // Reads instances from a file and returns a vector of Instance objects
    void InstanceGrid::readInstancesFromFile(const std::string& filename) {
        std::ifstream infile(filename);
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string name;
            float x, y;
            if (iss >> name >> x >> y) {
                Instance inst(name, x, y);
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
            out << name << " " << x << " " << y << "\n";
        }
        out.close();
    }

    std::pair<int, int> InstanceGrid::getCell(float x, float y) const {
        int cellX = static_cast<int>(std::floor(x / binSize));
        int cellY = static_cast<int>(std::floor(y / binSize));
        return {cellX, cellY};
    }