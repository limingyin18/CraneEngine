// CraneEngine.cpp : Defines the entry point for the application.
//

#define VMA_IMPLEMENTATION
#include "Render.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		LOGE("{}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOGW("{}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		LOGI("{}", pCallbackData->pMessage);
		break;
	default:
		LOGD("{}", pCallbackData->pMessage);
		break;
	}

	return VK_FALSE;
}

Render::Render() : vmaAllocator{ nullptr,[](VmaAllocator* vma) {vmaDestroyAllocator(*vma); } }
{
	appName = "Test";
	engineName = "Crane Vision";
	apiVersion = VK_API_VERSION_1_1;
	uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0);
	uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);

	instanceExtensions = {
	VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
	VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#ifndef NDEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif

	};

	layers = {
	#ifndef NDEBUG
		"VK_LAYER_KHRONOS_validation",
	#endif
		"VK_LAYER_KHRONOS_synchronization2"
	};

	deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
		VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,
	};

	// set default clear values
	clearValues[0].color.float32[0] = 0.0f;
	clearValues[0].color.float32[1] = 0.0f;
	clearValues[0].color.float32[2] = 0.0f;
	clearValues[0].color.float32[3] = 0.0f;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;
}

void Render::init()
{
	loadAPI();
	createInstance();
	createSurface();
	createLogicalDevice();

	createVmaAllocator();
	createCommandPool();
	createDescriptorPool();

	createSwapchain();
	createDepthStencilImage();
	createRenderPass();
	createFrameBuffers();

	buildPipelineBuilder();

	allocateCommandBuffer();

	createSynchronization();

	initEngine();
	createAsset();

	LOGI("初始化完成");
}

void Crane::Render::update()
{
	device->waitForFences(inFlightFences[currentFrame].get(), true, UINT64_MAX);
	currBuffIndex = device->acquireNextImageKHR(swapchain.get(), UINT64_MAX, imageAcquiredSemaphores[currentFrame].get());
	device->waitForFences(imagesInFlightFences[currBuffIndex], true, UINT64_MAX);
	imagesInFlightFences[currBuffIndex] = inFlightFences[currentFrame].get();
	device->resetFences(inFlightFences[currentFrame].get());

	updateApp();

	updateCameraBuffer();
	updateSceneParameters();

	updateCullData();

	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(*vmaAllocator, bufferIndirect.bufferMemory, &allocInfo);
	auto bufferIndirectP = static_cast<vk::DrawIndexedIndirectCommand*>(allocInfo.pMappedData);
	for (uint32_t i = 0, firstIndex = 0; i < draws.size(); ++i)
	{
		bufferIndirectP[i].instanceCount = 0;
	}

	computeQueue.submit(1, &submitInfoCompute, vk::Fence{});
	computeQueue.waitIdle();

	draw();

	//present
	presentInfo.pImageIndices = &currBuffIndex;
	presentQueue.presentKHR(presentInfo);
	currentFrame = (currentFrame + 1) % swapchainImages.size();
}

