#include "Renderer/Vulkan/VulkanRenderDevice.h"

#include "IWindow.h"
#include "Renderer/IVertexArray.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanShader.h"
#include "Renderer/Vulkan/VulkanTexture.h"
#include "Renderer/Vulkan/VulkanVertexArray.h"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_vulkan.h"
#include "Utils/Log.h"

#include <algorithm>
#include <cstring>
#include <optional>

namespace {
	bool Check(VkResult result, const char* operation) {
		if (result == VK_SUCCESS)
			return true;
		LOG_ERROR("Vulkan {} failed with error {}", operation, static_cast<int>(result));
		return false;
	}

	VkFormat ToVkFormat(BufferElementType type) {
		switch (type) {
		case BufferElementType::Float: return VK_FORMAT_R32_SFLOAT;
		case BufferElementType::Float2: return VK_FORMAT_R32G32_SFLOAT;
		case BufferElementType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
		case BufferElementType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case BufferElementType::Int:
		case BufferElementType::Bool: return VK_FORMAT_R32_SINT;
		case BufferElementType::Int2: return VK_FORMAT_R32G32_SINT;
		case BufferElementType::Int3: return VK_FORMAT_R32G32B32_SINT;
		case BufferElementType::Int4: return VK_FORMAT_R32G32B32A32_SINT;
		default: return VK_FORMAT_UNDEFINED;
		}
	}
}

VulkanRenderDevice::~VulkanRenderDevice() {
	Shutdown();
}

bool VulkanRenderDevice::Init(IWindow* window) {
	if (!window || !window->GetHandle()) {
		LOG_ERROR("Cannot initialize Vulkan without a valid window");
		return false;
	}
	if (!Check(volkInitialize(), "loader initialization"))
		return false;

	SDL_Window* sdlWindow = static_cast<SDL_Window*>(window->GetHandle());
	mWindowWidth = window->GetWidth();
	mWindowHeight = window->GetHeight();
	Uint32 extensionCount = 0;
	const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
	if (!extensions || extensionCount == 0) {
		LOG_ERROR("Failed to query Vulkan instance extensions: {}", SDL_GetError());
		return false;
	}

	std::vector<const char*> instanceExtensions(extensions, extensions + extensionCount);
	VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = "2dTacticsGame";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "2dTacticsGame";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;
	VkInstanceCreateInfo instanceInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
	if (!Check(vkCreateInstance(&instanceInfo, nullptr, &mInstance), "instance creation"))
		return false;
	volkLoadInstance(mInstance);

	if (!SDL_Vulkan_CreateSurface(sdlWindow, mInstance, nullptr, &mSurface)) {
		LOG_ERROR("Vulkan surface creation failed: {}", SDL_GetError());
		return false;
	}

	uint32_t physicalDeviceCount = 0;
	if (!Check(vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr),
		"physical device enumeration") || physicalDeviceCount == 0) {
		LOG_ERROR("No Vulkan physical devices are available");
		return false;
	}
	std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, devices.data());

	for (VkPhysicalDevice device : devices) {
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
		std::vector<VkQueueFamilyProperties> families(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		for (uint32_t i = 0; i < familyCount; ++i) {
			if (families[i].queueCount && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				graphicsFamily = i;
			VkBool32 present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &present);
			if (families[i].queueCount && present)
				presentFamily = i;
		}
		if (graphicsFamily && presentFamily) {
			mPhysicalDevice = device;
			mGraphicsQueueFamily = *graphicsFamily;
			mPresentQueueFamily = *presentFamily;
			break;
		}
	}
	if (mPhysicalDevice == VK_NULL_HANDLE) {
		LOG_ERROR("No Vulkan device supports graphics and presentation");
		return false;
	}

	float priority = 1.0f;
	std::array<VkDeviceQueueCreateInfo, 2> queues{};
	uint32_t queueCount = 1;
	queues[0] = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
		mGraphicsQueueFamily, 1, &priority};
	if (mPresentQueueFamily != mGraphicsQueueFamily) {
		queues[1] = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
			mPresentQueueFamily, 1, &priority};
		queueCount = 2;
	}
	const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	deviceInfo.queueCreateInfoCount = queueCount;
	deviceInfo.pQueueCreateInfos = queues.data();
	deviceInfo.enabledExtensionCount = 1;
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;
	if (!Check(vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice),
		"logical device creation"))
		return false;
	volkLoadDevice(mDevice);
	vkGetDeviceQueue(mDevice, mGraphicsQueueFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, mPresentQueueFamily, 0, &mPresentQueue);
	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.instance = mInstance;
	allocatorInfo.physicalDevice = mPhysicalDevice;
	allocatorInfo.device = mDevice;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;
	if (vmaCreateAllocator(&allocatorInfo, &mAllocator) != VK_SUCCESS) {
		LOG_ERROR("Failed to create Vulkan memory allocator");
		return false;
	}

	VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = mGraphicsQueueFamily;
	if (!Check(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool),
		"command pool creation") || !CreateSwapchain())
		return false;

	for (uint32_t i = 0; i < mImageAvailableSemaphores.size(); ++i) {
		VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
		VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (!Check(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]),
			"semaphore creation") ||
			!Check(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]),
				"semaphore creation") ||
			!Check(vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]),
				"fence creation"))
			return false;
	}
	if (!CreateDrawResources())
		return false;
	mInitialized = true;
	return true;
}

