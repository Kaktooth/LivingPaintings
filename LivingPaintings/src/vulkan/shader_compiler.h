#pragma once
#include "consts.h"
#include "shaderc/shaderc.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

class ShaderCompiler {
    static std::map<std::string, std::filesystem::directory_entry> listModifiedFiles();
    static std::vector<char> readShaderFile(const std::string& shaderPath, bool spvShader);
    static void compile(std::filesystem::path shaderPath, const std::string& shaderCode);

public:
    static void compileIfChanged();
    static std::map<std::string, std::vector<char>> getCompiledShaders();
};