void Crane::Render::draw()
{
	commandBuffer[currBuffIndex]->reset(vk::CommandBufferResetFlagBits{});
	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBuffer[currBuffIndex]->begin(commandBufferBeginInfo);

	vk::RenderPassBeginInfo rpBeginInfo{
		.renderPass = renderPass.get(),
		.framebuffer = framebuffers[currBuffIndex].get(),
		.renderArea = {.offset = {.x = 0, .y = 0},
						.extent = {.width = width, .height = height}},
		.clearValueCount = 2,
		.pClearValues = clearValues };
	commandBuffer[currBuffIndex]->beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);

	// begin
	{
		vk::Pipeline pipelineLast = nullptr;

		commandBuffer[currBuffIndex]->bindVertexBuffers(uint32_t(0), 1, (vk::Buffer*)&vertBuff.buffer, vertOffsets);
		commandBuffer[currBuffIndex]->bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
		for (uint32_t i = 0; i < draws.size(); ++i)
		{
			IndirectBatch& draw = draws[i];
			vk::Pipeline pipelineNew = draw.renderable->material->pipelinePass->pipeline.get();
			vk::PipelineLayout pipelineLayout = draw.renderable->material->pipelinePass->pipelineLayout.get();
			vk::DescriptorSet* descriptorSetP = draw.renderable->material->descriptorSets.data();
			uint32_t descriptorSetCount = draw.renderable->material->descriptorSets.size();
			MeshBase *meshNew = draw.renderable->mesh;
			if (pipelineNew != pipelineLast)
			{
				commandBuffer[currBuffIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineNew);

				commandBuffer[currBuffIndex]->pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
					0, sizeof(cameraPushConstants[currBuffIndex]), &cameraPushConstants[currBuffIndex]);

				pipelineLast = pipelineNew;
			}

			vector<uint32_t> offsets{ sceneParametersUniformOffset * currBuffIndex };
			commandBuffer[currBuffIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSetCount, descriptorSetP, offsets.size(), offsets.data());
			VkDeviceSize offsetIndirect = i * sizeof(vk::DrawIndexedIndirectCommand);
			commandBuffer[currBuffIndex]->drawIndexedIndirect(bufferIndirect.buffer, offsetIndirect, 1, sizeof(vk::DrawIndexedIndirectCommand));
		}
	}
	// end

	commandBuffer[currBuffIndex]->endRenderPass();
	commandBuffer[currBuffIndex]->end();

	submitInfo.pCommandBuffers = &commandBuffer[currBuffIndex].get();
	submitInfo.pWaitSemaphores = &imageAcquiredSemaphores[currentFrame].get();
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame].get();

	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame].get();

	if (drawGUIFlag) // drawGUIFlag
	{
		graphicsQueue.submit(submitInfo);
		drawGUI();
	}
	else
	{
		graphicsQueue.submit(submitInfo, inFlightFences[currentFrame].get());
	}
}

void Crane::Render::loadAPI()
{
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
	apiVersion = vk::enumerateInstanceVersion();
	LOGI("支持的 Vulkan 版本 {}.{}.{}", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));
}

void Render::createInstance()
{
	LOGI("创建 vulkan instance");

	validataInstanceExtensions();
	validataInstanceLayers();

	// initialize the vk::ApplicationInfo structure
	vk::ApplicationInfo applicationInfo{ .pApplicationName = appName.c_str(),
										.applicationVersion = appVersion,
										.pEngineName = engineName.c_str(),
										.engineVersion = engineVersion,
										.apiVersion = apiVersion };

	// initialize the vk::InstanceCreateInfo
	vk::InstanceCreateInfo instanceCreateInfo{ .pApplicationInfo = &applicationInfo,
											  .enabledLayerCount = static_cast<uint32_t>(layers.size()),
											  .ppEnabledLayerNames = layers.data(),
											  .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
											  .ppEnabledExtensionNames = instanceExtensions.data() };

	instance = vk::createInstanceUnique(instanceCreateInfo, nullptr);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

#ifndef NDEBUG
	vk::DebugUtilsMessengerCreateInfoEXT createInfoDebug{ .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
																			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
																			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
													   .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
																	vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
																	vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
													   .pfnUserCallback = debugCallback };
	debugMessenger = instance->createDebugUtilsMessengerEXTUnique(createInfoDebug);
#endif
}

void Render::validataInstanceExtensions()
{
	std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

	std::set<std::string> requiredExtensions(instanceExtensions.cbegin(), instanceExtensions.cend());
	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);
	if (!requiredExtensions.empty())
	{
		LOGE("extension not availabe:");
		for (const auto& extension : requiredExtensions)
			LOGE("\t{}", extension);
		throw runtime_error("instance extension required not availiable");
	}
	else
	{
		LOGI("enable intance extension:");
		for (const auto& extension : instanceExtensions)
			LOGI("\t{}", extension);
	}
}

void Render::validataInstanceLayers()
{
	std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	std::set<std::string> requiredLayers(layers.cbegin(), layers.cend());
	for (const auto& layer : availableLayers)
		requiredLayers.erase(layer.layerName);
	if (!requiredLayers.empty())
	{
		LOGE("layers not availabe:");
		for (const auto& layer : requiredLayers)
			LOGE("\t{}", layer);
		throw runtime_error("insance layer required not availiable");
	}
	else
	{
		LOGI("enable layer:");
		for (const auto& layer : layers)
			LOGI("\t{}", layer);
	}
}