void VulkanRenderDevice::Shutdown() {
	if (mDevice) {
		vkDeviceWaitIdle(mDevice);
		DestroyDrawResources();
		for (size_t i = 0; i < mImageAvailableSemaphores.size(); ++i) {
			vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
		}
		DestroySwapchain();
		vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
		vmaDestroyAllocator(mAllocator);
		mAllocator = VK_NULL_HANDLE;
		vkDestroyDevice(mDevice, nullptr);
	}
	if (mSurface && mInstance)
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	if (mInstance)
		vkDestroyInstance(mInstance, nullptr);
	mInstance = VK_NULL_HANDLE;
	mSurface = VK_NULL_HANDLE;
	mPhysicalDevice = VK_NULL_HANDLE;
	mDevice = VK_NULL_HANDLE;
	mCommandPool = VK_NULL_HANDLE;
	mInitialized = false;
	mFrameStarted = false;
}

void VulkanRenderDevice::EnableVsync(bool enable) {
	if (mVsync == enable)
		return;
	mVsync = enable;
	if (mInitialized) {
		vkDeviceWaitIdle(mDevice);
		DestroySwapchain();
		if (!CreateSwapchain())
			LOG_ERROR("Failed to recreate Vulkan swapchain after changing Vsync");
	}
}

void VulkanRenderDevice::EnableBlending(bool enable) {
	(void)enable;
}

void VulkanRenderDevice::EnableDepthTest(bool enable) {
	(void)enable;
}

void VulkanRenderDevice::SetViewport(int x, int y, int width, int height) {
	mViewport[0] = x;
	mViewport[1] = y;
	mViewport[2] = width;
	mViewport[3] = height;
}

void VulkanRenderDevice::SetClearColor(float r, float g, float b, float a) {
	mClearColor[0] = r;
	mClearColor[1] = g;
	mClearColor[2] = b;
	mClearColor[3] = a;
}

void VulkanRenderDevice::Clear(ClearFlags flags) {
	if (!mInitialized || !BeginFrame())
		return;
	VkClearValue clearValues[2]{};
	std::memcpy(clearValues[0].color.float32, mClearColor, sizeof(mClearColor));
	clearValues[1].depthStencil = {1.0f, 0};
	VkRenderPassBeginInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	renderPassInfo.renderPass = mRenderPass;
	renderPassInfo.framebuffer = mFramebuffers[mImageIndex];
	renderPassInfo.renderArea.extent = mSwapchainExtent;
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;
	vkCmdBeginRenderPass(mCommandBuffers[mImageIndex], &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);
	(void)flags;
}

