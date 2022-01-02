#include "CVUtils.hpp"

/**
 * @brief get predictions from score maps
 * 
 * @param batch_heatmaps : vector([batch_size, num_joints, height, width]) 
 */
std::array<std::pair<std::pair<uint32_t, uint32_t>, float>, 17>
GetMaxPreds(const std::vector<float> &heatmap)
{
    uint32_t joints = 17;
    uint32_t height = 64;
    uint32_t weight = 64;

    std::array<std::pair<std::pair<uint32_t, uint32_t>, float>, 17> points;

    for (uint32_t i = 0; i < joints; ++i)
    {
        float maxValue = 0.f;
        for (uint32_t j = 0; j < height; ++j)
        {
            for (uint32_t k = 0; k < weight; ++k)
            {
                float value = heatmap[i * weight * height + j * weight + k];
                if (value > maxValue)
                {
                    points[i] = {std::pair<uint32_t, uint32_t>(j, k), maxValue};
                    maxValue = value;
                }
            }
        }
    }

    return points;
}

std::array<std::pair<std::pair<float, float>, float>, 17>
PostProcessingPoints(const std::array<std::pair<std::pair<uint32_t, uint32_t>, float>, 17> &points,
                     const std::vector<float> &heatmap)
{
    uint32_t width = 64;
    uint32_t height = 64;

    std::array<std::pair<std::pair<float, float>, float>, 17> pointsPost;

    // post-processing
    for (uint32_t i = 0; i < 17; ++i)
    {
        auto p = points[i];
        pointsPost[i] = p;
        uint32_t px = std::floor(p.first.second + 0.5);
        uint32_t py = std::floor(p.first.first + 0.5);
        if (1 < px && px < (width - 1) && 1 < py && py < (height - 1))
        {
            float diffX = heatmap[i*width*height+py * width + px + 1] - heatmap[i*width*height+py * width + px - 1];
            float diffY = heatmap[i*width*height+(py + 1) * width + px] - heatmap[i*width*height+(py - 1) * width + px];
            pointsPost[i].first.second += sgn(diffX) * .25;
            pointsPost[i].first.first += sgn(diffY) * .25;
        }
    }

    return pointsPost;
}