void Render::createLogicalDevice()
{
	LOGI("创建逻辑设备");

	getPhysicalDevice();

	getQueueFamilyIndex();

	validataDeviceExtensions();

	vk::PhysicalDeviceFeatures features{ .multiDrawIndirect = true, .samplerAnisotropy = true };
	auto supportedFeatures = physicalDevice.getFeatures();
	auto supportedFeatures2 = physicalDevice.getFeatures2();
	if (!supportedFeatures.multiDrawIndirect)
	{
		LOGE("feature not availabe:");
		LOGE("\tmultuDrawIndirect");
		throw runtime_error("feature required not availiable");
	}
	if (!supportedFeatures.samplerAnisotropy)
	{
		LOGE("feature not availabe:");
		LOGE("\tsamplerAnisotropy");
		throw runtime_error("feature required not availiable");
	}

	LOGI("启用特性:");
	LOGI("\tmultiDrawIndirect");
	LOGI("\tsamplerAnisotropy");

	// 指定队列信息

	float queuePriorities[1] = { 0.f };
	vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.push_back({ .queueFamilyIndex = graphicsQueueFamilyIndex,
													.queueCount = 1,
													.pQueuePriorities = queuePriorities });

	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		queueCreateInfos.push_back({ .queueFamilyIndex = presentQueueFamilyIndex,
														.queueCount = 1,
														.pQueuePriorities = queuePriorities });
	}

	if (computeQueueFamilyIndex != graphicsQueueFamilyIndex)
	{
		queueCreateInfos.push_back({ .queueFamilyIndex = computeQueueFamilyIndex,
														.queueCount = 1,
														.pQueuePriorities = queuePriorities });
	}

	// 指定设备创建信息

	vk::DeviceCreateInfo ci{ .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
	.pQueueCreateInfos = queueCreateInfos.data(),
	.enabledLayerCount = static_cast<uint32_t>(layers.size()),
	.ppEnabledLayerNames = layers.data(),
	.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
	.ppEnabledExtensionNames = deviceExtensions.data(),
	.pEnabledFeatures = &features };

	device = physicalDevice.createDeviceUnique(ci);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

	graphicsQueue = device->getQueue(graphicsQueueFamilyIndex, 0);
	presentQueue = device->getQueue(presentQueueFamilyIndex, 0);
	computeQueue = device->getQueue(computeQueueFamilyIndex, 0);
}

void Render::validataDeviceExtensions()
{
	std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	if (!requiredExtensions.empty())
	{
		LOGE("device extension not availabe:");
		for (const auto& extension : requiredExtensions)
			LOGE("\t{}", extension);
		throw runtime_error("device extension required not availiable");
	}
	else
	{
		LOGI("enable device extension:");
		for (const auto& ext : deviceExtensions)
			LOGI("\t{}", ext);
	}
}

void Render::getPhysicalDevice()
{
	vector<vk::PhysicalDevice> gpus = instance->enumeratePhysicalDevices();
	if (gpus.empty()) throw runtime_error("can't find GPU");

	for (uint32_t i = 0; i < gpus.size(); ++i)
	{
		if (gpus[i].getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			physicalDevice = gpus[i];
	}
	if (physicalDevice.operator VkPhysicalDevice() == nullptr)
		throw runtime_error("no discrete gpu");

	physicalDeviceProperties = physicalDevice.getProperties();
}

void Render::getQueueFamilyIndex()
{
	vector<vk::QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();

	// find graphics and present queue family index
	graphicsQueueFamilyIndex = UINT32_MAX;
	presentQueueFamilyIndex = UINT32_MAX;
	for (unsigned i = 0; i < queueFamilyProps.size(); ++i)
	{
		if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
			graphicsQueueFamilyIndex = i;

		VkBool32 supportPresent;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface.get(), &supportPresent);
		if (supportPresent)
			presentQueueFamilyIndex = i;
	}

	// find queue family support graphics and present both
	for (unsigned i = 0; i < queueFamilyProps.size(); ++i)
	{
		VkBool32 supportPresent;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface.get(), &supportPresent);
		if (supportPresent)
		{
			if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				graphicsQueueFamilyIndex = i;
				presentQueueFamilyIndex = i;
			}
		}
	}

	if (graphicsQueueFamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX)
		throw runtime_error("failed to find graphics queue family");

	// find compute queue family
	computeQueueFamilyIndex = UINT32_MAX;
	for (unsigned i = 0; i < queueFamilyProps.size(); ++i)
	{
		if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eCompute)
			computeQueueFamilyIndex = i;
	}

	// find compute queue family but different with graphics
	computeQueueFamilyIndex = UINT32_MAX;
	for (unsigned i = 0; i < queueFamilyProps.size(); ++i)
	{
		if ((queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eCompute) && i != graphicsQueueFamilyIndex)
			computeQueueFamilyIndex = i;
	}

	if (computeQueueFamilyIndex == UINT32_MAX)
		throw runtime_error("failed to find compute queue family");
}

