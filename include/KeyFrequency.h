#ifndef KEYFREQUENCY_H
#define KEYFREQUENCY_H

/**
 * @file KeyFrequency.h
 * @brief Key-Frequency data structure.
 * @author Guillermo M. Paris
 * @date 2020-01-08
 */

#include <string>
#include <vector>

struct KeyFrequency
{
    KeyFrequency(const std::string&& sKey, const std::string&& sFreq)
      : key(sKey), frequency(sFreq) {}

    std::string  key;
    std::string  frequency;
};

using KeyFrequencyVector = std::vector<KeyFrequency>;

#endif // KEYFREQUENCY_H

