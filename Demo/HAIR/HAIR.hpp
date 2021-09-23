#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class HAIR : public SDL2_IMGUI_BASE
{
private:
	Crane::PipelinePassGraphics pipelinePassPhong;
	Crane::MaterialBuilderPhong materialBuilderPhong;

	Crane::Actor chessboard;

	Crane::Actor hair;
	float radius = 0.01f;
	uint32_t hairRenderableOffset = 0;

	std::vector<Eigen::Vector3f> verticesHair;
	std::vector<Eigen::Vector3f> tangentsHair;
	std::vector<Eigen::Vector3f> normalsHair;

	CranePhysics::PositionBasedDynamics pbd;
	uint32_t hairPhysicsIndexOffset = 0;

	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

	void createChessboard();
	void createHair();

public:
	explicit HAIR(std::shared_ptr<SDL_Window> win);
	~HAIR();
	HAIR(const HAIR &rhs) = delete;
	HAIR(HAIR &&rhs) = delete;
	HAIR &operator=(const HAIR &rhs) = delete;
	HAIR &operator=(HAIR &&rhs) = delete;
};
