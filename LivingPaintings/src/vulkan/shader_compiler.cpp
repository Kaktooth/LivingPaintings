// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shader_compiler.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

map<string, const vector<char>> compiledShaders {};

const map<string, shaderc_shader_kind> shaderTypes {
    { ".vert", shaderc_glsl_vertex_shader },
    { ".frag", shaderc_glsl_fragment_shader },
    { ".comp", shaderc_glsl_compute_shader },
    { ".geom", shaderc_glsl_geometry_shader },
    { ".tesc", shaderc_glsl_tess_control_shader },
    { ".tese", shaderc_glsl_tess_evaluation_shader }
};

void ShaderCompiler::compileIfChanged(string const& shadersPath)
{
    auto shaderEntries = listModifiedFiles(shadersPath);
    for (const auto& shaderEntry : shaderEntries) {
        const auto shaderPath = shaderEntry.second.path();
        const auto filename = shaderEntry.first + ".spv";
        const auto spvPath = shaderPath.string() + ".spv";
        auto buffer = readFile(shaderPath.string());
        compile(shaderPath, buffer.data());

        const auto compiledShader = readFile(spvPath);
        compiledShaders.insert({ filename, compiledShader });
    }
}

void ShaderCompiler::compile(filesystem::path shaderPath, const string& shaderCode)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    const auto* filename = (const char*)shaderPath.filename().c_str();
    const auto ext = shaderPath.extension().string();
    auto path = filesystem::absolute(shaderPath).string();
    shaderc::SpvCompilationResult shaderModule = compiler.CompileGlslToSpv(shaderCode, shaderTypes.find(ext)->second, filename, options);

    if (shaderModule.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << shaderModule.GetErrorMessage();
    }
    vector<uint32_t> compiled { shaderModule.cbegin(), shaderModule.cend() };

    auto data = reinterpret_cast<const char*>(compiled.data());
    ofstream shaderFile(path + ".spv", ios::ate | ios::binary);
    shaderFile.clear();
    shaderFile.write(data, strlen(data) * compiled.size());
    shaderFile.close();
}

map<string, filesystem::directory_entry> ShaderCompiler::listModifiedFiles(const string& shadersPath)
{
    map<string, filesystem::directory_entry> filePaths {};
    filesystem::path rootPath(shadersPath);

    for (const auto& entry : filesystem::directory_iterator { rootPath }) {
        if (entry.is_regular_file()) {
            filePaths.insert({ entry.path().filename().string(), entry });
        }
    }

    map<string, filesystem::directory_entry> compilePaths {};
    for (const auto& entry : filePaths) {
        bool spvFile = entry.second.path().extension() == ".spv";

        if (!spvFile) {
            auto spv = entry.first + ".spv";

            if (filePaths.contains(spv) && entry.second.last_write_time() < filePaths[spv].last_write_time()) {
                auto spvPath = entry.second.path().string() + ".spv";
                auto compiledShader = readFile(spvPath);
                compiledShaders.insert({ spv, compiledShader });
            } else {
                compilePaths.insert(entry);
            }
        }
    }
    return compilePaths;
}

vector<char> ShaderCompiler::readFile(const string& shaderPath)
{
    ifstream shaderFile(shaderPath, ios::ate | ios::binary);
    if (!shaderFile.is_open()) {
        throw runtime_error("Failed to open shader file");
    }
    size_t fileSize = (size_t)shaderFile.tellg();
    vector<char> buffer(fileSize);
    shaderFile.seekg(0);
    shaderFile.read(buffer.data(), fileSize);
    shaderFile.close();

    return buffer;
}

map<string, const vector<char>> ShaderCompiler::getCompiledShaders()
{
    return compiledShaders;
}
