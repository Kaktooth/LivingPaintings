// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "consts.h"
#include "shader_compiler.h"
#include <vulkan/vulkan_core.h>

class ShaderManager {

    ShaderCompiler shaderCompiler;

    VkShaderModule createShaderModule(VkDevice& device, std::vector<char> shaderCode);

public:
    void createShaderModules(VkDevice& device);
    void destroyShaderModules(VkDevice& device);
    std::map<VkShaderModule, VkShaderStageFlagBits> getShaderModules();
};
