#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "instance.hpp"

// Reads instances from a file and returns a vector of Instance objects
std::vector<Instance> readInstancesFromFile(const std::string& filename);