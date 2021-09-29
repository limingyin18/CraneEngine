#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "Engine.hpp"

#include "OceanAmpl.hpp"

namespace Crane
{
	class OCEAN
	{
	public:
		std::shared_ptr<MeshBase> mesh;
		Material *material;
	private:
		// render

		Render* context = nullptr;
		Crane::PipelinePassGraphics pipelinePass;
		MaterialBuilder materialBuilder;
		Crane::Buffer lambda;

		void createRender();

		// ampl

		Crane::OceanAmpl oceanAmpl;
		Crane::PipelinePassCompute amplPipelinePass;
		Crane::MaterialBuilder materialBuilderAmpl;
		Crane::Material materialAmpl;
		Crane::Buffer h_tlide_0, h_tlide_0_conj;
		Crane::Buffer NB, MB, Lx, Lz, t;

		vk::UniqueBuffer bufferAmpl, bufferNormalX, bufferNormalZ, bufferDx, bufferDz;
		vk::UniqueImage imageAmpl, imageNormalX, imageNormalZ, imageDx, imageDz;
		vk::UniqueImageView imageViewAmpl, imageViewNormalX, imageViewNormalZ, imageViewDx, imageViewDz;
		vk::UniqueDeviceMemory deviceMeomoryAmpl, deviceMeomoryNormalX, deviceMeomoryNormalZ, deviceMeomoryDx, deviceMeomoryDz;
		vk::DescriptorBufferInfo descriptorBufferInfoAmpl, descriptorBufferInfoNormalX, descriptorBufferInfoNormalZ, descriptorBufferInfoDx, descriptorBufferInfoDz;
		vk::DescriptorImageInfo descriptorImageInfoAmpl, descriptorImageInfoNormalX, descriptorImageInfoNormalZ, descriptorImageInfoDx, descriptorImageInfoDz;
		void createBufferOcean(vk::UniqueBuffer &buffer, vk::UniqueImage &image, vk::UniqueImageView &imageView, vk::UniqueDeviceMemory &deviceMemory, 
			vk::DescriptorBufferInfo &descriptorBufferInfo, vk::DescriptorImageInfo &descriptorImageInfo);


		void createAmpl();

		// ifft2

		Crane::PipelinePassCompute iff2PipelinePass;
		Crane::MaterialBuilder materialBuilderIfft2;
		Crane::Material materialIff2Ampl, materialIff2NormalX, materialIff2NormalZ, materialIff2Dx, materialIff2Dz;

		void createIfft2();

		// sign

		Crane::PipelinePassCompute pipelinePassSign;
		Crane::MaterialBuilder materialBuilderSign;
		Crane::Material materialSignAmpl, materialSignNormalX, materialSignNormalZ, materialSignDx, materialSignDz;

		void createSign();


		// command
		vk::UniqueCommandPool computeCommandPool;
		std::vector<vk::UniqueCommandBuffer> computeCommandBuffers;
		vk::SubmitInfo computeSubmitInfo;

		vk::Queue computeQueue;

		void createCommand();


	public:
		explicit OCEAN();
		~OCEAN() = default;
		OCEAN(const OCEAN& rhs) = delete;
		OCEAN(OCEAN&& rhs) = delete;
		OCEAN& operator=(const OCEAN& rhs) = delete;
		OCEAN& operator=(OCEAN&& rhs) = delete;

		void init(Crane::Render* ctx);
		void update(float dtAll);
	};
}
