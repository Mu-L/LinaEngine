/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef FileSystem_HPP
#define FileSystem_HPP

#include "FileData.hpp"
#include "Data/Vector.hpp"

namespace Lina
{
	class FileSystem
	{
	public:
		// Path
		static bool			  DeleteFileInPath(const String& path);
		static bool			  CreateFolderInPath(const String& path);
		static bool			  DeleteDirectory(const String& path);
		static Vector<String> GetFilesInDirectory(const String& path, String extensionFilter = "");
		static Vector<String> GetFilesAndFoldersInDirectory(const String& path);
		static bool			  ChangeDirectoryName(const String& oldPath, const String& newPath);
		static bool			  FileExists(const String& path);
		static String		  GetFilePath(const String& fileName);
		static String		  RemoveExtensionFromPath(const String& filename);
		static String		  GetFilenameAndExtensionFromPath(const String& filename);
		static String		  GetFileExtension(const String& file);
		static String		  ReadFileContentsAsString(const String& filePath);
		static void			  ReadFileContentsToVector(const String& filePath, Vector<char>& vec);
		static String		  GetRunningDirectory();

		// File and folder
		static bool	  FolderContainsDirectory(Folder* root, const String& path, DirectoryItem*& outItem);
		static void	  GetFolderHierarchToRoot(Folder* folder, Vector<Folder*>& hierarchy);
		static void	  CreateNewSubfolder(Folder* parent);
		static void	  DeleteFolder(Folder* folder);
		static void	  ParentPathUpdated(Folder* folder);
		static void	  ParentPathUpdated(File* file);
		static void	  ChangeFolderName(Folder* folder, const String& newName);
		static void	  ChangeFileName(File* file, const String& newName);
		static void	  RewriteFileContents(File* file, const String& newContent);
		static bool	  FolderContainsFilter(const Folder* folder, const String& filter);
		static bool	  SubfoldersContainFilter(const Folder* folder, const String& filter);
		static bool	  FileContainsFilter(const File& file, const String& filter);
		static File*  FindFile(Folder* root, const String& path);
		static String GetUniqueDirectoryName(Folder* parent, const String& prefix, const String& extension);

		// Utility

		/// <summary>
		/// ! You are responsible for deleting returned buffer!
		/// </summary>
		/// <param name="input"></param>
		/// <returns></returns>
		static char* WCharToChar(const wchar_t* input);

		/// <summary>
		/// ! You are responsible for deleting returned buffer!
		/// </summary>
		/// <param name="input"></param>
		/// <returns></returns>
		static wchar_t* CharToWChar(const char* input);

		static String		  ToUpper(const String& input);
		static String		  ToLower(const String& input);
		static Vector<String> Split(const String& s, char delim);
	};

} // namespace Lina

#endif
