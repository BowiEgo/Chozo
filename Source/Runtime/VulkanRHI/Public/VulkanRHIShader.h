#pragma once

#include "RHIShader.h"

class VULKAN_RHI_API CVulkanRHIShader : public IRHIShader {
public:
    CVulkanRHIShader() = default;
    virtual ~CVulkanRHIShader() = default;

    virtual EShaderStage GetStage() const override {
        return EShaderStage::None;
    };
    virtual const std::string &GetName() const override {
        std::string name("");
        return name;
    };

private:
    vk::raii::ShaderModule m_VertexModule{nullptr};
    vk::raii::ShaderModule m_FragmentModule{nullptr};
};