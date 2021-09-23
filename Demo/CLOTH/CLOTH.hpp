#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class CLOTH : public SDL2_IMGUI_BASE
{
private:
	Crane::PipelinePassGraphics pipelinePassPhong;
	Crane::MaterialBuilderPhong materialBuilderPhong;

	Crane::Actor chessboard;

	Crane::Actor cloak;
	uint32_t offsetCloak = 0;

	Crane::Actor dragon;

	Crane::Actor soldier;

	Crane::Actor cubeTest;

	Crane::Actor sphereTest;

	CranePhysics::PositionBasedDynamics pbd;

	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

	void createChessboard();

	void createCloak();
	void createCubeTest();
	void createSphereTest();

	void createDragon();

	void createSoldiers();

public:
	explicit CLOTH(std::shared_ptr<SDL_Window> win);
	~CLOTH();
	CLOTH(const CLOTH &rhs) = delete;
	CLOTH(CLOTH &&rhs) = delete;
	CLOTH &operator=(const CLOTH &rhs) = delete;
	CLOTH &operator=(CLOTH &&rhs) = delete;
};
