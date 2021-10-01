#include "Atmosphere.hpp"
#include "Physics/Intersect.hpp"


using namespace std;
using namespace Eigen;
using namespace CranePhysics;

Eigen::Vector3f Atmosphere::computeIncidentLight(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir,
	float tmin, float tmax) const
{
	// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky

	auto rect = RaySphereIntersect(orig, dir, Vector3f{ 0.f, 0.f, 0.f }, atmosphereRadius);
	if (!rect.has_value())
		return { 0.f, 0.f, 0.f };
	float t0 = rect.value().first;
	float t1 = rect.value().second;
    if (t0 > tmin && t0 > 0) tmin = t0;
    if (t1 < tmax) tmax = t1;

    uint32_t numSamples = 16;
    uint32_t numSamplesLight = 8;
    float segmentLength = (tmax - tmin) / numSamples;
    float tCurrent = tmin;
    Vector3f sumR{ 0.f, 0.f, 0.f }, sumM{ 0.f, 0.f, 0.f }; // mie and rayleigh contribution 
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dir.dot(sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction 
    float phaseR = 3.f / (16.f * std::numbers::pi) * (1 + mu * mu);
    float g = 0.76f;
    float phaseM = 3.f / (8.f * std::numbers::pi) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));

    Vector3f samplePosition;
    for (uint32_t i = 0; i < numSamples; ++i) 
    {
        samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir;
        float height = samplePosition.norm() - earthRadius;
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength;
        float hm = exp(-height / Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
        auto rect = RaySphereIntersect(samplePosition, sunDirection, Vector3f{ 0.f, 0.f, 0.f }, atmosphereRadius);
        t0Light = rect.value().first;
        t1Light = rect.value().second;
        float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        uint32_t j;

        Vector3f samplePositionLight;
        for (j = 0; j < numSamplesLight; ++j)
        {
            samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection;
            float heightLight = samplePositionLight.norm() - earthRadius;
            if (heightLight < 0) break;
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight) {
            Vector3f tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
            Vector3f attenuation(exp(-tau.x()), exp(-tau.y()), exp(-tau.z()));
            sumR += attenuation * hr;
            sumM += attenuation * hm;
        }
        tCurrent += segmentLength;
    }

    // We use a magic number here for the intensity of the sun (20). We will make it more
    // scientific in a future revision of this lesson/code
    return (sumR.cwiseProduct(betaR) * phaseR + sumM.cwiseProduct(betaM) * phaseM) * 20;
}

void renderSkydome(const Vector3f& sunDir, const char* filename)
{
    Atmosphere atmosphere(sunDir);
    auto t0 = std::chrono::high_resolution_clock::now();

    const unsigned width = 512, height = 512;
    Vector3f* image = new Vector3f[width * height], * p = image;
    memset(image, 0x0, sizeof(Vector3f) * width * height);
    for (unsigned j = 0; j < height; ++j) {
        float y = 2.f * (j + 0.5f) / float(height - 1) - 1.f;
        for (unsigned i = 0; i < width; ++i, ++p) {
            float x = 2.f * (i + 0.5f) / float(width - 1) - 1.f;
            float z2 = x * x + y * y;
            if (z2 <= 1) {
                float phi = std::atan2(y, x);
                float theta = std::acos(1 - z2);
                Vector3f dir(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
                // 1 meter above sea level
                *p = atmosphere.computeIncidentLight(Vector3f(0, atmosphere.earthRadius + 1, 0), dir, 0, numeric_limits<float>::max());
            }
        }
        fprintf(stderr, "\b\b\b\b\%3d%c", (int)(100 * j / (width - 1)), '%');
    }

    std::cout << "\b\b\b\b" << ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - t0)).count() << " seconds" << std::endl;
    // Save result to a PPM image (keep these flags if you compile under Windows)
    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    p = image;
    for (unsigned j = 0; j < height; ++j) {
        for (unsigned i = 0; i < width; ++i, ++p) {
#if 1 
            // Apply tone mapping function
            (*p)[0] = (*p)[0] < 1.413f ? pow((*p)[0] * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-(*p)[0]);
            (*p)[1] = (*p)[1] < 1.413f ? pow((*p)[1] * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-(*p)[1]);
            (*p)[2] = (*p)[2] < 1.413f ? pow((*p)[2] * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-(*p)[2]);
#endif 
            ofs << (unsigned char)(std::min(1.f, (*p)[0]) * 255)
                << (unsigned char)(std::min(1.f, (*p)[1]) * 255)
                << (unsigned char)(std::min(1.f, (*p)[2]) * 255);
        }
    }
    ofs.close();
    delete[] image;
}