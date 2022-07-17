#include "SDL2_IMGUI_BASE.hpp"

using namespace std;
using namespace Crane;

static const string TITLE = "SDL2 WIN32 Testing";
static const uint32_t VERSION = VK_MAKE_VERSION(1, 0, 0);

SDL2_IMGUI_BASE::SDL2_IMGUI_BASE(shared_ptr<SDL_Window> win)
	: Engine{}, window{ win }, imguiContext{ nullptr, ImGui::DestroyContext }
{
	addInstanceExtensions();
	drawGUIFlag = true;
}

SDL2_IMGUI_BASE::~SDL2_IMGUI_BASE()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
}

void SDL2_IMGUI_BASE::addInstanceExtensions()
{
	unsigned extensionCount = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(window.get(), &extensionCount, nullptr))
		throw runtime_error("failed to get instance extensions required by SDL");

	size_t addExtensionCount = instanceExtensions.size();
	instanceExtensions.resize(addExtensionCount + extensionCount);

	if (!SDL_Vulkan_GetInstanceExtensions(window.get(), &extensionCount,
										  instanceExtensions.data() + addExtensionCount))
		throw runtime_error("failed to get instance extensions required by SDL");

	// erase duplicate
	sort(instanceExtensions.begin(), instanceExtensions.end(),
		 [](const char *a, const char *b)
		 { return strcmp(a, b) < 0; });
	auto it = unique(instanceExtensions.begin(), instanceExtensions.end(),
					 [](const char *a, const char *b)
					 { return strcmp(a, b) == 0; });
	instanceExtensions.erase(it, instanceExtensions.end());
}

void SDL2_IMGUI_BASE::createSurface()
{
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(window.get(), instance.get(), &surface) != SDL_TRUE)
		throw runtime_error("failed to create surface");
	this->surface = vk::UniqueSurfaceKHR(surface, instance.get());
}

void SDL2_IMGUI_BASE::createAssetApp()
{
	LOGI("SDL2_IMGUI ��ʼ��");

	vk::CommandPoolCreateInfo guiCmdPoolCreateInfo = {
		.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = graphicsQueueFamilyIndex };
	guiCommandPool = device->createCommandPoolUnique(guiCmdPoolCreateInfo);

	vk::CommandBufferAllocateInfo cmdAllocInfo = {
		.commandPool = guiCommandPool.get(),
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = static_cast<uint32_t>(swapchainImages.size()) };
	guiCmdBuffs = device->allocateCommandBuffersUnique(cmdAllocInfo);

	imguiInit();
}

void SDL2_IMGUI_BASE::imguiInit()
{
	//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	vk::DescriptorPoolSize pool_sizes[] =
		{
			{vk::DescriptorType::eSampler, 1000},
			{vk::DescriptorType::eCombinedImageSampler, 1000},
			{vk::DescriptorType::eSampledImage, 1000},
			{vk::DescriptorType::eStorageImage, 1000},
			{vk::DescriptorType::eUniformTexelBuffer, 1000},
			{vk::DescriptorType::eStorageTexelBuffer, 1000},
			{vk::DescriptorType::eUniformBuffer, 1000},
			{vk::DescriptorType::eStorageBuffer, 1000},
			{vk::DescriptorType::eUniformBufferDynamic, 1000},
			{vk::DescriptorType::eStorageBufferDynamic, 1000},
			{vk::DescriptorType::eInputAttachment, 1000}};

	vk::DescriptorPoolCreateInfo pool_info = {
	.flags =vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
	.maxSets = 1000,
	.poolSizeCount = std::size(pool_sizes),
	.pPoolSizes = pool_sizes
	};

	imguiPool = device->createDescriptorPoolUnique(pool_info);

	// 2: initialize imgui library

	//this initializes the core structures of imgui
	imguiContext.reset(ImGui::CreateContext());
	ImGui::SetCurrentContext(imguiContext.get());

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	/*
	ImGui_ImplVulkan_LoadFunctions(
		[](const char *function_name, void *)
		{ return vkGetInstanceProcAddr(instance.get(), function_name); });*/
	//this initializes imgui for SDL
	ImGui_ImplSDL2_InitForVulkan(window.get());

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance.get();
	init_info.PhysicalDevice = physicalDevice;
	init_info.Device = device.get();
	init_info.Queue = graphicsQueue;
	init_info.DescriptorPool = imguiPool.get();
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;

	ImGui_ImplVulkan_Init(&init_info, renderPass.get());

	//execute a gpu command to upload imgui font textures
	auto singleCmd = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(singleCmd);
	endSingleTimeCommands(singleCmd);

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void SDL2_IMGUI_BASE::drawGUI()
{
	//imgui new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(window.get());
	ImGui::NewFrame();

	setImgui();

	ImGui::Render();

	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	guiCmdBuffs[currBuffIndex]->begin(commandBufferBeginInfo);

	VkRenderPassBeginInfo rpBeginInfo{};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = nullptr;
	rpBeginInfo.renderPass = guiRenderPass.get();
	rpBeginInfo.framebuffer = framebuffers[currBuffIndex].get();
	rpBeginInfo.renderArea.offset.x = 0;
	rpBeginInfo.renderArea.offset.y = 0;
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = nullptr;

	vkCmdBeginRenderPass(guiCmdBuffs[currBuffIndex].get(), &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), guiCmdBuffs[currBuffIndex].get());

	vkCmdEndRenderPass(guiCmdBuffs[currBuffIndex].get());

	if (vkEndCommandBuffer(guiCmdBuffs[currBuffIndex].get()) != VK_SUCCESS)
		throw runtime_error("failed to record command buffer");

	submitInfo.pCommandBuffers = &guiCmdBuffs[currBuffIndex].get();
	submitInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame].get();
	submitInfo.pSignalSemaphores = &guiRenderFinishedSemaphores[currentFrame].get();
	graphicsQueue.submit(submitInfo, inFlightFences[currentFrame].get());
	presentInfo.pWaitSemaphores = &guiRenderFinishedSemaphores[currentFrame].get();
}

void SDL2_IMGUI_BASE::setImgui()
{
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

	if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}