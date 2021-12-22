#include "OCEAN.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;


void OCEAN::updateBox()
{
	void* data;

	vkMapMemory(device.get(), ocean.deviceMeomoryAmpl.get(), 0, N * M * 2 * 4, 0, &data);
	memcpy(&height, data, N * M * 2 * 4);

	vkMapMemory(device.get(), ocean.deviceMeomoryDx.get(), 0, N * M * 2 * 4, 0, &data);
	memcpy(&dx, data, N * M * 2 * 4);

	vkMapMemory(device.get(), ocean.deviceMeomoryDz.get(), 0, N * M * 2 * 4, 0, &data);
	memcpy(&dz, data, N * M * 2 * 4);


	for (auto& b : boxs)
	{
		float xx = b.position.x();
		float zz = b.position.z();
		float h = getHeight(xx, zz);
		Vector3f position = { xx, h, zz };
		b.setPosition(position);
	}

	for (auto& b : boats)
	{
		b.seaHeight = getHeight(b.position.x(), b.position.z());
	}

	vkUnmapMemory(device.get(), ocean.deviceMeomoryAmpl.get());
	vkUnmapMemory(device.get(), ocean.deviceMeomoryDx.get());
	vkUnmapMemory(device.get(), ocean.deviceMeomoryDz.get());
}

float OCEAN::getHeight(float xx, float zz)
{
	int i = std::round( fmod((1000.f + xx), 200.f) / 200.f * float(N - 1));
	int j = std::round(fmod((1000.f + zz), 200.f) / 200.f * float(M - 1));

	/*
	// 因为dxdy，迭代找到真正的xy处的h
	for (int k = 0; k < 4; k++)
	{
		float x = dx[(i+j*N)*2];
		float z = dz[(i+j*N)*2];
		
		if (k % 2 == 0)
		{
			xx += x;
			zz += z;
		}
		else
		{
			xx -= x;
			zz -= z;
		}

		i = std::round((xx + 100.f) / 200.f * float(N - 1));
		j = std::round((zz + 100.f) / 200.f * float(M - 1));
	}
	*/
	return height[(i+j*N)*2];
}

