// CraneVision.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <string>
#include <stack>
#include <set>

#include <vk_mem_alloc.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "Logging.hpp"
#include "Utils.hpp"
#include "Buffer.hpp"

// TODO: Reference additional headers your program requires here.

namespace CraneEngine
{
	class RenderEngine
	{
	public:
		VisionEngine();

		void init();
		void train();

	private:
		void createInstance();
		void validataInstanceExtensions();
		void validataInstanceLayers();

		void createLogicalDevice();
		void validataDeviceExtensions();
		void getPhysicalDevice();
		void getQueueFamilyIndex();

		void createVmaAllocator();

		void createAsset();

		vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);
		void createShader();

		void createDescriptorLayout();
		void createDescriptorPool();
		void createDescriptorSet();
		void createPipelineLayout();
		void createPipelineCache();
		void createPipeline();

		void createCommandPool();
		void allocateCommandBuffer();
		void buildCommandBuffer();

		vk::CommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(vk::CommandBuffer cmdBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		// data

		

		std::stack<std::function<void()>> deletors;

		std::string appName;
		std::string engineName;
		uint32_t apiVersion;
		uint32_t appVersion;
		uint32_t engineVersion;

		std::vector<const char*> instanceExtensions;
		std::vector<const char*> layers;
		std::vector<const char*> deviceExtensions;

		vk::UniqueInstance instance;
		vk::UniqueDebugUtilsMessengerEXT debugMessenger;
		vk::PhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties gpuProperties;
		vk::UniqueDevice device;
		unsigned computeQueueFamilyIndex;
		vk::Queue computeQueue;

		vk::UniqueCommandPool commandPool;
		std::vector<vk::UniqueCommandBuffer> commandBuffer;
		vk::SubmitInfo submitInfo;

		VmaAllocator vmaAllocator;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
		vk::UniqueDescriptorSetLayout descriptorSetLayout;
		vk::UniqueDescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSet;

		vk::UniqueShaderModule computeShader;
		std::vector<vk::SpecializationMapEntry> specializationMapEntries;
		vk::SpecializationInfo specializationInfo;
		vk::UniquePipelineLayout pipelineLayout;
		vk::UniquePipelineCache pipelineCache;
		vk::UniquePipeline pipeline;

		vk::DescriptorBufferInfo inputBufferInfo, outputBufferInfo;

		Buffer inputBuffer, outputBuffer;
	};
}