void Render::createSwapchain()
{
	LOGI("创建交换链");

	vk::SurfaceCapabilitiesKHR surfCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());

	width = surfCapabilities.currentExtent.width;
	height = surfCapabilities.currentExtent.height;

	vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface.get());

	swapchainImageFormat = formats[0].format;

	// 选择呈现模式，prefer > Mailbox > FIFORelaxed > FIFO

	vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface.get());
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	if (any_of(presentModes.cbegin(), presentModes.cend(), [this](vk::PresentModeKHR p)
		{ return p == preferPresentMode; }))
		presentMode = preferPresentMode;
	else if (any_of(presentModes.cbegin(), presentModes.cend(), [](vk::PresentModeKHR p)
		{ return p == vk::PresentModeKHR::eMailbox; }))
		presentMode = vk::PresentModeKHR::eMailbox;
	else if (any_of(presentModes.cbegin(), presentModes.cend(), [](vk::PresentModeKHR p)
		{ return p == vk::PresentModeKHR::eFifoRelaxed; }))
		presentMode = vk::PresentModeKHR::eFifoRelaxed;

	// create swapchain

	// 指定交换创建信息
	vk::SwapchainCreateInfoKHR swapchainCreateInfo{ .surface = surface.get(),
													.minImageCount = surfCapabilities.minImageCount + 1,
													.imageFormat = swapchainImageFormat,
													.imageColorSpace = formats[0].colorSpace,
													.imageExtent = {.width = width, .height = height},
													.imageArrayLayers = 1,
													.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
													.imageSharingMode = vk::SharingMode::eExclusive,
													.preTransform = surfCapabilities.currentTransform,
													.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
													.presentMode = presentMode,
													.clipped = true,
													.oldSwapchain = nullptr };
	unsigned queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between the
		// queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);

	// get swapchain images
	swapchainImages = device->getSwapchainImagesKHR(swapchain.get());

	// create swapchain image views
	swapchainImageViews.resize(swapchainImages.size());
	for (uint32_t i = 0; i < swapchainImageViews.size(); i++)
	{
		vk::ImageViewCreateInfo viewInfo{
		.image = swapchainImages[i],
		.viewType = vk::ImageViewType::e2D,
		.format = swapchainCreateInfo.imageFormat,
		.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1} };

		swapchainImageViews[i] = device->createImageViewUnique(viewInfo);
	}
}

void Render::createVmaAllocator()
{
	LOGI("创建缓冲分配器 vmaAllocator");

	auto vkGetImageMemoryRequirements2KHR =
		PFN_vkGetImageMemoryRequirements2KHR(vkGetDeviceProcAddr(device.get(), "vkGetImageMemoryRequirements2KHR"));

	auto vkGetBufferMemoryRequirements2KHR =
		PFN_vkGetBufferMemoryRequirements2KHR(vkGetDeviceProcAddr(device.get(), "vkGetBufferMemoryRequirements2KHR"));

	VmaVulkanFunctions vmaVulkanFunc{};
	vmaVulkanFunc.vkAllocateMemory = vkAllocateMemory;
	vmaVulkanFunc.vkBindBufferMemory = vkBindBufferMemory;
	vmaVulkanFunc.vkBindImageMemory = vkBindImageMemory;
	vmaVulkanFunc.vkCreateBuffer = vkCreateBuffer;
	vmaVulkanFunc.vkCreateImage = vkCreateImage;
	vmaVulkanFunc.vkDestroyBuffer = vkDestroyBuffer;
	vmaVulkanFunc.vkDestroyImage = vkDestroyImage;
	vmaVulkanFunc.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vmaVulkanFunc.vkFreeMemory = vkFreeMemory;
	vmaVulkanFunc.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vmaVulkanFunc.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vmaVulkanFunc.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaVulkanFunc.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaVulkanFunc.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vmaVulkanFunc.vkMapMemory = vkMapMemory;
	vmaVulkanFunc.vkUnmapMemory = vkUnmapMemory;
	vmaVulkanFunc.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vmaVulkanFunc.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vmaVulkanFunc.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device.get();
	allocatorInfo.instance = instance.get();
	allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	allocatorInfo.pVulkanFunctions = &vmaVulkanFunc;

	VmaAllocator* vma = new VmaAllocator;
	if (vmaCreateAllocator(&allocatorInfo, vma) != VK_SUCCESS)
		throw runtime_error("failed to create vma vmaAllocator");
	vmaAllocator.reset(vma);
}

