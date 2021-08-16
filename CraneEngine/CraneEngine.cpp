// CraneVision.cpp : Defines the entry point for the application.
//

#include "CraneEngine.hpp"

using namespace std;
using namespace CraneVision;

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

VisionEngine::VisionEngine()
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
		"VK_LAYER_KHRONOS_validation"
	#endif
	};

	deviceExtensions = {
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
	};
}

void VisionEngine::init()
{
	// load api
	vk::DynamicLoader dl;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	createInstance();
	createLogicalDevice();

	createVmaAllocator();
	createAsset();

	createShader();

	createDescriptorPool();
	createDescriptorLayout();
	createDescriptorSet();
	createPipelineLayout();
	createPipelineCache();
	createPipeline();

	createCommandPool();
	allocateCommandBuffer();
	buildCommandBuffer();
}

void VisionEngine::train()
{
	LOGI("train");

	device->waitIdle();
	computeQueue.submit(1, &submitInfo, vk::Fence{});
	device->waitIdle();
}

void CraneVision::VisionEngine::createInstance()
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

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = appName.c_str();
	appInfo.applicationVersion = appVersion;
	appInfo.pEngineName = engineName.c_str();
	appInfo.engineVersion = engineVersion;
	appInfo.apiVersion = apiVersion;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = instanceExtensions.size();
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();
	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();

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

void CraneVision::VisionEngine::validataInstanceExtensions()
{
	uint32_t availableExtensionsCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, availableExtensions.data());

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