void VulkanRenderDevice::DrawIndexed(
	IVertexArray* vertexArray,
	uint32_t indexCount,
	DrawIndexSize size) {
	(void)size;
	const VkCommandBuffer command = GetCurrentCommandBuffer();
	if (!command || !vertexArray || !mBoundShader)
		return;
	const uint32_t count = indexCount ? indexCount : vertexArray->GetIndexCount();
	const auto* vertexBuffer = static_cast<VulkanVertexArray*>(vertexArray)->GetVertexBuffer();
	const VulkanTexture* fallback = mBoundTextures[0];
	if (count == 0 || !vertexBuffer || !fallback || mDrawIndex >= MaxDrawsPerFrame) {
		if (mDrawIndex >= MaxDrawsPerFrame)
			LOG_ERROR("Exceeded the maximum number of Vulkan draws per frame");
		mBoundTextures.fill(nullptr);
		return;
	}
	const VkPipeline pipeline = GetPipeline(*mBoundShader, vertexBuffer->GetLayout());
	if (!pipeline) {
		mBoundTextures.fill(nullptr);
		return;
	}

	VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocateInfo.descriptorPool = mDescriptorPools[mCurrentFrame];
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &mDescriptorSetLayout;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	if (!Check(vkAllocateDescriptorSets(mDevice, &allocateInfo, &descriptorSet),
		"descriptor set allocation")) {
		mBoundTextures.fill(nullptr);
		return;
	}

	const VkDeviceSize uniformOffset = mUniformStride * mDrawIndex++;
	auto* uniformData = static_cast<char*>(mUniformMapped[mCurrentFrame]) + uniformOffset;
	std::memcpy(uniformData, &mBoundShader->GetViewProjection(), sizeof(glm::mat4));
	vmaFlushAllocation(mAllocator, mUniformAllocations[mCurrentFrame], uniformOffset,
		sizeof(glm::mat4));

	// Every sampler in the array must be valid, so unused slots reuse the white texture
	std::array<VkDescriptorImageInfo, MaxTextureSlots> imageInfos{};
	for (uint32_t i = 0; i < MaxTextureSlots; ++i) {
		const VulkanTexture* texture = mBoundTextures[i] ? mBoundTextures[i] : fallback;
		imageInfos[i] = {texture->GetSampler(), texture->GetImageView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
	}
	VkDescriptorBufferInfo bufferInfo{mUniformBuffers[mCurrentFrame], 0, sizeof(glm::mat4)};
	VkWriteDescriptorSet writes[2]{};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = descriptorSet;
	writes[0].dstBinding = 0;
	writes[0].descriptorCount = MaxTextureSlots;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[0].pImageInfo = imageInfos.data();
	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = descriptorSet;
	writes[1].dstBinding = 1;
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writes[1].pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(mDevice, 2, writes, 0, nullptr);
	mBoundTextures.fill(nullptr);

	vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkViewport viewport{static_cast<float>(mViewport[0]), static_cast<float>(mViewport[1]),
		static_cast<float>(mViewport[2]), static_cast<float>(mViewport[3]), 0.0f, 1.0f};
	if (mViewport[2] <= 0 || mViewport[3] <= 0) {
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mSwapchainExtent.width);
		viewport.height = static_cast<float>(mSwapchainExtent.height);
	}
	vkCmdSetViewport(command, 0, 1, &viewport);
	VkRect2D scissor{{0, 0}, mSwapchainExtent};
	vkCmdSetScissor(command, 0, 1, &scissor);
	const uint32_t dynamicOffset = static_cast<uint32_t>(uniformOffset);
	vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
		&descriptorSet, 1, &dynamicOffset);
	vertexArray->Bind();
	vkCmdDrawIndexed(command, count, 1, 0, 0, 0);
}

void VulkanRenderDevice::BindTexture(uint32_t slot, const VulkanTexture* texture) {
	if (slot < MaxTextureSlots)
		mBoundTextures[slot] = texture;
}

void VulkanRenderDevice::OnShaderDestroyed(const VulkanShader* shader) {
	if (mBoundShader == shader)
		mBoundShader = nullptr;
	const auto it = mPipelines.find(shader);
	if (it == mPipelines.end())
		return;
	if (mDevice) {
		vkDeviceWaitIdle(mDevice);
		vkDestroyPipeline(mDevice, it->second, nullptr);
	}
	mPipelines.erase(it);
}

bool VulkanRenderDevice::CreateDrawResources() {
	VkDescriptorSetLayoutBinding bindings[2]{};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].descriptorCount = MaxTextureSlots;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = bindings;
	if (!Check(vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout),
		"descriptor set layout creation"))
		return false;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
	if (!Check(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout),
		"pipeline layout creation"))
		return false;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
	const VkDeviceSize alignment = std::max<VkDeviceSize>(
		properties.limits.minUniformBufferOffsetAlignment, 1);
	mUniformStride = (sizeof(glm::mat4) + alignment - 1) / alignment * alignment;

	for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			MaxDrawsPerFrame * MaxTextureSlots};
		poolSizes[1] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MaxDrawsPerFrame};
		VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		poolInfo.maxSets = MaxDrawsPerFrame;
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		if (!Check(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPools[i]),
			"descriptor pool creation"))
			return false;
		if (!VulkanMemory::CreateBuffer(mAllocator, mUniformStride * MaxDrawsPerFrame,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
			mUniformBuffers[i], mUniformAllocations[i])) {
			LOG_ERROR("Failed to create Vulkan uniform buffer");
			return false;
		}
		if (!Check(vmaMapMemory(mAllocator, mUniformAllocations[i], &mUniformMapped[i]),
			"uniform buffer mapping"))
			return false;
	}
	return true;
}

