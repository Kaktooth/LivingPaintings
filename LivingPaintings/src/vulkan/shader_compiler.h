// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "shaderc/shaderc.hpp"
#include <filesystem>
#include <map>
#include <string>

class ShaderCompiler {

    std::map<std::string, std::filesystem::directory_entry> listModifiedFiles(const std::string& shadersPath);
    std::vector<char> readFile(const std::string& shaderPath);
    void compile(const std::filesystem::path shaderPath, const std::string& shaderCode);

public:
    void compileIfChanged(const std::string& shaderPath);
    std::map<std::string, const std::vector<char>> getCompiledShaders();
};
