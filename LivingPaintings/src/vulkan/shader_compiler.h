// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#pragma once
#include "shaderc/shaderc.hpp"
#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

class ShaderCompiler {

    static std::map<std::string, std::filesystem::directory_entry>
    listModifiedFiles(const std::string& shadersPath);
    static std::vector<char> readShaderFile(const std::string& shaderPath, bool spvShader);
    static void compile(const std::filesystem::path shaderPath,
        const std::string& shaderCode);

public:
    static void compileIfChanged(const std::string& shaderPath);
    static std::map<std::string, std::vector<char>> getCompiledShaders();
};
