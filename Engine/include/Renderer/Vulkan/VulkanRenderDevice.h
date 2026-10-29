#pragma once

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif

#include "Renderer/IRenderDevice.h"

#include <volk.h>
#include <vk_mem_alloc.h>

#include <array>
#include <functional>
#include <unordered_map>
#include <vector>

class VulkanShader;
class VulkanTexture;
class BufferLayout;

class VulkanRenderDevice : public IRenderDevice {
public:
    VulkanRenderDevice() = default;
    virtual ~VulkanRenderDevice();

	bool Init(IWindow* window) override;
	void Shutdown() override;

	void EnableVsync(bool enable) override;
	void EnableBlending(bool enable) override;
	void EnableDepthTest(bool enable) override;

	void SetViewport(int x, int y, int width, int height) override;
	void SetClearColor(float r, float g, float b, float a) override;
	void Clear(ClearFlags flags) override;

	void DrawIndexed(
		IVertexArray* vertexArray,
		uint32_t indexCount = 0,
		DrawIndexSize size = DrawIndexSize::UShort) override;

	void Present() override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	ITexture* CreateTexture() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IShader* CreateShader() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IVertexArray* CreateVertexArray() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IVertexBuffer* CreateVertexBuffer() const override;

	[[nodiscard("Returned pointer will leak memory if not handled")]]
	IIndexBuffer* CreateIndexBuffer() const override;

	VkDevice GetDevice() const { return mDevice; }
	VmaAllocator GetAllocator() const { return mAllocator; }
	VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
	VkCommandPool GetCommandPool() const { return mCommandPool; }
	VkQueue GetGraphicsQueue() const { return mGraphicsQueue; }
	VkCommandBuffer GetCurrentCommandBuffer() const {
		return mFrameStarted ? mCommandBuffers[mImageIndex] : VK_NULL_HANDLE;
	}
	bool ExecuteImmediate(const std::function<void(VkCommandBuffer)>& command) const;

	static constexpr uint32_t MaxTextureSlots = 32;
	static constexpr uint32_t MaxFramesInFlight = 2;
	uint32_t GetFrameIndex() const { return mCurrentFrame; }
	uint64_t GetFrameSerial() const { return mFrameSerial; }
	void SetBoundShader(const VulkanShader* shader) { mBoundShader = shader; }
	void BindTexture(uint32_t slot, const VulkanTexture* texture);
	void OnShaderDestroyed(const VulkanShader* shader);

private:
	bool CreateSwapchain();
	void DestroySwapchain();
	bool CreateDrawResources();
	void DestroyDrawResources();
	VkPipeline GetPipeline(const VulkanShader& shader, const BufferLayout& layout);
	bool BeginFrame();
	void EndFrame();

	VkInstance mInstance = VK_NULL_HANDLE;
	VkSurfaceKHR mSurface = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;
	VmaAllocator mAllocator = VK_NULL_HANDLE;
	VkQueue mGraphicsQueue = VK_NULL_HANDLE;
	VkQueue mPresentQueue = VK_NULL_HANDLE;
	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> mCommandBuffers;
	std::vector<VkImage> mSwapchainImages;
	std::vector<VkImageView> mSwapchainImageViews;
	std::vector<VkFramebuffer> mFramebuffers;
	std::vector<VkFence> mImagesInFlight;
	VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
	VkRenderPass mRenderPass = VK_NULL_HANDLE;
	// One depth buffer and render-finished semaphore per swapchain image so
	// frames in flight never share them
	std::vector<VkImage> mDepthImages;
	std::vector<VmaAllocation> mDepthAllocations;
	std::vector<VkImageView> mDepthViews;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	VkFormat mSwapchainFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D mSwapchainExtent{};
	std::array<VkSemaphore, 2> mImageAvailableSemaphores{};
	std::array<VkFence, 2> mInFlightFences{};
	uint32_t mCurrentFrame = 0;
	uint32_t mImageIndex = 0;
	uint32_t mGraphicsQueueFamily = 0;
	uint32_t mPresentQueueFamily = 0;
	bool mFrameStarted = false;
	bool mVsync = false;
	float mClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	bool mInitialized = false;

	static constexpr uint32_t MaxDrawsPerFrame = 512;
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
	std::array<VkDescriptorPool, MaxFramesInFlight> mDescriptorPools{};
	std::array<VkBuffer, MaxFramesInFlight> mUniformBuffers{};
	std::array<VmaAllocation, MaxFramesInFlight> mUniformAllocations{};
	std::array<void*, MaxFramesInFlight> mUniformMapped{};
	VkDeviceSize mUniformStride = 0;
	uint32_t mDrawIndex = 0;
	uint64_t mFrameSerial = 0;
	std::unordered_map<const VulkanShader*, VkPipeline> mPipelines;
	const VulkanShader* mBoundShader = nullptr;
	std::array<const VulkanTexture*, MaxTextureSlots> mBoundTextures{};
	int mViewport[4] = {0, 0, 0, 0};
	uint32_t mWindowWidth = 0;
	uint32_t mWindowHeight = 0;
};
