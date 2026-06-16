#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>

// VMA Konfiguration für die Kopplung mit Volk
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vma/vk_mem_alloc.h>

#include <axiom/renderer/RendererRHI.h>
#include <axiom/core/Logger.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>

namespace axiom {

	inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	// Interne Struktur für das Tracking allokierter Buffer im Ressourcen-Pool
	struct VulkanBufferResource {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = nullptr;
		VmaAllocationInfo allocInfo {};
		uint64_t size = 0;
	};

	// Private Datenstruktur (Pimpl-Versteck für Vulkan-Interna)
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

		// Synchronisation für In-Flight-Frames
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		// Command-Infrastruktur pro Frame
		std::vector<VkCommandPool> commandPools;
		std::vector<VkCommandBuffer> commandBuffers;

		// Speicherverwaltung
		VmaAllocator allocator = VK_NULL_HANDLE;
		std::vector<VulkanBufferResource> buffers;

		uint32_t currentFrameIndex = 0;
		uint32_t currentSwapchainImageIndex = 0;

		// Transient Allocator Daten
		static constexpr uint64_t TRANSIENT_POOL_SIZE = 16 * 1024 * 1024; // 16 MB pro Frame
		std::vector<BufferHandle> transientBuffers;
		uint64_t transientOffset = 0;
	};

	// Interne Hilfsfunktion zur Erstellung/Wiedererstellung der Swapchain
	void create_swapchain_internal(GraphicsDeviceImpl* impl, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE) {
		if (oldSwapchain != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(impl->device);
			for (auto imageView : impl->swapchainImageViews) {
				vkDestroyImageView(impl->device, imageView, nullptr);
			}
			impl->swapchainImageViews.clear();
			impl->swapchainImages.clear();
		}

		if (impl->surface == VK_NULL_HANDLE) {
			if (!SDL_Vulkan_CreateSurface(impl->window, impl->instance, nullptr, &impl->surface)) {
				throw std::runtime_error(std::string("[Vulkan] Surface-Erstellung fehlgeschlagen: ") + SDL_GetError());
			}
		}

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(impl->physicalDevice, impl->surface, &capabilities);

		// Abfangen, falls das Fenster minimiert ist (Breite/Höhe ist 0)
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
			.presentMode = VK_PRESENT_MODE_FIFO_KHR, // Standard-VSync
			.clipped = VK_TRUE,
			.oldSwapchain = oldSwapchain
		};

		VkSwapchainKHR newSwapchain;
		if (vkCreateSwapchainKHR(impl->device, &createInfo, nullptr, &newSwapchain) != VK_SUCCESS) {
			throw std::runtime_error("[Vulkan] Konnte Swapchain nicht erzeugen!");
		}

		if (oldSwapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(impl->device, oldSwapchain, nullptr);
		}
		impl->swapchain = newSwapchain;

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
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			if (vkCreateImageView(impl->device, &viewInfo, nullptr, &impl->swapchainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("[Vulkan] ImageView-Erstellung für Swapchain-Bild fehlgeschlagen!");
			}
		}
	}

	GraphicsDevice::GraphicsDevice(SDL_Window* window) {
		m_impl = new GraphicsDeviceImpl();
		m_impl->window = window;

		// 1. Volk initialisieren
		if (volkInitialize() != VK_SUCCESS) {
			throw std::runtime_error("[Vulkan] Volk Meta-Loader konnte nicht initialisiert werden!");
		}

		// 2. Instanz aufbauen (Vulkan 1.3 zwingend für Dynamic Rendering)
		VkApplicationInfo appInfo {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Axiom Engine",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Axiom",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_3
		};

		uint32_t extensionCount = 0;
		const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

		VkInstanceCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = extensionCount,
			.ppEnabledExtensionNames = sdlExtensions
		};

		if (vkCreateInstance(&createInfo, nullptr, &m_impl->instance) != VK_SUCCESS) {
			throw std::runtime_error("[Vulkan] Instance-Erstellung fehlgeschlagen!");
		}
		volkLoadInstance(m_impl->instance);

		// 3. Physikalische GPU wählen
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, nullptr);
		if (deviceCount == 0) throw std::runtime_error("[Vulkan] Keine GPU mit Vulkan-Support gefunden!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, devices.data());

		for (const auto& dev : devices) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(dev, &props);
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				m_impl->physicalDevice = dev;
				AXIOM_INFO("[Vulkan] GPU gewählt: {} (Dediziert)", props.deviceName);
				break;
			}
		}
		if (m_impl->physicalDevice == VK_NULL_HANDLE) {
			m_impl->physicalDevice = devices[0];
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(m_impl->physicalDevice, &props);
			AXIOM_INFO("[Vulkan] GPU gewählt: {} (Fallback)", props.deviceName);
		}

		// 4. Queue Families auflösen
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physicalDevice, &queueFamilyCount, queueFamilies.data());

		bool foundGraphics = false;
		for (uint32_t i = 0; i < queueFamilyCount; i++) {
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				m_impl->graphicsQueueFamilyIndex = i;
				foundGraphics = true;
				break;
			}
		}
		if (!foundGraphics) throw std::runtime_error("[Vulkan] Keine Graphics Queue Family gefunden!");

		// 5. Logisches Device erstellen und Dynamic Rendering aktivieren
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_impl->graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkPhysicalDeviceVulkan13Features features13 {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.dynamicRendering = VK_TRUE // Schaltet vkCmdBeginRendering frei
		};

		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo deviceCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features13,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data()
		};

		if (vkCreateDevice(m_impl->physicalDevice, &deviceCreateInfo, nullptr, &m_impl->device) != VK_SUCCESS) {
			throw std::runtime_error("[Vulkan] Logisches Device konnte nicht erzeugt werden!");
		}
		volkLoadDevice(m_impl->device);

		vkGetDeviceQueue(m_impl->device, m_impl->graphicsQueueFamilyIndex, 0, &m_impl->graphicsQueue);

		// 6. Swapchain initialisieren
		create_swapchain_internal(m_impl);

		// 7. Synchronisations-Strukturen aufbauen
		m_impl->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_impl->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_impl->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT // Startet geöffnet!
		};

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_impl->device, &semInfo, nullptr, &m_impl->imageAvailableSemaphores[i]);
			vkCreateSemaphore(m_impl->device, &semInfo, nullptr, &m_impl->renderFinishedSemaphores[i]);
			vkCreateFence(m_impl->device, &fenceInfo, nullptr, &m_impl->inFlightFences[i]);
		}

		// 8. Command Pools und primary Command Buffer allokieren
		m_impl->commandPools.resize(MAX_FRAMES_IN_FLIGHT);
		m_impl->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandPoolCreateInfo poolInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = m_impl->graphicsQueueFamilyIndex
		};

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkCreateCommandPool(m_impl->device, &poolInfo, nullptr, &m_impl->commandPools[i]);

			VkCommandBufferAllocateInfo allocInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = m_impl->commandPools[i],
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};
			vkAllocateCommandBuffers(m_impl->device, &allocInfo, &m_impl->commandBuffers[i]);
		}

		// 9. VMA (Vulkan Memory Allocator) initialisieren
		VmaVulkanFunctions vmaFunctions {
			.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
			.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};

		VmaAllocatorCreateInfo allocAllocInfo {
			.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice = m_impl->physicalDevice,
			.device = m_impl->device,
			.pVulkanFunctions = &vmaFunctions,
			.instance = m_impl->instance,
			.vulkanApiVersion = VK_API_VERSION_1_3
		};
		vmaCreateAllocator(&allocAllocInfo, &m_impl->allocator);

		// Zwei große transiente Buffer erzeugen (einen pro In-Flight-Frame)
		m_impl->transientBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			// Wir flaggen den Buffer als Vertex-, Index- und Uniform-tauglich
			m_impl->transientBuffers[i] = create_buffer(
				GraphicsDeviceImpl::TRANSIENT_POOL_SIZE,
				static_cast<BufferUsage>(
				static_cast<uint32_t>(BufferUsage::Vertex) |
				static_cast<uint32_t>(BufferUsage::Index) |
				static_cast<uint32_t>(BufferUsage::Uniform)
			)
			);
		}
	}

	GraphicsDevice::~GraphicsDevice() {
		if (m_impl) {
			if (m_impl->device != VK_NULL_HANDLE) {
				vkDeviceWaitIdle(m_impl->device);

				for (auto& buf : m_impl->buffers) {
					if (buf.buffer != VK_NULL_HANDLE) {
						vmaDestroyBuffer(m_impl->allocator, buf.buffer, buf.allocation);
					}
				}

				if (m_impl->allocator != VK_NULL_HANDLE) {
					vmaDestroyAllocator(m_impl->allocator);
				}

				for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					vkDestroyCommandPool(m_impl->device, m_impl->commandPools[i], nullptr);
					vkDestroySemaphore(m_impl->device, m_impl->imageAvailableSemaphores[i], nullptr);
					vkDestroySemaphore(m_impl->device, m_impl->renderFinishedSemaphores[i], nullptr);
					vkDestroyFence(m_impl->device, m_impl->inFlightFences[i], nullptr);
				}

				for (auto imageView : m_impl->swapchainImageViews) {
					vkDestroyImageView(m_impl->device, imageView, nullptr);
				}
				vkDestroySwapchainKHR(m_impl->device, m_impl->swapchain, nullptr);
				vkDestroyDevice(m_impl->device, nullptr);
			}

			if (m_impl->instance != VK_NULL_HANDLE) {
				if (m_impl->surface != VK_NULL_HANDLE) {
					vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
				}
				vkDestroyInstance(m_impl->instance, nullptr);
			}
			delete m_impl;
		}
	}

	void GraphicsDevice::handle_resize(int, int) {
		create_swapchain_internal(m_impl, m_impl->swapchain);
	}

	void* GraphicsDevice::get_native_buffer_handle(BufferHandle handle) {
		if (!m_impl || static_cast<size_t>(handle) >= m_impl->buffers.size()) {
			return nullptr;
		}
		return static_cast<void*>(m_impl->buffers[static_cast<size_t>(handle)].buffer);
	}

	bool GraphicsDevice::begin_frame(CommandBuffer& outCmdBuffer) {
		if (m_impl->swapchainExtent.width == 0 || m_impl->swapchainExtent.height == 0) {
			return false;
		}

		// Achtung: m_impl->device nutzen!
		vkWaitForFences(m_impl->device, 1, &m_impl->inFlightFences[m_impl->currentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(
			m_impl->device, m_impl->swapchain, std::numeric_limits<uint64_t>::max(),
			m_impl->imageAvailableSemaphores[m_impl->currentFrameIndex], VK_NULL_HANDLE, &imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			handle_resize(0, 0); // Richtiger Aufruf über Member-Kontext
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("[Vulkan] Konnte kein neues Swapchain-Bild akquirieren!");
		}

		m_impl->currentSwapchainImageIndex = imageIndex;
		vkResetFences(m_impl->device, 1, &m_impl->inFlightFences[m_impl->currentFrameIndex]);

		vkResetCommandPool(m_impl->device, m_impl->commandPools[m_impl->currentFrameIndex], 0);

		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		vkBeginCommandBuffer(m_impl->commandBuffers[m_impl->currentFrameIndex], &beginInfo);

		m_impl->transientOffset = 0;

		// Weitergabe des rohen Handles an das RHI-Objekt
		outCmdBuffer.set_native_handle(m_impl->commandBuffers[m_impl->currentFrameIndex], this, m_impl->currentSwapchainImageIndex);

		return true;
	}

	void GraphicsDevice::end_frame() {
		vkEndCommandBuffer(m_impl->commandBuffers[m_impl->currentFrameIndex]);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_impl->imageAvailableSemaphores[m_impl->currentFrameIndex],
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_impl->commandBuffers[m_impl->currentFrameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &m_impl->renderFinishedSemaphores[m_impl->currentFrameIndex]
		};

		if (vkQueueSubmit(m_impl->graphicsQueue, 1, &submitInfo, m_impl->inFlightFences[m_impl->currentFrameIndex]) != VK_SUCCESS) {
			throw std::runtime_error("[Vulkan] Befehlsabgabe an die Queue schlug fehl!");
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
			throw std::runtime_error("[Vulkan] Präsentation des Frames fehlgeschlagen!");
		}

		m_impl->currentFrameIndex = (m_impl->currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;


	}

	TransientAllocation GraphicsDevice::allocate_transient(uint64_t size, uint32_t alignment) {
		// Alignment-Korrektur (Vulkan verlangt oft spezifische Ausrichtungen, z.B. 16 oder 64 Bytes)
		uint64_t alignedOffset = (m_impl->transientOffset + alignment - 1) & ~(alignment - 1);

		if (alignedOffset + size > GraphicsDeviceImpl::TRANSIENT_POOL_SIZE) {
			throw std::runtime_error("[Vulkan] Transient Allocator Out of Memory! Pool-Größe reicht nicht aus.");
		}

		// Offset updaten
		m_impl->transientOffset = alignedOffset + size;

		// Aktiven physischen Buffer dieses Frames holen
		BufferHandle activeBuffer = m_impl->transientBuffers[m_impl->currentFrameIndex];
		const auto& internalBuffer = m_impl->buffers[activeBuffer];

		// Pointer errechnen
		uint8_t* basePointer = static_cast<uint8_t*>(internalBuffer.allocInfo.pMappedData);

		return TransientAllocation {
			.bufferHandle = activeBuffer,
			.offset = alignedOffset,
			.pMappedData = basePointer + alignedOffset
		};
	}

	BufferHandle GraphicsDevice::create_buffer(uint64_t size, BufferUsage usage) {
		VkBufferCreateInfo bufferInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		if (static_cast<uint32_t>(usage) & static_cast<uint32_t>(BufferUsage::Vertex))   bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (static_cast<uint32_t>(usage) & static_cast<uint32_t>(BufferUsage::Index))    bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (static_cast<uint32_t>(usage) & static_cast<uint32_t>(BufferUsage::Uniform))  bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (static_cast<uint32_t>(usage) & static_cast<uint32_t>(BufferUsage::Storage))  bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (static_cast<uint32_t>(usage) & static_cast<uint32_t>(BufferUsage::Indirect)) bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

		VmaAllocationCreateInfo allocCreateInfo {
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO
		};

		VulkanBufferResource res {};
		res.size = size;

		if (vmaCreateBuffer(m_impl->allocator, &bufferInfo, &allocCreateInfo, &res.buffer, &res.allocation, &res.allocInfo) != VK_SUCCESS) {
			throw std::runtime_error("[Vulkan] VMA konnt den Buffer-Speicher nicht bereitstellen!");
		}

		m_impl->buffers.push_back(res);
		return static_cast<BufferHandle>(m_impl->buffers.size() - 1);
	}

	TextureHandle GraphicsDevice::create_texture(TextureFormat, uint32_t, uint32_t) {
		return 0;
	}
	void GraphicsDevice::submit(CommandBuffer&) {
	}

	void* GraphicsDevice::get_current_swapchain_image_view() {
		if (!m_impl || m_impl->swapchainImageViews.empty()) return nullptr;
		return m_impl->swapchainImageViews[m_impl->currentSwapchainImageIndex];
	}

	void* GraphicsDevice::get_current_swapchain_image() {
		if (!m_impl || m_impl->swapchainImages.empty()) return nullptr;
		return m_impl->swapchainImages[m_impl->currentSwapchainImageIndex];
	}

	void GraphicsDevice::get_swapchain_extent(uint32_t& outWidth, uint32_t& outHeight) {
		if (!m_impl) {
			outWidth = 0;
			outHeight = 0;
			return;
		}
		outWidth = m_impl->swapchainExtent.width;
		outHeight = m_impl->swapchainExtent.height;
	}

} // namespace Axiom