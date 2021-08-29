// CraneVision.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <string>
#include <stack>
#include <set>
#include <tuple>

#include <vk_mem_alloc.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <Eigen/Eigen>

#include "Logging.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Camera.hpp"
#include "RenderableBase.hpp"
#include "MeshBase.hpp"
#include "MaterialSystem.hpp"


namespace Crane
{
	class Render
	{
	protected:
		struct SceneParameters
		{
			Eigen::Vector4f fogColor;     // w is for exponent
			Eigen::Vector4f fogDistances; //x for min, y for max, zw unused.
			Eigen::Vector4f ambientColor;
			Eigen::Vector4f sunlightDirection; //w for sun power
			Eigen::Vector4f sunlightColor;
		};

		struct CameraPushConstant
		{
			Eigen::Vector4f position;
			Eigen::Matrix4f projView;
		};

	public:
		Render();

		void init();

		void update();
		virtual void updateApp() = 0;

	protected:
		void draw();
		virtual void drawGUI() {};

		void loadAPI();
		void createInstance();
		/**
		 * @brief create surface with WSI system implmented in child class
		 */
		virtual void createSurface() = 0;

		virtual void createAssetApp() = 0;

		void validataInstanceExtensions();
		void validataInstanceLayers();

		void createLogicalDevice();
		void validataDeviceExtensions();
		void getPhysicalDevice();
		void getQueueFamilyIndex();

		void createSwapchain();

		void createVmaAllocator();

		// render pass
		void createDepthStencilImage();
		void createRenderPass();
		void createFrameBuffers();

		void buildPipelineBuilder();

		void createAsset();
		void createCameraPushConstant();
		void createSceneParametersUniformBuffer();

		// build mesh
		void buildRenderable();

		void createDescriptorPool();
		void createPipelineCache();
		void createGraphicsPipeline();
		void createComputePipeline();

		void createCommandPool();
		void allocateCommandBuffer();

		// synchronization
		void createSynchronization();

		void updateCameraBuffer();
		void updateSceneParameters();

		vk::CommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(vk::CommandBuffer cmdBuffer);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		size_t padUniformBufferSize(size_t originalSize, VkPhysicalDeviceProperties gpuProperties);

		std::tuple<Image, vk::UniqueImageView> createTextureImage(uint32_t texWidth, uint32_t texHeight,
			uint32_t texChannels, void* pixels);

	protected:
		std::string appName;
		std::string engineName;
		uint32_t apiVersion;
		uint32_t appVersion;
		uint32_t engineVersion;

		std::vector<const char*> instanceExtensions;
		std::vector<const char*> layers;
		std::vector<const char*> deviceExtensions;

		vk::DynamicLoader dl;
		vk::UniqueInstance instance;
		vk::UniqueDebugUtilsMessengerEXT debugMessenger;
		vk::UniqueSurfaceKHR surface;
		vk::PhysicalDevice physicalDevice;
		vk::UniqueDevice device;
		uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex, computeQueueFamilyIndex;
		vk::Queue graphicsQueue, presentQueue, computeQueue;

		vk::UniqueCommandPool commandPool;
		std::vector<vk::UniqueCommandBuffer> commandBuffer;

		std::unique_ptr<VmaAllocator, void(*)(VmaAllocator*)> vmaAllocator;

		uint32_t width, height;
		vk::Format swapchainImageFormat;
		vk::PresentModeKHR preferPresentMode;
		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::UniqueImageView> swapchainImageViews;

		Image depthStencilImage;
		vk::UniqueImageView depthStencilImageView;
		vk::UniqueRenderPass renderPass, guiRenderPass;
		vk::ClearValue clearValues[2];
		std::vector<vk::UniqueFramebuffer> framebuffers;

		// global asset
		Camera camera;
		std::vector<CameraPushConstant> cameraPushConstants;

		SceneParameters sceneParameters;
		Buffer sceneParameterBuffer;
		vk::DescriptorBufferInfo sceneParameterBufferDescriptorInfo;
		uint32_t sceneParametersUniformOffset;

		vk::UniqueSampler textureSampler;
		Image imageBlank, imageLilac;
		vk::UniqueImageView imageViewBlank, imageViewLilac;

		// renderable
		std::vector<RenderableBase> renderables;
		std::unordered_map<std::string, std::shared_ptr<MeshBase>> loadMeshs;
		std::unordered_map<std::string, Image> loadImages;
		std::unordered_map<std::string, std::shared_ptr<vk::ImageView>> loadImageViews;
		std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
		std::unordered_map<std::string, Image> normalImages;
		std::unordered_map<std::string, std::shared_ptr<vk::ImageView>> normalImageViews;
		std::vector<vk::DescriptorImageInfo> descriptorImageInfosNormal;
		std::unordered_map<std::string, Image> metallicImages;
		std::unordered_map<std::string, std::shared_ptr<vk::ImageView>> metallicImageViews;
		std::vector<vk::DescriptorImageInfo> descriptorImageInfosMetallic;

		uint32_t modelMatrixOffset;
		std::vector<uint8_t> modelMatrix;
		Buffer modelMatrixBuffer;
		vk::DescriptorBufferInfo modelMatrixBufferDescriptorInfo;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		const vk::DeviceSize vertOffsets[1] = { 0 };
		Buffer vertBuff, indexBuffer;

		vk::UniqueDescriptorPool descriptorPool;

		vk::UniquePipelineCache pipelineCache;

		// synchronization
		std::vector<vk::UniqueSemaphore> imageAcquiredSemaphores;
		std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
		std::vector<vk::UniqueSemaphore> guiRenderFinishedSemaphores;
		std::vector<vk::UniqueFence> inFlightFences;
		std::vector<vk::Fence> imagesInFlightFences;
		uint32_t currentFrame = 0;
		uint32_t currBuffIndex = 0;

		std::array<vk::PipelineStageFlags, 1> waitPipelineStageFlags;
		vk::SubmitInfo submitInfo;
		vk::PresentInfoKHR presentInfo;
		bool drawGUIFlag = true;

		std::unordered_map<PipelineType, PipelinePass> pipelinePasss;
		PipelineBuilder pipelineBuilder;
	};
}
