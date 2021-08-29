#pragma once

#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "spirv_reflect.h"

#include "Logging.hpp"
#include "MeshBase.hpp"
#include "Buffer.hpp"


namespace Crane
{
	class PipelineBuilder
	{
	public:
		PipelineBuilder();

		vk::Device device;
		vk::PipelineCache pipelineCache;

		std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions;
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
		vk::PipelineVertexInputStateCreateInfo vi;

		vk::PipelineInputAssemblyStateCreateInfo ia;

		vk::PipelineRasterizationStateCreateInfo rs;

		vk::PipelineColorBlendAttachmentState att_state[1];
		vk::PipelineColorBlendStateCreateInfo cb;

		vk::Viewport viewport;

		vk::Rect2D scissor;

		vk::PipelineViewportStateCreateInfo vp;

		vk::PipelineDepthStencilStateCreateInfo ds;

		vk::PipelineMultisampleStateCreateInfo ms;


		vk::UniquePipeline build(const std::vector<vk::PipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfos,
			vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass);
	};


	enum class PipelineType
	{
		BASIC = 0,
		PHONG,
		COMPUTE
	};

	class PipelinePass
	{
	public:
		PipelineType pipelineType;
		vk::Device device;
		vk::UniquePipeline pipeline;
		vk::UniquePipelineLayout layout;

		std::vector<vk::UniqueShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;

		std::vector<vk::PushConstantRange> pushConstantRanges;
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>> bindings;
		std::vector<vk::UniqueDescriptorSetLayout> setLayouts;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

		vk::RenderPass renderPass;

		void addShader(std::vector<char>& shaderCode, vk::ShaderStageFlagBits);
		void buildLayout();
	};

	class Material
	{
	public:
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
		std::vector<vk::DescriptorSet> descriptorSets;

		vk::DescriptorPool descriptorPool;

		PipelinePass* pipelinePass;
		void buildSets();
	};

	class MaterialPhong : public Material
	{
	public:
		vk::ImageView imageView;
		vk::DescriptorImageInfo imageInfo;
	};

	class MaterialCompute : public Material
	{
	public:
		std::vector<Crane::Buffer> buffers;
	};
}
