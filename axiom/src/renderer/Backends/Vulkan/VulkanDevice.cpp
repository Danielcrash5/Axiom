#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>

#include <axiom/renderer/RendererRHI.h>
#include <axiom/core/Logger.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h> // SDLs Vulkan-Anbindung
#include <stdexcept>
#include <vector>
#include <iostream>
#include <algorithm>

namespace axiom {
	inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	// Das private Herz unseres Vulkan-Geräts
	struct GraphicsDeviceImpl {
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphicsQueue = VK_NULL_HANDLE;
		uint32_t graphicsQueueFamilyIndex = 0;
		SDL_Window* window = nullptr;

		// Swapchain Ressourcen
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D swapchainExtent = { 0, 0 };
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;

		// Synchronisation für Doppel-Pufferung (In-Flight)
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		uint32_t currentFrameIndex = 0; // Schaltet jeden Frame zwischen 0 und 1 um
		uint32_t currentSwapchainImageIndex = 0; // Welches konkrete Swapchain-Bild wir gerade nutzen

		// Command Pools und Command Buffer pro In-Flight-Frame
		std::vector<VkCommandPool> commandPools;
		std::vector<VkCommandBuffer> commandBuffers;
	};

	void create_swapchain_internal(GraphicsDeviceImpl* impl, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE) {
		// Falls wir ein Re-Creation durchführen, müssen wir zuerst auf die GPU warten,
		// damit wir keine Ressourcen löschen, die gerade noch im Flug sind!
		if (oldSwapchain != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(impl->device);

			// Die alten ImageViews zerstören
			for (auto imageView : impl->swapchainImageViews) {
				vkDestroyImageView(impl->device, imageView, nullptr);
			}
			impl->swapchainImageViews.clear();
			impl->swapchainImages.clear();
		}

		// 1. Surface via SDL3 erstellen (nur beim ersten Mal!)
		if (impl->surface == VK_NULL_HANDLE) {
			if (!SDL_Vulkan_CreateSurface(impl->window, impl->instance, nullptr, &impl->surface)) {
				throw std::runtime_error(std::string("Vulkan Surface Fehler: ") + SDL_GetError());
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(impl->physicalDevice, impl->graphicsQueueFamilyIndex, impl->surface, &presentSupport);
			if (!presentSupport) {
				throw std::runtime_error("Graphics Queue unterstützt keine Präsentation!");
			}
		}

		// 2. Capabilities abfragen, um neue Dimensionen zu validieren
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(impl->physicalDevice, impl->surface, &capabilities);

		// Wenn das Fenster minimiert ist, sind die Extents 0. Wir fangen das ab.
		if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0) {
			impl->swapchainExtent = { 0, 0 };
			return;
		}

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			impl->swapchainExtent = capabilities.currentExtent;
		}
		else {
			int w, h;
			SDL_GetWindowSize(impl->window, &w, &h);
			impl->swapchainExtent = {
				std::clamp(static_cast<uint32_t>(w), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp(static_cast<uint32_t>(h), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		}

		// 3. Format bestimmen (wie gehabt)
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(impl->physicalDevice, impl->surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(impl->physicalDevice, impl->surface, &formatCount, formats.data());

		impl->swapchainFormat = formats[0].format;
		for (const auto& availableFormat : formats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				impl->swapchainFormat = availableFormat.format;
				break;
			}
		}

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			imageCount = capabilities.maxImageCount;
		}

		// 4. Swapchain erzeugen – hier übergeben wir die OLD Swapchain
		VkSwapchainCreateInfoKHR createInfo {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = impl->surface,
			.minImageCount = imageCount,
			.imageFormat = impl->swapchainFormat,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = impl->swapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_FIFO_KHR,
			.clipped = VK_TRUE,
			.oldSwapchain = oldSwapchain // VULKAN OPTIMIERUNG
		};

		VkSwapchainKHR newSwapchain;
		if (vkCreateSwapchainKHR(impl->device, &createInfo, nullptr, &newSwapchain) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan Swapchain konnte nicht neu erstellt werden!");
		}

		// Jetzt, wo die neue steht, können wir die alte physisch zerstören
		if (oldSwapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(impl->device, oldSwapchain, nullptr);
		}
		impl->swapchain = newSwapchain;

		// 5. Images & Views neu aufbauen
		vkGetSwapchainImagesKHR(impl->device, impl->swapchain, &imageCount, nullptr);
		impl->swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(impl->device, impl->swapchain, &imageCount, impl->swapchainImages.data());

		impl->swapchainImageViews.resize(imageCount);
		for (size_t i = 0; i < impl->swapchainImages.size(); i++) {
			VkImageViewCreateInfo viewInfo {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = impl->swapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = impl->swapchainFormat,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.levelCount = 1,
					.layerCount = 1
				}
			};
			vkCreateImageView(impl->device, &viewInfo, nullptr, &impl->swapchainImageViews[i]);
		}
	}