void VulkanRenderDevice::DestroyDrawResources() {
	for (auto& [shader, pipeline] : mPipelines)
		vkDestroyPipeline(mDevice, pipeline, nullptr);
	mPipelines.clear();
	for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
		if (mDescriptorPools[i])
			vkDestroyDescriptorPool(mDevice, mDescriptorPools[i], nullptr);
		if (mUniformBuffers[i]) {
			if (mUniformMapped[i])
				vmaUnmapMemory(mAllocator, mUniformAllocations[i]);
			vmaDestroyBuffer(mAllocator, mUniformBuffers[i], mUniformAllocations[i]);
		}
		mDescriptorPools[i] = VK_NULL_HANDLE;
		mUniformBuffers[i] = VK_NULL_HANDLE;
		mUniformAllocations[i] = VK_NULL_HANDLE;
		mUniformMapped[i] = nullptr;
	}
	if (mPipelineLayout)
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	if (mDescriptorSetLayout)
		vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
	mPipelineLayout = VK_NULL_HANDLE;
	mDescriptorSetLayout = VK_NULL_HANDLE;
	mBoundShader = nullptr;
}

VkPipeline VulkanRenderDevice::GetPipeline(const VulkanShader& shader, const BufferLayout& layout) {
	if (const auto it = mPipelines.find(&shader); it != mPipelines.end())
		return it->second;

	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = shader.GetVertexModule();
	stages[0].pName = "vertexMain";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = shader.GetFragmentModule();
	stages[1].pName = "fragmentMain";

	VkVertexInputBindingDescription binding{0, layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX};
	std::vector<VkVertexInputAttributeDescription> attributes;
	uint32_t location = 0;
	for (const auto& element : layout)
		attributes.push_back({location++, 0, ToVkFormat(element.type), element.offset});
	VkPipelineVertexInputStateCreateInfo vertexInput{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &binding;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	vertexInput.pVertexAttributeDescriptions = attributes.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewportState{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterization{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterization.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization.cullMode = VK_CULL_MODE_NONE;
	rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = VK_TRUE;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo colorBlend{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	colorBlend.attachmentCount = 1;
	colorBlend.pAttachments = &blendAttachment;

	const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState{
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	info.stageCount = 2;
	info.pStages = stages;
	info.pVertexInputState = &vertexInput;
	info.pInputAssemblyState = &inputAssembly;
	info.pViewportState = &viewportState;
	info.pRasterizationState = &rasterization;
	info.pMultisampleState = &multisample;
	info.pDepthStencilState = &depthStencil;
	info.pColorBlendState = &colorBlend;
	info.pDynamicState = &dynamicState;
	info.layout = mPipelineLayout;
	info.renderPass = mRenderPass;
	VkPipeline pipeline = VK_NULL_HANDLE;
	if (!Check(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline),
		"graphics pipeline creation"))
		return VK_NULL_HANDLE;
	mPipelines.emplace(&shader, pipeline);
	return pipeline;
}

void VulkanRenderDevice::Present() {
	if (!mFrameStarted)
		return;
	EndFrame();
}

ITexture* VulkanRenderDevice::CreateTexture() const {
	return new VulkanTexture(const_cast<VulkanRenderDevice*>(this));
}

IShader* VulkanRenderDevice::CreateShader() const {
	return new VulkanShader(const_cast<VulkanRenderDevice*>(this));
}

IVertexArray* VulkanRenderDevice::CreateVertexArray() const {
	return new VulkanVertexArray(const_cast<VulkanRenderDevice*>(this));
}

IVertexBuffer* VulkanRenderDevice::CreateVertexBuffer() const {
	return new VulkanVertexBuffer(const_cast<VulkanRenderDevice*>(this));
}

IIndexBuffer* VulkanRenderDevice::CreateIndexBuffer() const {
	return new VulkanIndexBuffer(const_cast<VulkanRenderDevice*>(this));
}

bool VulkanRenderDevice::ExecuteImmediate(
	const std::function<void(VkCommandBuffer)>& command) const {
	if (!mDevice || !mCommandPool || !mGraphicsQueue)
		return false;
	VkCommandBufferAllocateInfo allocation{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocation.commandPool = mCommandPool;
	allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation.commandBufferCount = 1;
	VkCommandBuffer buffer = VK_NULL_HANDLE;
	if (!Check(vkAllocateCommandBuffers(mDevice, &allocation, &buffer), "upload command allocation"))
		return false;
	VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (!Check(vkBeginCommandBuffer(buffer, &begin), "upload command begin")) {
		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &buffer);
		return false;
	}
	command(buffer);
	if (!Check(vkEndCommandBuffer(buffer), "upload command end")) {
		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &buffer);
		return false;
	}
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &buffer;
	const bool success = Check(vkQueueSubmit(mGraphicsQueue, 1, &submit, VK_NULL_HANDLE),
		"upload submission") && Check(vkQueueWaitIdle(mGraphicsQueue), "upload completion");
	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &buffer);
	return success;
}

bool VulkanRenderDevice::BeginFrame() {
	if (mFrameStarted)
		return true;
	VkFence fence = mInFlightFences[mCurrentFrame];
	if (!Check(vkWaitForFences(mDevice, 1, &fence, VK_TRUE, UINT64_MAX), "fence wait"))
		return false;
	vkResetDescriptorPool(mDevice, mDescriptorPools[mCurrentFrame], 0);
	mDrawIndex = 0;
	++mFrameSerial;
	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX,
		mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return false;
	if (result != VK_SUBOPTIMAL_KHR && !Check(result, "swapchain image acquisition"))
		return false;
	mImageIndex = imageIndex;
	if (mImagesInFlight[mImageIndex] != VK_NULL_HANDLE &&
		!Check(vkWaitForFences(mDevice, 1, &mImagesInFlight[mImageIndex], VK_TRUE, UINT64_MAX),
			"swapchain image fence wait"))
		return false;
	mImagesInFlight[mImageIndex] = fence;
	if (!Check(vkResetFences(mDevice, 1, &fence), "fence reset") ||
		!Check(vkResetCommandBuffer(mCommandBuffers[mImageIndex], 0), "command buffer reset"))
		return false;
	VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	if (!Check(vkBeginCommandBuffer(mCommandBuffers[mImageIndex], &beginInfo),
		"command buffer begin"))
		return false;
	mFrameStarted = true;
	return true;
}

void VulkanRenderDevice::EndFrame() {
	vkCmdEndRenderPass(mCommandBuffers[mImageIndex]);
	if (!Check(vkEndCommandBuffer(mCommandBuffers[mImageIndex]), "command buffer end")) {
		mFrameStarted = false;
		return;
	}
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &mImageAvailableSemaphores[mCurrentFrame];
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[mImageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &mRenderFinishedSemaphores[mCurrentFrame];
	if (Check(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]),
		"queue submission")) {
		VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &mRenderFinishedSemaphores[mCurrentFrame];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mSwapchain;
		presentInfo.pImageIndices = &mImageIndex;
		const VkResult presentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);
		if (presentResult != VK_SUBOPTIMAL_KHR && presentResult != VK_ERROR_OUT_OF_DATE_KHR)
			Check(presentResult, "presentation");
	}
	mFrameStarted = false;
	mCurrentFrame = (mCurrentFrame + 1) % MaxFramesInFlight;
}