void CraneVision::VisionEngine::validataInstanceLayers()
{
	uint32_t availableLayersCount;
	vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(availableLayersCount);
	vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

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

void CraneVision::VisionEngine::createLogicalDevice()
{
	LOGI("创建逻辑设备");

	getPhysicalDevice();

	getQueueFamilyIndex();

	validataDeviceExtensions();

	// 指定队列信息
	float queuePriorities[1] = { 0.f };
	vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.push_back({ .queueFamilyIndex = computeQueueFamilyIndex,
													.queueCount = 1,
													.pQueuePriorities = queuePriorities });

	// 指定设备创建信息
	vk::DeviceCreateInfo ci{ .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
	.pQueueCreateInfos = queueCreateInfos.data(),
	.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
	.ppEnabledExtensionNames = deviceExtensions.data() };

	device = physicalDevice.createDeviceUnique(ci);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

	computeQueue = device->getQueue(computeQueueFamilyIndex, 0);
}

void CraneVision::VisionEngine::validataDeviceExtensions()
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

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

void CraneVision::VisionEngine::getPhysicalDevice()
{
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(instance.get(), &gpuCount, nullptr);
	if (gpuCount == 0)
		throw runtime_error("can't find GPU");
	vector<VkPhysicalDevice> gpus(gpuCount);
	vkEnumeratePhysicalDevices(instance.get(), &gpuCount, gpus.data());
	physicalDevice = gpus[0];
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(gpus[0], &properties);
	gpuProperties = properties;
	for (uint32_t i = 0; i < gpuCount; ++i)
	{
		vkGetPhysicalDeviceProperties(gpus[i], &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDevice = gpus[i];
			gpuProperties = properties;
		}
	}
}

void CraneVision::VisionEngine::getQueueFamilyIndex()
{
	// get queue family properties
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
	if (queueFamilyCount == 0)
		throw runtime_error("no queue family found");
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
		queueFamilyProps.data());

	// find compute queue family
	computeQueueFamilyIndex = UINT32_MAX;
	for (unsigned i = 0; i < queueFamilyCount; ++i)
	{
		if ((queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
			(queueFamilyProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
			(queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			computeQueueFamilyIndex = i;
	}

	if (computeQueueFamilyIndex == UINT32_MAX)
		throw runtime_error("failed to find compute queue family");
}

void CraneVision::VisionEngine::createVmaAllocator()
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

	if (vmaCreateAllocator(&allocatorInfo, &vmaAllocator) != VK_SUCCESS)
		throw runtime_error("failed to create vma vmaAllocator");
	deletors.push([=]()
		{ vmaDestroyAllocator(vmaAllocator); });
}

void CraneVision::VisionEngine::createAsset()
{
	uint32_t bufferSize = 1280 * 960 * 4;
	inputBuffer.create(vmaAllocator, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		{ computeQueueFamilyIndex });

	outputBuffer.create(vmaAllocator, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		{ computeQueueFamilyIndex });

	inputBufferInfo.buffer = inputBuffer.buffer;
	inputBufferInfo.offset = 0;
	inputBufferInfo.range = inputBuffer.size;

	outputBufferInfo.buffer = outputBuffer.buffer;
	outputBufferInfo.offset = 0;
	outputBufferInfo.range = outputBuffer.size;

	bindings.emplace_back();
	bindings[0].binding = 0;
	bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;

	bindings.emplace_back();
	bindings[1].binding = 1;
	bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;

	writeDescriptorSets.emplace_back();
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].pBufferInfo = &inputBufferInfo;

	writeDescriptorSets.emplace_back();
	writeDescriptorSets[1].dstBinding = 1;
	writeDescriptorSets[1].dstArrayElement = 0;
	writeDescriptorSets[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].pBufferInfo = &outputBufferInfo;

}

vk::UniqueShaderModule CraneVision::VisionEngine::createShaderModule(const std::vector<char>& code)
{
	LOGI("创建着色器");

	vk::ShaderModuleCreateInfo shaderModuleCreateInfo{ .codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data()) };

	return device->createShaderModuleUnique(shaderModuleCreateInfo);
}

void CraneVision::VisionEngine::createShader()
{
	auto shaderCode = Loader::readFile("test.comp.spv");
	computeShader = createShaderModule(shaderCode);

	// Set some shader parameters via specialization constants
	struct SpecializationData
	{
		uint32_t workgroup_size_x;
		uint32_t workgroup_size_y;
		uint32_t row;
		uint32_t col;
	} specializationData;
	specializationData.workgroup_size_x = 16;
	specializationData.workgroup_size_y = 16;
	specializationData.row = 960;
	specializationData.col = 1280;

	specializationMapEntries.emplace_back();
	specializationMapEntries[0].constantID = 0;
	specializationMapEntries[0].offset = offsetof(SpecializationData, workgroup_size_x);
	specializationMapEntries[0].size = sizeof(uint32_t);
	specializationMapEntries.emplace_back();
	specializationMapEntries[1].constantID = 1;
	specializationMapEntries[1].offset = offsetof(SpecializationData, workgroup_size_y);
	specializationMapEntries[1].size = sizeof(uint32_t);
	specializationMapEntries.emplace_back();
	specializationMapEntries[2].constantID = 2;
	specializationMapEntries[2].offset = offsetof(SpecializationData, row);
	specializationMapEntries[2].size = sizeof(uint32_t);
	specializationMapEntries.emplace_back();
	specializationMapEntries[3].constantID = 3;
	specializationMapEntries[3].offset = offsetof(SpecializationData, col);
	specializationMapEntries[3].size = sizeof(uint32_t);

	specializationInfo = { .mapEntryCount = (uint32_t)specializationMapEntries.size(),
		.pMapEntries = specializationMapEntries.data(), .dataSize = sizeof(SpecializationData),
		.pData = &specializationData };
}

void CraneVision::VisionEngine::createDescriptorLayout()
{
	LOGI("创建描述符布局");

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ .bindingCount = (uint32_t)bindings.size(),
		.pBindings = bindings.data() };

	descriptorSetLayout = device->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
}

void CraneVision::VisionEngine::createDescriptorPool()
{
	LOGI("创建描述符池");
	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
	{ {.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 10 } };

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{ .maxSets = 1, .poolSizeCount = (uint32_t)descriptorPoolSizes.size(), .pPoolSizes = descriptorPoolSizes.data() };

	descriptorPool = device->createDescriptorPoolUnique(descriptorPoolCreateInfo);
}

