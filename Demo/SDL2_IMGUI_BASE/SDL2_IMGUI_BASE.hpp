#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "Engine.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"


class SDL2_IMGUI_BASE : public Crane::Engine
{

protected:
	std::shared_ptr<SDL_Window> window;
	std::unique_ptr<ImGuiContext, decltype(&ImGui::DestroyContext)> imguiContext;
	vk::UniqueDescriptorPool imguiPool;

	vk::UniqueCommandPool guiCommandPool;
	std::vector<vk::UniqueCommandBuffer> guiCmdBuffs;

	void addInstanceExtensions();

	void imguiInit();

    void createSurface() override;

    void createAssetApp() override;

	void updateApp() override {};

	void drawGUI() override;

	virtual void setImgui();

public:
	explicit SDL2_IMGUI_BASE(std::shared_ptr<SDL_Window> win);
	~SDL2_IMGUI_BASE();
	SDL2_IMGUI_BASE(const SDL2_IMGUI_BASE &rhs) = delete;
	SDL2_IMGUI_BASE(SDL2_IMGUI_BASE &&rhs) = delete;
	SDL2_IMGUI_BASE &operator=(const SDL2_IMGUI_BASE &rhs) = delete;
	SDL2_IMGUI_BASE &operator=(SDL2_IMGUI_BASE &&rhs) = delete;
};
