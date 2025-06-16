#include <instanceListReader.hpp>

// Reads instances from a file and returns a vector of Instance objects
std::vector<Instance> readInstancesFromFile(const std::string& filename) {
    std::vector<Instance> instances;
    std::ifstream infile(filename);
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string name;
        float x, y;
        if (iss >> name >> x >> y) {
            instances.emplace_back(name, x, y);
        }
    }
    return instances;
}