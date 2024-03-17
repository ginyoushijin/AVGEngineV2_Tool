#include "gxpInterface.h"

#include "gxpArchive.h"
#include "utils/gxpUtils.h"

#include <io.h>

#include <fstream>
#include <vector>
#include <list>
#include <chrono>
#include <future>

struct FileInfo
{
	std::wstring fullPath;
	std::wstring relativePath;
	uint64_t size;
};

std::list<FileInfo> GetDirFileInfo(const std::wstring_view targetDir, const std::wstring_view whiteList)
{
	std::list <FileInfo> files;
	std::wstring filter;

	filter.reserve(whiteList.length() + 1);
	filter.push_back(L'\\');
	filter.append(whiteList);


	uint32_t relativeArchor = targetDir.length() + 1;
	_wfinddata_t findData;

	std::list<std::wstring> dirs;
	dirs.emplace_back(targetDir);

	while (!dirs.empty())
	{
		std::wstring currentPath = std::move(dirs.front());
		dirs.pop_front();	//read a directory

		std::wstring pattern;
		pattern.reserve(currentPath.size() + filter.size());
		pattern.append(currentPath);
		pattern.append(filter);	

		intptr_t findHandle = _wfindfirst(pattern.c_str(), &findData);

		if (findHandle == -1) continue;

		do
		{
			uint32_t fullPathLength = currentPath.length() + 1 + wcslen(findData.name);

			if (findData.attrib & _A_ARCH)
			{
				FileInfo curFileInfo;

				curFileInfo.fullPath.reserve(fullPathLength);
				curFileInfo.fullPath.append(currentPath);
				curFileInfo.fullPath.push_back(L'\\');
				curFileInfo.fullPath.append(findData.name);	//文件的全路径

				curFileInfo.relativePath.reserve(fullPathLength - relativeArchor);
				curFileInfo.relativePath.append(curFileInfo.fullPath.begin() + relativeArchor, curFileInfo.fullPath.end()); //文件相对于目标文件夹的相对路径

				curFileInfo.size = findData.size; //文件size

				files.emplace_back(std::move(curFileInfo));

			}
			else if (findData.attrib & _A_SUBDIR)
			{
				if (findData.name[0] == '.') continue;	//跳过 '.' 以及 '..'

				std::wstring curPath;

				curPath.reserve(fullPathLength);
				curPath.append(currentPath);
				curPath.push_back(L'\\');
				curPath.append(findData.name);

				dirs.emplace_back(std::move(curPath));
			}


		} while (_wfindnext(findHandle, &findData) == 0);

		_findclose(findHandle);
	}

	return files;
}

struct ImportEntry
{
	GxpContentsEntryInfo Info;
	wchar_t relativePath[_MAX_PATH]{ 0 };
};

void CreateGxpArchiveImportResources(const std::wstring_view resPath, const std::wstring_view createArchive, bool encryptFlag)
{
	std::list<FileInfo> fileinfoSet = GetDirFileInfo(resPath, L"*.*");

	if(fileinfoSet.empty())
	{
		printf("ERROR : No any file import to archive\n");
		return;
	}

	std::ofstream archive{ createArchive.data(),std::ios::binary | std::ios::out };

	if (!archive.is_open())
	{
		printf("ERROR : archive create failed\n");
		return;
	}

	auto watchOfStart = std::chrono::steady_clock::now();

	setlocale(LC_ALL, "");

	archive.write(c_archiveHeader, sizeof(c_archiveHeader));	//写入文件头

	GxpArchiveInfo archiveInfo;

	if (encryptFlag) archiveInfo.encryptFlag = FALSE;

	archive.write(reinterpret_cast<char*>(&archiveInfo), sizeof(archiveInfo));	//封包信息占位

	std::list<std::vector<char>> fileDataSet;
	uint32_t importCount = 0;
	uint64_t contentSize = 0;
	uint64_t dataSectionSize = 0;

	for (FileInfo& fileinfo : fileinfoSet)	//遍历目标文件夹文件，读取后写入封包
	{
		std::ifstream file(fileinfo.fullPath, std::ios::binary | std::ios::in);

		if (!file.is_open())
		{
			wprintf(L"ERROR : %ws open failed\n", fileinfo.fullPath.c_str());
			continue;
		}

		std::vector<char> dataBuffer;
		dataBuffer.resize(fileinfo.size);
		file.read(dataBuffer.data(), fileinfo.size);	//读取该文件数据

		ImportEntry importEntry;
		uint32_t pathLength = fileinfo.relativePath.length();
		uint32_t entryRealSize = sizeof(GxpContentsEntryInfo) + (pathLength + 1) * 2;	//计算封包目录区域的size

		importEntry.Info.dataSize.size64 = fileinfo.size;
		importEntry.Info.offsetInDataSection.size64 = dataSectionSize;
		importEntry.Info.fileNameLength = pathLength;
		importEntry.Info.entrySize = entryRealSize;	//set entry info

		wcscpy(importEntry.relativePath, fileinfo.relativePath.c_str());
		CharReplaceInUnicode(importEntry.relativePath, L'\\', L'/');

		if(encryptFlag)	//数据加密
		{
			DataTransform(reinterpret_cast<uint8_t*>(&importEntry), entryRealSize, 0);
			DataTransform(reinterpret_cast<uint8_t*>(dataBuffer.data()), dataBuffer.size(), 0);
		}

		archive.write(reinterpret_cast<char*>(&importEntry), entryRealSize);	//写入文件项info

		fileDataSet.emplace_back(std::move(dataBuffer));	//保存文件数据到集合
		contentSize += entryRealSize;
		dataSectionSize += fileinfo.size;	//更新封包的dataSectionSize记录
		importCount++;

	}

	for (auto& fileData : fileDataSet)	//写入所有文件数据到封包
	{
		archive.write(fileData.data(),fileData.size());
	}

	archiveInfo.contentsSize = contentSize;
	archiveInfo.infoSectionSize.size64 = 4 + sizeof(GxpArchiveInfo) + contentSize;
	archiveInfo.dataSectionSize.size64 = dataSectionSize;
	archiveInfo.entryCount = importCount;	//更新封包info

	archive.seekp(sizeof(c_archiveHeader), std::ios::beg);
	archive.write(reinterpret_cast<char*>(&archiveInfo), sizeof(GxpArchiveInfo));	//写入更新后封包的info

	auto watchOfEnd = std::chrono::steady_clock::now();

	printf("SUCCESS : imported files %d/%d to %ws in %llu ms\n",
		importCount,
		fileinfoSet.size(),
		createArchive.substr(createArchive.rfind(L'\\') + 1).data(),
		std::chrono::duration_cast<std::chrono::milliseconds>(watchOfEnd - watchOfStart).count()
	);
}