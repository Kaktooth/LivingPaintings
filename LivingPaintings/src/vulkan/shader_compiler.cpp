// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shader_compiler.h"

using namespace std;

map<string, vector<char>> compiledShaders {};

const map<string, shaderc_shader_kind> shaderTypes {
    { ".vert", shaderc_glsl_vertex_shader },
    { ".frag", shaderc_glsl_fragment_shader },
    { ".comp", shaderc_glsl_compute_shader },
    { ".geom", shaderc_glsl_geometry_shader },
    { ".tesc", shaderc_glsl_tess_control_shader },
    { ".tese", shaderc_glsl_tess_evaluation_shader }
};

void ShaderCompiler::compileIfChanged(const string& shadersPath)
{
    std::map<string, filesystem::directory_entry> shaderEntries = listModifiedFiles(shadersPath);
    for (const pair<string, filesystem::directory_entry>& shaderEntry : shaderEntries) {
        const std::filesystem::path shaderPath = shaderEntry.second.path();
        const std::string filename = shaderEntry.first + ".spv";
        const std::string spvPath = shaderPath.string() + ".spv";
        std::vector<char> buffer = readShaderFile(shaderPath.string(), false);
        compile(shaderPath, buffer.data());
        cout << "Shader compiled: " + shaderPath.string();

        const std::vector<char> compiledShader = readShaderFile(spvPath, true);
        if (compiledShaders.contains(filename)) {
            compiledShaders[filename] = compiledShader;
        } else {
            compiledShaders.insert({ filename, compiledShader });
        }
    }
}

void ShaderCompiler::compile(const filesystem::path shaderPath, const string& shaderCode)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    const char* filename = (const char*)shaderPath.filename().c_str();
    const std::string ext = shaderPath.extension().string();
    std::string path = filesystem::absolute(shaderPath).string();
    shaderc::SpvCompilationResult shaderModule = compiler.CompileGlslToSpv(shaderCode, shaderTypes.find(ext)->second, filename, options);

    if (shaderModule.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << shaderModule.GetErrorMessage();
        throw format_error(shaderModule.GetErrorMessage());
    }
    vector<uint32_t> compiled { shaderModule.cbegin(), shaderModule.cend() };

    const char* data = reinterpret_cast<const char*>(compiled.data());
    ofstream shaderFile(path + ".spv", ios::ate | ios::binary);
    shaderFile.clear();
    shaderFile.write(data, strlen(data) * compiled.size());
    shaderFile.close();
}

map<string, filesystem::directory_entry> ShaderCompiler::listModifiedFiles(const string& shadersPath)
{
    map<string, filesystem::directory_entry> filePaths {};
    map<string, filesystem::directory_entry> compilePaths {};
    filesystem::path rootPath(shadersPath);

    for (const std::filesystem::directory_entry& entry : filesystem::directory_iterator { rootPath }) {
        if (entry.is_regular_file()) {
            filePaths.insert({ entry.path().filename().string(), entry });
        }
    }

    for (const std::pair<string, filesystem::directory_entry>& entry : filePaths) {
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

vector<char> ShaderCompiler::readShaderFile(const string& shaderPath, bool spvShader)
{
    ifstream shaderFile(shaderPath, ios::ate | ios::binary);

    if (!shaderFile.is_open()) {
        throw runtime_error("Failed to open shader file.");
    }

    size_t fileSize = (size_t)shaderFile.tellg();
    vector<char> buffer(fileSize + !spvShader);
    shaderFile.seekg(0);
    shaderFile.read(buffer.data(), fileSize);
    shaderFile.close();
    spvShader ?: buffer[fileSize] = '\0';
    return buffer;
}

map<string, vector<char>> ShaderCompiler::getCompiledShaders()
{
    return compiledShaders;
}
