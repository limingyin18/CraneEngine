#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <numeric>

#include "SDL2_IMGUI_BASE.hpp"
#include "Actor.hpp"
#include "OCEAN.hpp"

class MINI_GAME : public SDL2_IMGUI_BASE
{
private:

	Crane::OCEAN ocean;
	std::array<Crane::Actor, 4> oceans;

	Crane::Actor chessboard;

	Crane::Actor cloak;

	Crane::Actor dragon;

	const uint32_t soldierCount = 10;
	std::vector<Crane::Actor> soldiers;
	std::vector<Eigen::Vector3f> targetPos;

	CranePhysics::PositionBasedDynamics pbd;

	void createAssetApp() override;
	void updateApp() override;

	void updateAI();

	void setImgui() override;

	void createChessboard();
	void createCloak();
	void createDragon();
	void createSoldiers();

public:
	explicit MINI_GAME(std::shared_ptr<SDL_Window> win);
	~MINI_GAME();
	MINI_GAME(const MINI_GAME &rhs) = delete;
	MINI_GAME(MINI_GAME &&rhs) = delete;
	MINI_GAME &operator=(const MINI_GAME &rhs) = delete;
	MINI_GAME &operator=(MINI_GAME &&rhs) = delete;
};
