#pragma once
#include "consts.h"
#include "shader_compiler.h"
#include <iostream>
#include <thread>
#include <vulkan/vulkan_core.h>
#include <windows.h>

class ShaderManager {

    static VkDevice device;

    static VkShaderModule createShaderModule(std::vector<char> shaderCode);

public:
    ~ShaderManager();
    static bool recreateGraphicsPipeline;
    static void notifyShaderFileChange();
    void createShaderModules(VkDevice& device);
    void destroyShaderModules();
    std::map<VkShaderStageFlagBits, VkShaderModule> getShaderModules();
};