void Render::createDepthStencilImage()
{
	depthStencilImage.create(*vmaAllocator, width, height, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	vk::ImageViewCreateInfo viewCreateInfo = {
	.image = vk::Image(depthStencilImage.image),
	.viewType = vk::ImageViewType::e2D,
	.format = vk::Format::eD24UnormS8Uint,
	.components = {.r = vk::ComponentSwizzle::eR,
					.g = vk::ComponentSwizzle::eG,
					.b = vk::ComponentSwizzle::eB,
					.a = vk::ComponentSwizzle::eA},
	.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1},
	};
	depthStencilImageView = device->createImageViewUnique(viewCreateInfo);
}

void Render::createRenderPass()
{
	vk::AttachmentDescription attachments[2];
	// color
	attachments[0].format = swapchainImageFormat;
	attachments[0].samples = vk::SampleCountFlagBits::e1;;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	// depth
	attachments[1].format = vk::Format::eD24UnormS8Uint;
	attachments[1].samples = vk::SampleCountFlagBits::e1;;
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference color_reference = {
		.attachment = 0,
		.layout = vk::ImageLayout::eColorAttachmentOptimal };

	vk::AttachmentReference depth_reference = {
		.attachment = 1,
		.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal };

	vk::SubpassDescription subpass = {
		.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_reference,
		.pDepthStencilAttachment = &depth_reference };

	// Subpass dependency to wait for wsi image acquired semaphore before starting layout transition
	vk::SubpassDependency subpassDenpendency = {
	.srcSubpass = VK_SUBPASS_EXTERNAL,
	.dstSubpass = 0,
	.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
	.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
	.srcAccessMask = vk::AccessFlagBits::eNoneKHR,
	.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite };

	vk::RenderPassCreateInfo renderPassCreateInfo{ .attachmentCount = 2,
												.pAttachments = attachments,
												.subpassCount = 1,
												.pSubpasses = &subpass,
												.dependencyCount = 1,
												.pDependencies = &subpassDenpendency };

	renderPass = device->createRenderPassUnique(renderPassCreateInfo);

	// create gui renderpass
	attachments[0].loadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].loadOp = vk::AttachmentLoadOp::eDontCare;
	guiRenderPass = device->createRenderPassUnique(renderPassCreateInfo);
}

void Render::createFrameBuffers()
{
	LOGI("create framebuffer");

	vk::ImageView attachments[2];
	attachments[1] = depthStencilImageView.get();

	vk::FramebufferCreateInfo framebufferCreateInfo = {
	.renderPass = renderPass.get(),
	.attachmentCount = 2,
	.pAttachments = attachments,
	.width = width,
	.height = height,
	.layers = 1 };

	framebuffers.resize(swapchainImageViews.size());
	for (unsigned i = 0; i < swapchainImageViews.size(); ++i)
	{
		attachments[0] = swapchainImageViews[i].get();
		framebuffers[i] = device->createFramebufferUnique(framebufferCreateInfo);
	}
}

void Render::buildPipelineBuilder()
{
	pipelineBuilder.device = device.get();

	pipelineBuilder.viewport = vk::Viewport{
		.y = (float)height,
		.width = (float)width,
		.height = -(float)height,
		.minDepth = 0.f,
		.maxDepth = 1.f };
	pipelineBuilder.scissor = vk::Rect2D{
		.extent = {.width = width, .height = height} };
	pipelineBuilder.vp = vk::PipelineViewportStateCreateInfo{
		.viewportCount = 1,
		.pViewports = &pipelineBuilder.viewport,
		.scissorCount = 1,
		.pScissors = &pipelineBuilder.scissor };

	pipelineBuilder.pipelineCache = pipelineCache.get();
}