bool VulkanRenderDevice::CreateSwapchain() {
	VkSurfaceCapabilitiesKHR capabilities{};
	if (!Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities),
		"surface capability query"))
		return false;
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
	if (formatCount == 0)
		return false;
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, formats.data());
	auto format = formats.front();
	for (const auto& candidate : formats)
		if (candidate.format == VK_FORMAT_B8G8R8A8_UNORM)
			format = candidate;
	mSwapchainFormat = format.format;
	mSwapchainExtent = capabilities.currentExtent;
	if (mSwapchainExtent.width == UINT32_MAX) {
		mSwapchainExtent.width = std::clamp<uint32_t>(mWindowWidth, capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		mSwapchainExtent.height = std::clamp<uint32_t>(mWindowHeight, capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);
	}
	uint32_t imageCount = std::max(capabilities.minImageCount + 1, 2u);
	if (capabilities.maxImageCount)
		imageCount = std::min(imageCount, capabilities.maxImageCount);
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!mVsync) {
		uint32_t modeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &modeCount, nullptr);
		std::vector<VkPresentModeKHR> modes(modeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &modeCount, modes.data());
		if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != modes.end())
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	info.surface = mSurface;
	info.minImageCount = imageCount;
	info.imageFormat = mSwapchainFormat;
	info.imageColorSpace = format.colorSpace;
	info.imageExtent = mSwapchainExtent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t families[] = {mGraphicsQueueFamily, mPresentQueueFamily};
	if (mGraphicsQueueFamily != mPresentQueueFamily) {
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = families;
	} else {
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	info.preTransform = capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = presentMode;
	info.clipped = VK_TRUE;
	if (!Check(vkCreateSwapchainKHR(mDevice, &info, nullptr, &mSwapchain), "swapchain creation"))
		return false;
	uint32_t actualCount = 0;
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &actualCount, nullptr);
	mSwapchainImages.resize(actualCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &actualCount, mSwapchainImages.data());
	mSwapchainImageViews.resize(actualCount);
	for (size_t i = 0; i < mSwapchainImages.size(); ++i) {
		VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		viewInfo.image = mSwapchainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = mSwapchainFormat;
		viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		if (!Check(vkCreateImageView(mDevice, &viewInfo, nullptr, &mSwapchainImageViews[i]),
			"image view creation"))
			return false;
	}
	VkAttachmentDescription attachment{};
	attachment.format = mSwapchainFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference reference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

	VkImageCreateInfo depthInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	depthInfo.imageType = VK_IMAGE_TYPE_2D;
	depthInfo.extent = {mSwapchainExtent.width, mSwapchainExtent.height, 1};
	depthInfo.mipLevels = 1;
	depthInfo.arrayLayers = 1;
	depthInfo.format = VK_FORMAT_D32_SFLOAT;
	depthInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	VmaAllocationCreateInfo depthAllocation{};
	depthAllocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (!Check(vmaCreateImage(mAllocator, &depthInfo, &depthAllocation, &mDepthImage,
		&mDepthAllocation, nullptr), "depth image creation"))
		return false;
	VkImageViewCreateInfo depthViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	depthViewInfo.image = mDepthImage;
	depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthViewInfo.format = VK_FORMAT_D32_SFLOAT;
	depthViewInfo.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
	if (!Check(vkCreateImageView(mDevice, &depthViewInfo, nullptr, &mDepthView),
		"depth image view creation"))
		return false;

	VkAttachmentDescription attachments[2]{};
	attachments[0] = attachment;
	attachments[1].format = VK_FORMAT_D32_SFLOAT;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentReference depthReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &reference;
	subpass.pDepthStencilAttachment = &depthReference;
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = dependency.srcStageMask;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	if (!Check(vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass),
		"render pass creation"))
		return false;
	mFramebuffers.resize(mSwapchainImageViews.size());
	mImagesInFlight.assign(mSwapchainImageViews.size(), VK_NULL_HANDLE);
	for (size_t i = 0; i < mFramebuffers.size(); ++i) {
		VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferInfo.renderPass = mRenderPass;
		const VkImageView framebufferViews[] = {mSwapchainImageViews[i], mDepthView};
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = framebufferViews;
		framebufferInfo.width = mSwapchainExtent.width;
		framebufferInfo.height = mSwapchainExtent.height;
		framebufferInfo.layers = 1;
		if (!Check(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mFramebuffers[i]),
			"framebuffer creation"))
			return false;
	}
	mCommandBuffers.resize(mFramebuffers.size());
	VkCommandBufferAllocateInfo allocationInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocationInfo.commandPool = mCommandPool;
	allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocationInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());
	return Check(vkAllocateCommandBuffers(mDevice, &allocationInfo, mCommandBuffers.data()),
		"command buffer allocation");
}

void VulkanRenderDevice::DestroySwapchain() {
	for (auto framebuffer : mFramebuffers)
		vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
	if (mRenderPass)
		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	if (mDepthView)
		vkDestroyImageView(mDevice, mDepthView, nullptr);
	if (mDepthImage)
		vmaDestroyImage(mAllocator, mDepthImage, mDepthAllocation);
	mDepthView = VK_NULL_HANDLE;
	mDepthImage = VK_NULL_HANDLE;
	mDepthAllocation = VK_NULL_HANDLE;
	for (auto view : mSwapchainImageViews)
		vkDestroyImageView(mDevice, view, nullptr);
	if (mSwapchain)
		vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
	mFramebuffers.clear();
	mSwapchainImageViews.clear();
	mSwapchainImages.clear();
	mCommandBuffers.clear();
	mImagesInFlight.clear();
	mRenderPass = VK_NULL_HANDLE;
	mSwapchain = VK_NULL_HANDLE;
}
