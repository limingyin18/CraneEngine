#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"
#include <Module/OceanFFT/Ocean.hpp>
#include "Boat.hpp"

constexpr int N = 32;
constexpr int M = 32;

class OCEAN : public SDL2_IMGUI_BASE
{
public:
	explicit OCEAN(std::shared_ptr<SDL_Window> win);
	~OCEAN();
	OCEAN(const OCEAN &rhs) = delete;
	OCEAN(OCEAN &&rhs) = delete;
	OCEAN &operator=(const OCEAN &rhs) = delete;
	OCEAN &operator=(OCEAN &&rhs) = delete;

private:
	void createAssetApp() override;

	void updateApp() override;
	void updatePhysics();
	void updateGraphics();

private:
	Crane::Ocean ocean;
	std::array<Crane::Actor, 100> oceans;
	void createOceans();
	float getHeight(float xx, float zz);

	Crane::Actor box;
	std::array<Crane::Actor, 3000> boxs;
	uint32_t offsetBox = 0;
	void createBox();
	void updateBox();

	float height[N * M * 2];
	float dx[N * M * 2];
	float dz[N * M * 2];

	std::array<Crane::Boat, 100> boats;
	void createBoat();
};
