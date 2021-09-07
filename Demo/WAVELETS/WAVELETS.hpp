#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

#include "WaveletsAmpl.hpp"

class WAVELETS : public SDL2_IMGUI_BASE
{
private:
	Crane::Plane ocean;
	Crane::Buffer indexB;
	vk::DescriptorBufferInfo indexBInfo;
	Crane::WaveletsAmpl waveletsAmpl;

	Crane::PipelinePass amplPipelinePass;
	Crane::Material materialAmpl;
	Crane::Buffer ampl, normalX, normalZ, dx, dz, h_tlide_0, h_tlide_0_conj;
	vk::DescriptorBufferInfo amplInfo, normalXInfo, normalZInfo, dxInfo, dzInfo, h_tlide_0Info, h_tlide_0_conjInfo;
	Crane::Buffer NB, MB, Lx, Lz, t;
	vk::DescriptorBufferInfo NBInfo, MBInfo, LxInfo, LzInfo, tInfo;

	Crane::PipelinePass iff2PipelinePass;
	Crane::Material materialIff2Ampl, materialIff2NormalX, materialIff2NormalZ, materialIff2Dx, materialIff2Dz;

	Crane::PipelinePass pipelinePassSign;
	Crane::Material materialSignAmpl, materialSignNormalX, materialSignNormalZ, materialSignDx, materialSignDz;

	vk::UniqueCommandPool computeCommandPool;
	std::vector<vk::UniqueCommandBuffer> computeCommandBuffers;
	vk::SubmitInfo computeSubmitInfo;
	void createOceanSSBO(Crane::Buffer& buff, vk::DescriptorBufferInfo& info, uint32_t binding);
	void createOceanUniform(Crane::Buffer& buff, vk::DescriptorBufferInfo& info, uint32_t binding);
	void bindIff2SSBO(Crane::Material &material, vk::DescriptorBufferInfo& info);


	Crane::PipelinePass pipelinePassOcean;
	Crane::Material materialOcean;
	Crane::Buffer lambda;
	vk::DescriptorBufferInfo lambdaInfo;

	Crane::Plane floorEnv{2, 2};
	Eigen::Vector3f model0{0.f, 0.f, 0.f};
	Crane::MaterialPhong materialPhong;

	Crane::Cube cube{ 2 };

	Crane::Image textureImage;
	vk::UniqueImageView textureImageView;


	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

public:
	explicit WAVELETS(std::shared_ptr<SDL_Window> win);
	~WAVELETS();
	WAVELETS(const WAVELETS &rhs) = delete;
	WAVELETS(WAVELETS &&rhs) = delete;
	WAVELETS &operator=(const WAVELETS &rhs) = delete;
	WAVELETS &operator=(WAVELETS &&rhs) = delete;
};
