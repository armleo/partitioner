#include "randomInstanceGenerator.hpp"
#include <chrono>
#include <algorithm>

// Constructor: initializes bounds and random number generator
RandomInstanceGenerator::RandomInstanceGenerator(float minX, float maxX, float minY, float maxY)
    : minX(minX), maxX(maxX), minY(minY), maxY(maxY) {
    // Seed with current time for randomness
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

// Generates a random float in [min, max]
float RandomInstanceGenerator::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

// Generates a random string of given length (alphanumeric)
std::string RandomInstanceGenerator::randomName(size_t length) {
    std::string charset =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    std::uniform_int_distribution<size_t> dist(0, charset.length() - 1);


    std::string result;
    result.reserve(length);

    for (int i = 0; i < length; ++i) {
        result += charset[dist(rng)];
    }
    return result;
}

// Generates a file with random instances
void RandomInstanceGenerator::generateToFile(const std::string& filename, size_t count, size_t nameLength) {
    std::ofstream out(filename);
    if (!out) return;
    for (int i = 0; i < count; ++i) {
        std::string name = randomName(nameLength);
        float x = randomFloat(minX, maxX);
        float y = randomFloat(minY, maxY);
        out << name << " " << x << " " << y << "\n";
    }
    out.close();
}