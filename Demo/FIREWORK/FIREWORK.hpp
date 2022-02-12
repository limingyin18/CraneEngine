#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <numeric>

#include "SDL2_IMGUI_BASE.hpp"
#include "Actor.hpp"

class FIREWORK final : public SDL2_IMGUI_BASE
{
public:
	explicit FIREWORK(std::shared_ptr<SDL_Window> win);
	~FIREWORK();
	FIREWORK(const FIREWORK &rhs) = delete;
	FIREWORK(FIREWORK &&rhs) = delete;
	FIREWORK &operator=(const FIREWORK &rhs) = delete;
	FIREWORK &operator=(FIREWORK &&rhs) = delete;

private:
    void createAssetApp() override;

	void updateApp() override;
	void updateAI();
	void updatePhysics();
	void updateGraphics();

	void setImgui() override;
};
