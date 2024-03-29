#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"
#include "ChessboardActor.hpp"
#include "ClothActor.hpp"
#include "SoldierActor.hpp"
#include "Skybox.hpp"

class CLOTH : public SDL2_IMGUI_BASE
{
private:
/*
	Crane::Actor chessboard;

	Crane::Actor cloak;
	uint32_t offsetCloak = 0;
	Crane::Actor flagCloth;
	uint32_t offsetPhyFlagCloth = 0;

	Crane::Actor dragon;
    std::vector<std::shared_ptr<CranePhysics::Rigidbody>> rgbDragon;

	Crane::Actor soldier;

	Crane::Actor cubeTest;

	Crane::Actor sphereTest;
	uint32_t sphereTestPhyIndex;

	Crane::Actor sphereA;
	Crane::Actor sphereB;
	uint32_t sphereAPhyIndex;


	std::vector<Crane::Actor> boxs;
	*/
	std::shared_ptr<ClothActor> clothActor;
	std::shared_ptr<SoldierActor> soldierActor;
	std::shared_ptr<Crane::Actor> chessboard;
    std::shared_ptr<Crane::GraphicsPrimitive> chessboardGP;
	std::shared_ptr<Crane::ChessboardActor> chessboardActor;
	CranePhysics::PositionBasedDynamics pbd;

	std::shared_ptr<SkyboxActor> skybox;

	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

	void createChessboard();

/*
	void createCloak();
	void createFlagCloth();
	void createCubeTest();
	void createSphereTest();

	void createDragon();

	void createSoldiers();
	*/

public:
	explicit CLOTH(std::shared_ptr<SDL_Window> win);
	~CLOTH();
	CLOTH(const CLOTH &rhs) = delete;
	CLOTH(CLOTH &&rhs) = delete;
	CLOTH &operator=(const CLOTH &rhs) = delete;
	CLOTH &operator=(CLOTH &&rhs) = delete;
};
