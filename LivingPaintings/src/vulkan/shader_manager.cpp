// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shader_manager.h"
#include "../config.hpp"
using namespace std;

map<VkShaderModule, VkShaderStageFlagBits> shaderModules {};

const map<string, VkShaderStageFlagBits> shaderTypes {
    { ".vert", VK_SHADER_STAGE_VERTEX_BIT },
    { ".frag", VK_SHADER_STAGE_FRAGMENT_BIT },
    { ".comp", VK_SHADER_STAGE_COMPUTE_BIT },
    { ".geom", VK_SHADER_STAGE_GEOMETRY_BIT },
    { ".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT },
    { ".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT }
};

void ShaderManager::createShaderModules(VkDevice& device)
{
    auto shaderPath = RETRIEVE_STRING(RESOURCE_SHADER_PATH);
    shaderCompiler.compileIfChanged(shaderPath);

    for (const auto& shader : shaderCompiler.getCompiledShaders()) {
        auto shaderModule = ShaderManager::createShaderModule(device, shader.second);
        auto spvExt = 4;
        auto shaderExt = 5;
        auto index = shader.first.size() - spvExt;
        auto ext = shader.first.substr(index - shaderExt, shaderExt);
        shaderModules.insert({ shaderModule, shaderTypes.find(ext)->second });
    }
}

VkShaderModule ShaderManager::createShaderModule(VkDevice& device, vector<char> shaderCode)
{

    VkShaderModuleCreateInfo shaderModuleInfo {};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw runtime_error("Failed to create shader module.");
    }

    return shaderModule;
}

void ShaderManager::destroyShaderModules(VkDevice& device)
{
    for (const auto& shaderModule : shaderModules) {
        vkDestroyShaderModule(device, shaderModule.first, nullptr);
    }
}

map<VkShaderModule, VkShaderStageFlagBits> ShaderManager::getShaderModules()
{
    return shaderModules;
}