void Render::createAsset()
{
	createCameraPushConstant();
	createSceneParametersUniformBuffer();

	LOGI("创建sampler")
		vk::SamplerCreateInfo samplerInfo{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = physicalDevice.getProperties().limits.maxSamplerAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = vk::CompareOp::eAlways,
		.borderColor = vk::BorderColor::eIntOpaqueBlack,
		.unnormalizedCoordinates = VK_FALSE };
	textureSampler = device->createSamplerUnique(samplerInfo);

	LOGI("创建颜色")
		vector<uint8_t> blankPiexls{ 255, 255, 255, 255 };
	vector<uint8_t> lilacPiexls{ 179, 153, 255, 255 };
	std::tie(imageBlank, imageViewBlank) = createTextureImage(1, 1, 4, blankPiexls.data());
	std::tie(imageLilac, imageViewLilac) = createTextureImage(1, 1, 4, lilacPiexls.data());

	descriptorImageInfoBlank.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	descriptorImageInfoBlank.imageView = imageViewBlank.get();
	descriptorImageInfoBlank.sampler = textureSampler.get();

	descriptorImageInfoLilac.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	descriptorImageInfoLilac.imageView = imageViewLilac.get();
	descriptorImageInfoLilac.sampler = textureSampler.get();

	createAssetApp();

	if (!renderables.empty()) buildRenderable();
}

void Render::createCameraPushConstant()
{
	LOGI("创建相机 push constant");

	cameraPushConstants.resize(swapchainImages.size());
	for (auto& c : cameraPushConstants)
	{
		Eigen::Vector4f position;
		position.head(3) = camera.getCameraPos();
		position[3] = 1.0f;
		c.position = position;
		c.projView = camera.projection * camera.view;
	}
}

