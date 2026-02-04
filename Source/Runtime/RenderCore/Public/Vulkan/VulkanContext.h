#pragma once

#include "GraphicsContext.h"

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

class RENDERCORE_API CVulkanContext : public CGraphicsContext {
public:
    CVulkanContext(IRendererWindow *windowHandle);
    ~CVulkanContext() {};

    virtual void Init() override;
    virtual void SwapBuffers() override;

    virtual void CreateRenderer() override { // TODO: Remove
        CreateImageViews();
        // CreateGraphicsPipeline();
        // CreateCommandPool();
        // CreateVertexBuffer();
        // CreateCommandBuffers();
        // CreateSyncObjects();
    }

    vk::raii::Device &GetDevice() { return m_LogicalDevice; }

private:
    void CreateVKInstance();
    void SetupDebugMessenger();
    void CreateVKSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();

public:
    void CreateImageViews();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateVertexBuffer();
    void CreateCommandBuffers();
    void CreateSyncObjects();

private:
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
    vk::raii::SurfaceKHR m_Surface = nullptr;

    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_LogicalDevice = nullptr;

    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::Queue m_ComputeQueue = nullptr;

    vk::raii::SwapchainKHR m_Swapchain = nullptr;
    std::vector<vk::Image> m_SwapchainImages;
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    vk::Format m_SwapchainImageFormat;
    vk::Extent2D m_SwapchainExtent;
};