void CraneVision::VisionEngine::createDescriptorSet()
{
	LOGI("创建描述符集");
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{ .descriptorPool = descriptorPool.get(), .descriptorSetCount = 1, .pSetLayouts = &descriptorSetLayout.get() };

	descriptorSet = device->allocateDescriptorSets(descriptorSetAllocateInfo);

	// write
	for (size_t i = 0; i < descriptorSet.size(); i++)
	{
		for (auto& d : writeDescriptorSets)
			d.dstSet = descriptorSet[i];

		device->updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}
}

void CraneVision::VisionEngine::createPipelineLayout()
{
	LOGI("创建管线布局");

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{ .setLayoutCount = 1,
	.pSetLayouts = &descriptorSetLayout.get() };

	pipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutCreateInfo);
}

void CraneVision::VisionEngine::createPipelineCache()
{
	LOGI("创建管线缓存");

	vk::PipelineCacheCreateInfo pipelineCacheCreateInfo{};

	pipelineCache = device->createPipelineCacheUnique(pipelineCacheCreateInfo);
}

void CraneVision::VisionEngine::createPipeline()
{
	LOGI("创建计算管线");


	vk::PipelineShaderStageCreateInfo computeShaderStageCreateInfo{ .stage = vk::ShaderStageFlagBits::eCompute,
		.module = computeShader.get(), .pName = "main", .pSpecializationInfo = &specializationInfo };

	vk::ComputePipelineCreateInfo computePipelineCreateInfo{ .stage = computeShaderStageCreateInfo,
		.layout = pipelineLayout.get(), };

	pipeline = device->createComputePipelineUnique(pipelineCache.get(), computePipelineCreateInfo);
}

void CraneVision::VisionEngine::createCommandPool()
{
	LOGI("创建命令池");

	vk::CommandPoolCreateInfo commandPoolCreateInfo{ .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
	.queueFamilyIndex = computeQueueFamilyIndex };

	commandPool = device->createCommandPoolUnique(commandPoolCreateInfo);
}

void CraneVision::VisionEngine::allocateCommandBuffer()
{
	LOGI("分配命令缓冲");

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{ .commandPool = commandPool.get(),
		.level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };

	commandBuffer = device->allocateCommandBuffersUnique(commandBufferAllocateInfo);
}

void CraneVision::VisionEngine::buildCommandBuffer()
{
	LOGI("记录命令");
	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBuffer[0]->begin(commandBufferBeginInfo);

	commandBuffer[0]->bindPipeline(vk::PipelineBindPoint::eCompute, pipeline.get());

	commandBuffer[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout.get(), 0, 1, descriptorSet.data(), 0, nullptr);

	commandBuffer[0]->dispatch(1, 1, 1);

	commandBuffer[0]->end();

	// initialize compute submit info
	submitInfo.commandBufferCount = commandBuffer.size();
	submitInfo.pCommandBuffers = &commandBuffer[0].get();
}

vk::CommandBuffer CraneVision::VisionEngine::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{ .commandPool = commandPool.get(),
		.level = vk::CommandBufferLevel::ePrimary,  .commandBufferCount = 1 };

	auto singleBuffer = device->allocateCommandBuffers(commandBufferAllocateInfo);

	vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	singleBuffer[0].begin(beginInfo);

	return singleBuffer[0];
}

void CraneVision::VisionEngine::endSingleTimeCommands(vk::CommandBuffer cmdBuffer)
{
	cmdBuffer.end();

	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &cmdBuffer };

	computeQueue.submit(1, &submitInfo, vk::Fence{});
	computeQueue.waitIdle();

	device->freeCommandBuffers(commandPool.get(), 1, &cmdBuffer);
}

void CraneVision::VisionEngine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}