void Render::createSceneParametersUniformBuffer()
{
	LOGI("创建场景参数缓冲");

	sceneParameters.ambientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	sceneParameters.fogColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	sceneParameters.fogDistances = { 0.2f, 0.2f, 0.2f, 1.0f };
	sceneParameters.sunlightColor = { 0.9f, 0.9f, 0.9f, 1.0f };
	Eigen::Vector3f sunDirection = {0.f, -1.f, -0.5f };
	sunDirection.normalize();
	sceneParameters.sunlightDirection = { sunDirection.x(), sunDirection.y(), sunDirection.z(), 1.0f };

	vector<SceneParameters> sceneParametersData(swapchainImages.size(), sceneParameters);
	sceneParametersUniformOffset = padUniformBufferSize(sizeof(SceneParameters), physicalDevice.getProperties());

	const size_t sceneParameterBufferSize = swapchainImages.size() * sceneParametersUniformOffset;
	sceneParameterBuffer.create(*vmaAllocator, sceneParameterBufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	sceneParameterBuffer.update(sceneParametersData.data());

	sceneParameterBufferDescriptorInfo.buffer = sceneParameterBuffer.buffer;
	sceneParameterBufferDescriptorInfo.offset = 0;
	sceneParameterBufferDescriptorInfo.range = sizeof(SceneParameters);
}

void Render::buildRenderable()
{
	LOGI("构建可渲染对象");

	compactDraws();

	modelMatrixOffset = padUniformBufferSize(sizeof(Eigen::Matrix4f), physicalDevice.getProperties());
	const size_t modelMatrixBufferSize = renderables.size() * modelMatrixOffset;
	modelMatrix.resize(modelMatrixBufferSize);
	auto modelMatrixPtr = modelMatrix.data();

	uint32_t offset = 0;
	for (auto& d : draws)
	{
		auto& v = renderables[d.first];
		for (size_t i = 0; i < v.mesh->data.size(); i++)
			vertices.push_back(v.mesh->data[i]);
		for (size_t i = 0; i < v.mesh->indices.size(); i++)
			indices.push_back(offset + v.mesh->indices[i]);
		offset += v.mesh->data.size();

		for (uint32_t i = 0; i < d.count; ++i)
		{
			*(reinterpret_cast<Eigen::Matrix4f*>(modelMatrixPtr)) = *renderables[d.first + i].transformMatrix;
			modelMatrixPtr = modelMatrixPtr + modelMatrixOffset;
		}
	}

	modelMatrixBuffer.create(*vmaAllocator, modelMatrixBufferSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	modelMatrixBuffer.update(modelMatrix.data());

	modelMatrixBufferDescriptorInfo.buffer = modelMatrixBuffer.buffer;
	modelMatrixBufferDescriptorInfo.offset = 0;
	modelMatrixBufferDescriptorInfo.range = modelMatrixBufferSize;


	for_each(renderables.begin(), renderables.end(), [](const auto&v) {
		v.material->update();
		});

	LOGI("创建顶点缓冲");
	{
		vk::CommandBuffer cmdBuffVert = beginSingleTimeCommands();
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		Buffer stagingBuffer = Buffer::createStagingBuffer(vertices.data(), bufferSize,
			*vmaAllocator);

		vertBuff.create(*vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(cmdBuffVert, stagingBuffer.buffer, vertBuff.buffer, 1, &copyRegion);
		endSingleTimeCommands(cmdBuffVert);
	}

	LOGI("创建索引缓冲");
	{
		vk::CommandBuffer cmdBuffIndex = beginSingleTimeCommands();
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		Buffer stagingBuffer = Buffer::createStagingBuffer(indices.data(), bufferSize,
			*vmaAllocator);

		indexBuffer.create(*vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(cmdBuffIndex, stagingBuffer.buffer, indexBuffer.buffer, 1, &copyRegion);
		endSingleTimeCommands(cmdBuffIndex);
	}

	LOGI("创建间接绘制缓冲");
	{
		bufferIndirect.create(*vmaAllocator, draws.size() * sizeof(vk::DrawIndexedIndirectCommand),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(*vmaAllocator, bufferIndirect.bufferMemory, &allocInfo);
		auto bufferIndirectP = static_cast<vk::DrawIndexedIndirectCommand*>(allocInfo.pMappedData);
		for (uint32_t i = 0, firstIndex = 0; i < draws.size(); ++i)
		{
			bufferIndirectP[i].firstIndex = firstIndex;
			bufferIndirectP[i].firstInstance = draws[i].first;
			bufferIndirectP[i].indexCount = renderables[draws[i].first].mesh->indices.size();
			bufferIndirectP[i].instanceCount = 0;
			bufferIndirectP[i].vertexOffset = 0;

			firstIndex += bufferIndirectP[i].indexCount;
		}
		descriptorBufferInfoIndirect.buffer = bufferIndirect.buffer;
		descriptorBufferInfoIndirect.range = bufferIndirect.size;
	}

	device->updateDescriptorSets(materialCull.writeDescriptorSets.size(),
		materialCull.writeDescriptorSets.data(), 0, nullptr);

	// record command
	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBuffersCompute[0]->begin(commandBufferBeginInfo);

	commandBuffersCompute[0]->bindPipeline(vk::PipelineBindPoint::eCompute, materialCull.pipelinePass->pipeline.get());
	commandBuffersCompute[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute,
		materialCull.pipelinePass->pipelineLayout.get(), 0, materialCull.descriptorSets.size(), materialCull.descriptorSets.data(), 0, nullptr);

	commandBuffersCompute[0]->dispatch(renderables.size() / 1024+1, 1, 1);

	commandBuffersCompute[0]->end();
}

void Render::createDescriptorPool()
{
	LOGI("创建描述符池");
	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
	{ {.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000 } };

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{ .maxSets = 1000, .poolSizeCount = (uint32_t)descriptorPoolSizes.size(), .pPoolSizes = descriptorPoolSizes.data() };

	descriptorPool = device->createDescriptorPoolUnique(descriptorPoolCreateInfo);
}

void Render::createCommandPool()
{
	LOGI("创建命令池");

	vk::CommandPoolCreateInfo commandPoolCreateInfo{ .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
	.queueFamilyIndex = graphicsQueueFamilyIndex };
	commandPool = device->createCommandPoolUnique(commandPoolCreateInfo);

	vk::CommandPoolCreateInfo commandPoolCreateInfoCompute{ .queueFamilyIndex = computeQueueFamilyIndex };
	commandPoolCompute = device->createCommandPoolUnique(commandPoolCreateInfoCompute);
}

void Render::allocateCommandBuffer()
{
	LOGI("分配命令缓冲");

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{ .commandPool = commandPool.get(),
		.level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = static_cast<uint32_t>(swapchainImages.size()) };
	commandBuffer = device->allocateCommandBuffersUnique(commandBufferAllocateInfo);

	vk::CommandBufferAllocateInfo commandBufferAllocateInfoCompute{ .commandPool = commandPoolCompute.get(),
		.level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
	commandBuffersCompute = device->allocateCommandBuffersUnique(commandBufferAllocateInfoCompute);
}

void Crane::Render::createSynchronization()
{
	LOGI("创建同步");

	// create semaphores
	imageAcquiredSemaphores.resize(swapchainImages.size());
	renderFinishedSemaphores.resize(swapchainImages.size());
	guiRenderFinishedSemaphores.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); ++i)
	{
		// create image acquired semaphore
		vk::SemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo{};
		imageAcquiredSemaphores[i] = device->createSemaphoreUnique(imageAcquiredSemaphoreCreateInfo);

		// create render finished semaphore
		vk::SemaphoreCreateInfo renderFinishedSemaphoreCreateInfo{};
		renderFinishedSemaphores[i] = device->createSemaphoreUnique(renderFinishedSemaphoreCreateInfo);

		// create gui render finished semaphore
		vk::SemaphoreCreateInfo guiRenderFinishedSemaphoreCreateInfo{};
		guiRenderFinishedSemaphores[i] = device->createSemaphoreUnique(guiRenderFinishedSemaphoreCreateInfo);
	}

	// create fence
	inFlightFences.resize(swapchainImages.size());
	imagesInFlightFences.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); ++i)
	{
		vk::FenceCreateInfo inFlightFenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
		inFlightFences[i] = device->createFenceUnique(inFlightFenceInfo);
		imagesInFlightFences[i] = inFlightFences[i].get();
	}

	// set submit info
	waitPipelineStageFlags = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitPipelineStageFlags.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.signalSemaphoreCount = 1;

	// present info
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.get();
	presentInfo.waitSemaphoreCount = 1;

	// set cull submit info
	submitInfoCompute.commandBufferCount = commandBuffersCompute.size();
	submitInfoCompute.pCommandBuffers = &commandBuffersCompute[0].get();
}

void Crane::Render::updateCameraBuffer()
{
	Eigen::Vector4f position;
	position.head(3) = camera.getCameraPos();
	position[3] = 1.0f;
	cameraPushConstants[currBuffIndex].position = position;
	cameraPushConstants[currBuffIndex].projView = camera.projection * camera.view;
}

void Crane::Render::updateSceneParameters()
{
	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(*vmaAllocator, sceneParameterBuffer.bufferMemory, &allocInfo);
	memcpy(static_cast<uint8_t*>(allocInfo.pMappedData) +
		(sceneParametersUniformOffset * currBuffIndex),
		&sceneParameters, sizeof(SceneParameters));
}

vk::CommandBuffer Render::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{ .commandPool = commandPool.get(),
		.level = vk::CommandBufferLevel::ePrimary,  .commandBufferCount = 1 };

	auto singleBuffer = device->allocateCommandBuffers(commandBufferAllocateInfo);

	vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	singleBuffer[0].begin(beginInfo);

	return singleBuffer[0];
}

void Render::endSingleTimeCommands(vk::CommandBuffer cmdBuffer)
{
	cmdBuffer.end();

	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuffer };

	graphicsQueue.submit(1, &submitInfo, vk::Fence{});
	graphicsQueue.waitIdle();

	device->freeCommandBuffers(commandPool.get(), 1, &cmdBuffer);
}

void Render::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

size_t Render::padUniformBufferSize(size_t originalSize, VkPhysicalDeviceProperties gpuProperties)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}

uint32_t Crane::Render::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

std::tuple<Image, vk::UniqueImageView> Render::createTextureImage(uint32_t texWidth, uint32_t texHeight, uint32_t texChannels, void* pixels)
{
	Image image;
	image.create(*vmaAllocator, texWidth, texHeight, texChannels,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);
	vk::CommandBuffer cmdBuff = beginSingleTimeCommands();
	Buffer stagingBuffer = Buffer::createStagingBuffer(pixels, image.imageSize, *vmaAllocator);
	image.updateBuffer(stagingBuffer.buffer, cmdBuff);
	endSingleTimeCommands(cmdBuff);

	vk::ImageViewCreateInfo imageViewCreateInfo{ .image = image.image,
												.viewType = vk::ImageViewType::e2D,
												.format = vk::Format::eR8G8B8A8Unorm,
												.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
																	 .baseMipLevel = 0,
																	 .levelCount = 1,
																	 .baseArrayLayer = 0,
																	 .layerCount = 1} };

	vk::UniqueImageView imageView = device->createImageViewUnique(imageViewCreateInfo);
	return { std::move(image), std::move(imageView) };
}
