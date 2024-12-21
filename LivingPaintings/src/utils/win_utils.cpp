#include <filesystem>
#include <shobjidl.h>
#include <string.h>
#include <windows.h>

static std::filesystem::path toFilesystemPath(PWSTR filePath)
{
    size_t convertedChars = 0;
    size_t pathLength = wcslen(filePath) + 1;
    char* charPath = new char[pathLength];
    wcstombs_s(&convertedChars, charPath, pathLength, filePath, _TRUNCATE);
    return charPath;
}

static std::filesystem::path getOpenedWindowFilePath()
{
    PWSTR filePath = nullptr;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpen;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }

    if (filePath != nullptr)
        return toFilesystemPath(filePath);

    return std::filesystem::path();
}