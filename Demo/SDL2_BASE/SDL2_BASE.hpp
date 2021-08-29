#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "Engine.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

class SDL2_BASE : public Crane::Engine
{
protected:
	std::shared_ptr<SDL_Window> window;

	void addInstanceExtensions();

    void createSurface() override;
	void createAssetApp() override {};


public:
	explicit SDL2_BASE(std::shared_ptr<SDL_Window> win);
	~SDL2_BASE() = default;
	SDL2_BASE(const SDL2_BASE &rhs) = delete;
	SDL2_BASE(SDL2_BASE &&rhs) = delete;
	SDL2_BASE &operator=(const SDL2_BASE &rhs) = delete;
	SDL2_BASE &operator=(SDL2_BASE &&rhs) = delete;

	void updateApp() override {};
};
