#include "shader_compiler.h"
#include "../utils/path_params.hpp"

using Runtime::PATH_PARAMS;

const std::map<std::string, shaderc_shader_kind> shaderTypes {
    { ".vert", shaderc_glsl_vertex_shader },
    { ".frag", shaderc_glsl_fragment_shader },
    { ".comp", shaderc_glsl_compute_shader },
    { ".geom", shaderc_glsl_geometry_shader },
    { ".tesc", shaderc_glsl_tess_control_shader },
    { ".tese", shaderc_glsl_tess_evaluation_shader }
};

std::map<std::string, std::vector<char>> compiledShaders{};

void ShaderCompiler::compileIfChanged()
{
    const std::map<std::string, std::filesystem::directory_entry> shaderEntries = listModifiedFiles();
    for (const auto shaderEntry : shaderEntries) {
        std::filesystem::path shaderPath = shaderEntry.second.path();
        std::string filename = shaderEntry.first + ".spv";
        std::string spvPath = shaderPath.string() + ".spv";
        std::vector<char> buffer = readShaderFile(shaderPath.string(), false);
        compile(shaderPath, buffer.data());
        std::cout << "Shader compiled: " + shaderPath.string();

        const std::vector<char> compiledShader = readShaderFile(spvPath, true);
        if (compiledShaders.contains(filename)) {
            compiledShaders[filename] = compiledShader;
        } else {
            compiledShaders.insert({ filename, compiledShader });
        }
    }
}

void ShaderCompiler::compile(std::filesystem::path shaderPath, const std::string& shaderCode)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
    const char* filename = reinterpret_cast<const char*>(shaderPath.filename().c_str());
    const std::string ext = shaderPath.extension().string();
    std::string path = std::filesystem::absolute(shaderPath).string();
    shaderc::SpvCompilationResult shaderModule = compiler.CompileGlslToSpv(shaderCode, shaderTypes.find(ext)->second, filename, options);

    if (shaderModule.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << shaderModule.GetErrorMessage();
        throw std::runtime_error("Shader Compilation Error: " + shaderModule.GetErrorMessage());
    }
    std::vector<uint32_t> compiled { shaderModule.cbegin(), shaderModule.cend() };

    const char* data = reinterpret_cast<const char*>(compiled.data());
    std::ofstream shaderFile(path + ".spv", std::ios::ate | std::ios::binary);
    shaderFile.clear();
    shaderFile.write(data, strlen(data) * compiled.size());
    shaderFile.close();
}

std::map<std::string, std::filesystem::directory_entry> ShaderCompiler::listModifiedFiles()
{
    std::map<std::string, std::filesystem::directory_entry> filePaths {};
    std::map<std::string, std::filesystem::directory_entry> compilePaths {};
    std::filesystem::path rootPath(PATH_PARAMS.SHADER_PATH);

    auto dit = std::filesystem::directory_iterator { rootPath };
    for (const std::filesystem::directory_entry& entry : dit) {
        if (entry.is_regular_file()) {
            filePaths.insert({ entry.path().filename().string(), entry });
        }
    }

    for (const auto& entry : filePaths) {
        bool spvFile = entry.second.path().extension() == ".spv";

        if (!spvFile) {
            std::string spv = entry.first + ".spv";

            if (filePaths.contains(spv) && entry.second.last_write_time() < filePaths[spv].last_write_time()) {
                std::string spvPath = entry.second.path().string() + ".spv";
                std::vector<char> compiledShader = readShaderFile(spvPath, true);
                compiledShaders.insert({ spv, compiledShader });
            } else {
                compilePaths.insert(entry);
            }
        }
    }
    return compilePaths;
}

std::vector<char> ShaderCompiler::readShaderFile(const std::string& shaderPath, bool spvShader)
{
    std::ifstream shaderFile(shaderPath, std::ios::ate | std::ios::binary);

    if (!shaderFile.is_open()) {
        throw std::runtime_error("Failed to open shader file.");
    }

    size_t fileSize = static_cast<size_t>(shaderFile.tellg());
    std::vector<char> buffer(fileSize + !spvShader);
    shaderFile.seekg(0);
    shaderFile.read(buffer.data(), fileSize);
    shaderFile.close();

    if (!spvShader) {
        buffer[fileSize] = '\0';
    }

    return buffer;
}

std::map<std::string, std::vector<char>> ShaderCompiler::getCompiledShaders()
{
    return compiledShaders;
}
