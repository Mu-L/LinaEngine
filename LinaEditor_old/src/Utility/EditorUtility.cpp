#include "Utility/EditorUtility.hpp"

#include "Log/Log.hpp"
#include "Rendering/Material.hpp"

#include <cereal/archives/binary.hpp>
#include <filesystem>
#include <fstream>
#include <Data/String.hpp>
#include <sys/stat.h>

#ifdef LINA_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <commdlg.h>
#endif

namespace Lina::Editor
{
    bool EditorUtility::CreateFolderInPath(const String& path)
    {
        return std::filesystem::create_directory(path.c_str());
    }

    bool EditorUtility::DeleteDirectory(const String& path)
    {
        return false;
    }

    String EditorUtility::RemoveExtensionFromFilename(const String& fileName)
    {
        size_t lastindex = fileName.find_last_of(".");
        return fileName.substr(0, lastindex);
    }

    String EditorUtility::OpenFile(const char* filter, void* window)
    {
#ifdef LINA_PLATFORM_WINDOWS
        OPENFILENAMEA ofn;
        CHAR          szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize  = sizeof(OPENFILENAME);
        ofn.hwndOwner    = glfwGetWin32Window((GLFWwindow*)window);
        ofn.lpstrFile    = szFile;
        ofn.nMaxFile     = sizeof(szFile);
        ofn.lpstrFilter  = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            String replacedPath = ofn.lpstrFile;
            std::replace(replacedPath.begin(), replacedPath.end(), '\\', '/');
            return replacedPath;
        }

#endif
        return String();
    }

    String EditorUtility::SaveFile(const char* filter, void* window)
    {

#ifdef LINA_PLATFORM_WINDOWS
        OPENFILENAMEA ofn;
        CHAR          szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize  = sizeof(OPENFILENAME);
        ofn.hwndOwner    = glfwGetWin32Window((GLFWwindow*)window);
        ofn.lpstrFile    = szFile;
        ofn.nMaxFile     = sizeof(szFile);
        ofn.lpstrFilter  = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn) == TRUE)
        {
            String replacedPath = ofn.lpstrFile;
            std::replace(replacedPath.begin(), replacedPath.end(), '\\', '/');
            return replacedPath;
        }
#endif
        return String();
    }

    String EditorUtility::SelectPath(void* window)
    {

#ifdef LINA_PLATFORM_WINDOWS
        OPENFILENAMEA ofn;
        CHAR          szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize  = sizeof(OPENFILENAME);
        ofn.hwndOwner    = glfwGetWin32Window((GLFWwindow*)window);
        ofn.lpstrFile    = szFile;
        ofn.nMaxFile     = sizeof(szFile);
        ofn.lpstrFilter  = ".exe";
        ofn.nFilterIndex = 1;
        ofn.Flags        = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            String replacedPath = ofn.lpstrFile;
            std::replace(replacedPath.begin(), replacedPath.end(), '\\', '/');
            return replacedPath;
        }

#endif
        return String();
    }

    bool EditorUtility::ChangeFilename(const char* folderPath, const char* oldName, const char* newName)
    {
        String oldPathStr = String(folderPath) + String(oldName);
        String newPathStr = String(folderPath) + String(newName);

        /*	Deletes the file if exists */
        if (std::rename(oldPathStr.c_str(), newPathStr.c_str()) != 0)
        {
            LINA_ERR("Can not rename file! Folder Path: {0}, Old Name: {1}, New Name: {2}", folderPath, oldName, newName);
            return false;
        }

        return true;
    }

} // namespace Lina::Editor