	// RHI-Funktion ausführen
	void GraphicsDevice::handle_resize(int newWidth, int newHeight) {
		// Ruft den internen Worker auf und reicht die aktuelle Swapchain als 'old' ein
		create_swapchain_internal(m_impl, m_impl->swapchain);
	}

	GraphicsDevice::GraphicsDevice(SDL_Window* window) {
		m_impl = new GraphicsDeviceImpl();
		m_impl->window = window;

		// 1. Volk initialisieren (lädt vkCreateInstance aus dem Treiber/Loader)
		if (volkInitialize() != VK_SUCCESS) {
			throw std::runtime_error("Volk konnte nicht initialisiert werden! Ist ein Grafiktreiber installiert?");
		}

		// 2. Vulkan Instanz erstellen
		VkApplicationInfo appInfo {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Axiom Engine",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Axiom",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_3 // Vulkan 1.3 garantiert uns Dynamic Rendering!
		};

		// SDL3 Extensions für das Window-Surface abfragen
		uint32_t extensionCount = 0;
		const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

		VkInstanceCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = extensionCount,
			.ppEnabledExtensionNames = sdlExtensions
		};

		if (vkCreateInstance(&createInfo, nullptr, &m_impl->instance) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan Instance konnte nicht erstellt werden!");
		}

		// Volk mit der erstellten Instanz füttern (lädt alle instanzabhängigen Funktionen)
		volkLoadInstance(m_impl->instance);

