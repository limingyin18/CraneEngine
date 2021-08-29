#include "SDL2_BASE.hpp"

using namespace std;
using namespace Crane;

static const string TITLE = "SDL2 WIN32 Testing";
static const uint32_t VERSION = VK_MAKE_VERSION(1, 0, 0);

SDL2_BASE::SDL2_BASE(shared_ptr<SDL_Window> win)
	: Engine{}, window{win}
{
	addInstanceExtensions();
}

void SDL2_BASE::addInstanceExtensions()
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

void SDL2_BASE::createSurface()
{
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(window.get(), instance.get(), &surface) != SDL_TRUE)
		throw runtime_error("failed to create surface");
	this->surface = vk::UniqueSurfaceKHR(surface, instance.get());
}