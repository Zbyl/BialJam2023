
#include "Utilities.h"

#include "zerrors.h"


#include <fstream>
#include <cmath>


std::string loadTextFile(const std::string& fileName) {
    std::ifstream input;
    input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        input.open(fileName, std::ios::binary);
    }
    catch (const std::exception& exc) {
        ZTHROW(FileNotFoundException()) << "Could not open input file: '" << fileName << "'. Error: " << exc.what();
    }

    try {
        // @todo This is an extremely inefficient implementation.
        std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        return data;
    }
    catch (const std::exception& exc) {
        ZTHROW() << "Error while reading file: '" << fileName << "'. Error: " << exc.what();
    }
}

std::tuple<int, float> divide(float value, float divisor) {
    ZASSERT(divisor > 0.0f);

    float integral;
    float fractional = std::modf(std::fabs(value) / std::fabs(divisor), &integral);
    int result = static_cast<int>(integral);
    float remainder = fractional * divisor;
    if (std::signbit(value)) {
        result = -result - 1;
        remainder = 1.0f - remainder;
    }

    return { result, remainder };
}