		// 3. Passende GPU (Physical Device) auswählen
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("Keine GPU mit Vulkan-Unterstützung gefunden!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, devices.data());

		// Einfache Strategie: Wir nehmen die erste dedizierte GPU, sonst die erste beste
		for (const auto& device : devices) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);

			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				m_impl->physicalDevice = device;
				AXIOM_INFO("Gewählte GPU: {} (Dedicated)", props.deviceName);
				break;
			}
		}

		if (m_impl->physicalDevice == VK_NULL_HANDLE) {
			m_impl->physicalDevice = devices[0];
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(m_impl->physicalDevice, &props);
			AXIOM_INFO("Gewählte GPU: {} (Fallback)", props.deviceName);
		}

		// 4. Queue Families suchen
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physicalDevice, &queueFamilyCount, queueFamilies.data());

		bool foundGraphicsQueue = false;
		for (uint32_t i = 0; i < queueFamilyCount; i++) {
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				m_impl->graphicsQueueFamilyIndex = i;
				foundGraphicsQueue = true;
				break;
			}
		}

		if (!foundGraphicsQueue) {
			throw std::runtime_error("Keine passende Graphics Queue Family gefunden!");
		}

		// 5. Logisches Device mit Dynamic Rendering Features erstellen
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_impl->graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// Hier aktivieren wir die Vulkan 1.3 Features (Dynamic Rendering)
		VkPhysicalDeviceVulkan13Features features13 {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.dynamicRendering = VK_TRUE // Schaltet vkCmdBeginRendering frei!
		};

		// Erwähnte Extensions (z.B. KHR_swapchain für die Bildausgabe)
		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkDeviceCreateInfo deviceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features13, // Features an die Kette hängen
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data()
		};

		if (vkCreateDevice(m_impl->physicalDevice, &deviceCreateInfo, nullptr, &m_impl->device) != VK_SUCCESS) {
			throw std::runtime_error("Logisches Vulkan Device konnte nicht erstellt werden!");
		}

		// Volk mit dem Device füttern (lädt alle geräteabhängigen Funktionen mit maximaler Performance)
		volkLoadDevice(m_impl->device);

		// Queue abgreifen
		vkGetDeviceQueue(m_impl->device, m_impl->graphicsQueueFamilyIndex, 0, &m_impl->graphicsQueue);

		// Swapchain generieren
		create_swapchain_internal(m_impl);

		m_impl->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_impl->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_impl->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT // WICHTIG: Startet signalisiert, damit der erste Frame nicht unendlich wartet!
		};

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_impl->device, &semaphoreInfo, nullptr, &m_impl->imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_impl->device, &semaphoreInfo, nullptr, &m_impl->renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_impl->device, &fenceInfo, nullptr, &m_impl->inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("Vulkan Synchronisationsobjekte konnten nicht erstellt werden!");
			}
		}

		m_impl->commandPools.resize(MAX_FRAMES_IN_FLIGHT);
		m_impl->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandPoolCreateInfo poolInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			// TRANSIENT bedeutet: Wir zerstören/resetten die Buffer hieraus sehr häufig
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = m_impl->graphicsQueueFamilyIndex
		};

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateCommandPool(m_impl->device, &poolInfo, nullptr, &m_impl->commandPools[i]) != VK_SUCCESS) {
				throw std::runtime_error("VkCommandPool konnte nicht erstellt werden!");
			}

			// Direkt einen primären Command Buffer aus dem neuen Pool allokieren
			VkCommandBufferAllocateInfo allocInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = m_impl->commandPools[i],
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};

			if (vkAllocateCommandBuffers(m_impl->device, &allocInfo, &m_impl->commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("VkCommandBuffer konnte nicht allokiert werden!");
			}
		}
	}

	GraphicsDevice::~GraphicsDevice() {
		if (m_impl) {
			if (m_impl->device != VK_NULL_HANDLE) {
				// 1. Image Views freigeben
				for (auto imageView : m_impl->swapchainImageViews) {
					vkDestroyImageView(m_impl->device, imageView, nullptr);
				}
				// 2. Swapchain zerstören
				vkDestroySwapchainKHR(m_impl->device, m_impl->swapchain, nullptr);

				vkDestroyDevice(m_impl->device, nullptr);
			}

			if (m_impl->instance != VK_NULL_HANDLE) {
				// 3. Surface zerstören
				if (m_impl->surface != VK_NULL_HANDLE) {
					vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
				}
				vkDestroyInstance(m_impl->instance, nullptr);
			}

			vkDestroyCommandPool(m_impl->device, pool, nullptr);
			delete m_impl;
		}
	}

	// Stubs für den Compiler-Erfolg
	BufferHandle GraphicsDevice::create_buffer(uint64_t size, BufferUsage usage) {
		return 0;
	}
	TextureHandle GraphicsDevice::create_texture(TextureFormat format, uint32_t width, uint32_t height) {
		return 0;
	}

	bool GraphicsDevice::begin_frame() {
		vkWaitForFences(m_impl->device, 1, &m_impl->inFlightFences[m_impl->currentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(
			m_impl->device, m_impl->swapchain, std::numeric_limits<uint64_t>::max(),
			m_impl->imageAvailableSemaphores[m_impl->currentFrameIndex], VK_NULL_HANDLE, &imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			handle_resize(0, 0);
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Fehler beim Anfordern des nächsten Swapchain-Bildes!");
		}

		m_impl->currentSwapchainImageIndex = imageIndex;
		vkResetFences(m_impl->device, 1, &m_impl->inFlightFences[m_impl->currentFrameIndex]);

		// NEU: Den Command Pool für diesen Frame komplett zurücksetzen (löscht alle alten Befehle)
		vkResetCommandPool(m_impl->device, m_impl->commandPools[m_impl->currentFrameIndex], 0);

		// NEU: Command Buffer Aufzeichnung starten
		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT // Wird nur genau einmal abgeschickt
		};

		vkBeginCommandBuffer(m_impl->commandBuffers[m_impl->currentFrameIndex], &beginInfo);

		return true;
	}

	void GraphicsDevice::end_frame() {
		// NEU: Command Buffer Aufzeichnung beenden
		vkEndCommandBuffer(m_impl->commandBuffers[m_impl->currentFrameIndex]);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_impl->imageAvailableSemaphores[m_impl->currentFrameIndex],
			.pWaitDstStageMask = waitStages,
			// MODIFIZIERT: Jetzt übergeben wir den echten aufgezeichneten Command Buffer!
			.commandBufferCount = 1,
			.pCommandBuffers = &m_impl->commandBuffers[m_impl->currentFrameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &m_impl->renderFinishedSemaphores[m_impl->currentFrameIndex]
		};

		if (vkQueueSubmit(m_impl->graphicsQueue, 1, &submitInfo, m_impl->inFlightFences[m_impl->currentFrameIndex]) != VK_SUCCESS) {
			throw std::runtime_error("Fehler beim Abschicken der Befehle an die GPU!");
		}

		VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_impl->renderFinishedSemaphores[m_impl->currentFrameIndex],
			.swapchainCount = 1,
			.pSwapchains = &m_impl->swapchain,
			.pImageIndices = &m_impl->currentSwapchainImageIndex
		};

		VkResult result = vkQueuePresentKHR(m_impl->graphicsQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			handle_resize(0, 0);
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Fehler beim Präsentieren des Swapchain-Bildes!");
		}

		m_impl->currentFrameIndex = (m_impl->currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void GraphicsDevice::submit(CommandBuffer& cmd) {
	}

} // namespace Axiom