#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class CLOTH : public SDL2_IMGUI_BASE
{

private:
	Eigen::Vector3f model1{0.f, 5.f, -5.f};
	Crane::Plane cloak{11, 11};

	Crane::Plane floorEnv{2, 2};
	Eigen::Vector3f model0{0.f, 0.f, 0.f};

	CranePhysics::PositionBasedDynamics pbd;

	std::vector<Crane::Cube> particles;

	Crane::Image textureImage;
	vk::UniqueImageView textureImageView;


	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

public:
	explicit CLOTH(std::shared_ptr<SDL_Window> win);
	~CLOTH();
	CLOTH(const CLOTH &rhs) = delete;
	CLOTH(CLOTH &&rhs) = delete;
	CLOTH &operator=(const CLOTH &rhs) = delete;
	CLOTH &operator=(CLOTH &&rhs) = delete;
};
