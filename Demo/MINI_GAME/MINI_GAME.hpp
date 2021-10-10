#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <numeric>

#include "SDL2_IMGUI_BASE.hpp"
#include "Actor.hpp"
#include "OCEAN.hpp"
#include "Sky.hpp"

class MINI_GAME : public SDL2_IMGUI_BASE
{
private:

	Crane::OCEAN ocean;
	std::array<Crane::Actor, 4> oceans;

	Crane::Actor chessboard;

	Crane::Actor ball;
	uint32_t offsetBall = 0;
	void createBall();

	Crane::Actor cloak;
	uint32_t offsetCloak = 0;

	Crane::Actor dragon;

	Crane::Actor human;
	uint32_t IDHuman = 0;
	void createHuman();
	std::vector<Crane::Actor> cubesHuman;

	Crane::Sky sky;

	const uint32_t soldierCount = 100;
	std::vector<Crane::Actor> soldiers;
	std::vector<Eigen::Vector3f> targetPos;

	CranePhysics::PositionBasedDynamics pbd;

	void createAssetApp() override;
	void updateApp() override;

	void updateAI();
	void updatePhysics();
	void updateGraphics();

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
