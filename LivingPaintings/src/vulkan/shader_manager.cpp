// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shader_manager.h"

using namespace std;

const char* shaderPath = RETRIEVE_STRING(RESOURCE_SHADER_PATH);

const int8_t spvExtNameLength = 4;
const int8_t shaderExtNameLength = 5;

VkDevice ShaderManager::device = VK_NULL_HANDLE;
bool ShaderManager::recreateGraphicsPipeline = false;

const std::map<string, VkShaderStageFlagBits> shaderTypes {
    { ".vert", VK_SHADER_STAGE_VERTEX_BIT },
    { ".frag", VK_SHADER_STAGE_FRAGMENT_BIT },
    { ".comp", VK_SHADER_STAGE_COMPUTE_BIT },
    { ".geom", VK_SHADER_STAGE_GEOMETRY_BIT },
    { ".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT },
    { ".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT }
};

std::map<VkShaderStageFlagBits, VkShaderModule> shaderModules {};

HANDLE hShaderFileChange;
DWORD dwWaitStatus;
std::thread fileChangeNotifyThread(ShaderManager::notifyShaderFileChange);

ShaderManager::~ShaderManager()
{
    if (fileChangeNotifyThread.joinable()) {
        fileChangeNotifyThread.join();
    }
}

void ShaderManager::notifyShaderFileChange()
{
    cout << "\n[Shader Notifier] Started. Waiting for change notification...\n";

    while (TRUE) {

        hShaderFileChange = FindFirstChangeNotificationA(
            shaderPath, false, FILE_NOTIFY_CHANGE_LAST_WRITE);

        dwWaitStatus = WaitForSingleObject(hShaderFileChange, INFINITE);
        std::this_thread::sleep_for(100ms);
        if (FindNextChangeNotification(hShaderFileChange)) {
            cout << "\n[Shader Notifier] Shader files directory changed.\n";
            recreateGraphicsPipeline = true;
            bool gettingNotified = true;
            while (gettingNotified) {
                hShaderFileChange = FindFirstChangeNotificationA(
                    shaderPath, false, FILE_NOTIFY_CHANGE_LAST_WRITE);

                dwWaitStatus = WaitForSingleObject(hShaderFileChange, 1000);
                if (FindNextChangeNotification(hShaderFileChange) == FALSE || dwWaitStatus == WAIT_TIMEOUT) {
                    gettingNotified = false;
                }
            }
            FindCloseChangeNotification(hShaderFileChange);
        } else {
            cout << "\n[Shader Notifier] Stopping. Failed to notify.\n";
            FindCloseChangeNotification(hShaderFileChange);
            break;
        }
    }
}

void ShaderManager::createShaderModules(VkDevice& device)
{
    ShaderManager::device = device;

    ShaderCompiler::compileIfChanged(shaderPath);

    for (std::pair<std::string, std::vector<char>> shader : ShaderCompiler::getCompiledShaders()) {
        VkShaderModule shaderModule = createShaderModule(shader.second);
        unsigned long index = shader.first.size() - spvExtNameLength;
        std::string ext = shader.first.substr(index - shaderExtNameLength,
            shaderExtNameLength);
        VkShaderStageFlagBits shaderType = shaderTypes.find(ext)->second;
        if (shaderModules.contains(shaderType)) {
            shaderModules[shaderType] = shaderModule;
        } else {
            shaderModules.insert({ shaderType, shaderModule });
        }
    }
}

VkShaderModule ShaderManager::createShaderModule(vector<char> shaderCode)
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

void ShaderManager::destroyShaderModules()
{
    for (const std::pair<const VkShaderStageFlagBits, VkShaderModule_T*>& shaderModule : shaderModules) {
        vkDestroyShaderModule(device, shaderModule.second, nullptr);
    }
}

map<VkShaderStageFlagBits, VkShaderModule> ShaderManager::getShaderModules()
{
    return shaderModules;
}
