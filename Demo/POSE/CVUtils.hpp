#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <cmath>

template <typename T>
int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

std::array<std::pair<std::pair<uint32_t, uint32_t>, float>, 17> GetMaxPreds(const std::vector<float> &batch_heatmaps);

std::array<std::pair<std::pair<float, float>, float>, 17>
PostProcessingPoints(const std::array<std::pair<std::pair<uint32_t, uint32_t>, float>, 17> &points,
					const std::vector<float> &heatmap);