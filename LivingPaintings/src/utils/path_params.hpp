#pragma once
#include "../config.hpp"
#include <cstdint>
#include <string>
#include <windows.h>

namespace Runtime {
    static struct PathParams {

        // TEXTURE_PATH variable is retrieved from Cmake with macros in file
        // config.hpp.in
        std::string TEXTURE_PATH;
        std::string SHADER_PATH;
        std::string PREPROCESS_SAM_MODEL_PATH;
        std::string SAM_MODEL_PATH;

        PathParams() {
            DWORD executablePathLength = 140;
            LPSTR lpExecutablePath = (LPSTR)malloc(executablePathLength * sizeof(char));
            GetCurrentDirectoryA(executablePathLength, lpExecutablePath);
            std::string executablePath = std::string(lpExecutablePath);
            TEXTURE_PATH = RETRIEVE_PATH(executablePath, TEXTURE_FILE_PATH);
            SHADER_PATH = RETRIEVE_PATH(executablePath, RESOURCE_SHADER_PATH);
            PREPROCESS_SAM_MODEL_PATH = RETRIEVE_PATH(executablePath, PREPROCESS_SAM_PATH);
            SAM_MODEL_PATH = RETRIEVE_PATH(executablePath, SAM_PATH);
        }
    } PATH_PARAMS;
}