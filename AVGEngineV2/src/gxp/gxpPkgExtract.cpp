#include "gxpInterface.h"

#include "gxpArchive.h"
#include "utils/gxpUtils.h"

#include <ShlObj.h>
#include <locale.h>

#include <fstream>
#include <vector>
#include <future>
#include <chrono>
#include <list>

struct ExtractEntry
{
	GxpContentsEntryInfo const* info;
	wchar_t* name;
};

static std::ifstream OpenGxpArchive(const std::wstring_view target)
{
	std::ifstream archive{ target.data(),std::ios::binary | std::ios::in };

	if (!archive.is_open()) return {};

	char fileHeader[4];

	archive.read(fileHeader, sizeof(fileHeader));

	if (memcmp(fileHeader, c_archiveHeader, sizeof(c_archiveHeader)) != 0)	//判断文件头
	{
		return {};
	}
	else
	{
		return archive;
	}

}

static void GetExtarctSetInEncrypt(std::vector<ExtractEntry>& set, uint8_t* const initAdress)
{
	uint8_t* entryAdress = initAdress;

	for (uint32_t i = 0, entrySize = 0; i < set.size(); ++i)
	{
		set[i].info = reinterpret_cast<GxpContentsEntryInfo const*>(entryAdress);
		set[i].name = reinterpret_cast<wchar_t*>(entryAdress + sizeof(GxpContentsEntryInfo));
		DataTransform(entryAdress, 4, 0);
		entrySize = *reinterpret_cast<uint32_t const*>(entryAdress);
		DataTransform(entryAdress + 4, entrySize - 4, 4);
		entryAdress += entrySize;
	}
}

static void GetExtractSetInUnEncrypt(std::vector<ExtractEntry>& set, uint8_t* const initAdress)
{
	uint8_t* entryAdress = initAdress;

	for (uint32_t i = 0; i < set.size(); ++i)
	{
		set[i].info = reinterpret_cast<GxpContentsEntryInfo const*>(entryAdress);
		set[i].name = reinterpret_cast<wchar_t*>(entryAdress + sizeof(GxpContentsEntryInfo));
		entryAdress += *reinterpret_cast<uint32_t const*>(entryAdress);
	}
}

uint32_t ExtractResourceTask(std::vector<ExtractEntry>::iterator iterator, const uint32_t estGoal, const uint32_t baseOffset, const std::wstring_view target, bool encryptFlag, const std::wstring_view extractPath)
{
	uint32_t extractCount = 0;
	std::vector<uint8_t> buffer;

	setlocale(LC_ALL, "");

	std::ifstream archive{ target.data(),std::ios::binary | std::ios::in };	//open archive stream

	if (!archive.is_open())
	{
		printf("ERROR : thread %d target file open failed\n", std::this_thread::get_id());
		return 0;
	}

	std::wstring fileFullPath;	//封包文件项导出的全路径

	fileFullPath.reserve(extractPath.length() + 1);
	fileFullPath.append(extractPath);
	fileFullPath.push_back(L'\\');

	const uint32_t nameSectionPos = fileFullPath.rfind(L'\\') + 1;

	for (uint32_t i = 0; i < estGoal; ++i, ++iterator)
	{
		CharReplaceInUnicode(iterator->name, '/', L'\\');
		fileFullPath.erase(nameSectionPos);
		fileFullPath.append(iterator->name);
		SHCreateDirectoryExW(NULL, fileFullPath.substr(0, fileFullPath.rfind(L'\\')).c_str(), nullptr);

		std::ofstream file{ fileFullPath,std::ios::binary | std::ios::out };

		if (!file.is_open())
		{
			wprintf(L"ERROR : file %ws create failed\n", iterator->name);
			continue;
		}

		buffer.resize(iterator->info->dataSize.size64);
		archive.seekg(baseOffset + iterator->info->offsetInDataSection.size64);
		archive.read(reinterpret_cast<char*>(buffer.data()), iterator->info->dataSize.size64);	//读取文件项数据
		if (encryptFlag) DataTransform(buffer.data(), iterator->info->dataSize.size64, 0);
		file.write(reinterpret_cast<char*>(buffer.data()), iterator->info->dataSize.size64);	//导出数据到文件

		file.close();
		extractCount++;
	}

	archive.close();

	return extractCount;
}

void ExtractResourcesFromGxpArchive(const std::wstring_view target, const std::wstring_view exPath)
{
	std::ifstream archive = OpenGxpArchive(target);

	if (!archive.is_open())
	{
		printf("ERROR : target file format don't match\n");
		return;
	}

	auto watchOfStart = std::chrono::steady_clock::now();

	GxpArchiveInfo archiveInfo;

	archive.read(reinterpret_cast<char*>(&archiveInfo), sizeof(archiveInfo));

	std::vector<uint8_t> contentsBuffer(archiveInfo.contentsSize);

	archive.read(reinterpret_cast<char*>(contentsBuffer.data()), archiveInfo.contentsSize);

	std::vector<ExtractEntry> extractSet(archiveInfo.entryCount);

	if (archiveInfo.encryptFlag)
	{
		GetExtarctSetInEncrypt(extractSet, contentsBuffer.data());
	}
	else
	{
		GetExtractSetInUnEncrypt(extractSet, contentsBuffer.data());
	}

	archive.close();

	uint32_t extractCount = 0;
	const uint32_t maxThread = std::thread::hardware_concurrency();

	if (archiveInfo.entryCount > maxThread*0xF)
	{
		std::list<std::future<uint32_t>> tasks;

		const uint32_t threadMaxTask = ceilf(static_cast<float>(archiveInfo.entryCount) / static_cast<float>(maxThread));
		uint32_t remainingTask = archiveInfo.entryCount;
		auto iter = extractSet.begin();

		for (uint32_t i = 0; i < maxThread; ++i)
		{
			uint32_t threadLoad = std::min(remainingTask, threadMaxTask);
			auto  task = std::async(std::launch::async, ExtractResourceTask, iter, threadLoad, archiveInfo.infoSectionSize.size32, target, archiveInfo.encryptFlag, exPath);
			remainingTask -= threadLoad;
			iter += threadLoad;
			tasks.emplace_back(std::move(task));
		}

		for (auto& task : tasks)
		{
			extractCount += task.get();
		}
	}
	else
	{
		extractCount = ExtractResourceTask(extractSet.begin(), archiveInfo.entryCount, archiveInfo.infoSectionSize.size32, target, archiveInfo.encryptFlag, exPath);
	}

	auto watchOfEnd = std::chrono::steady_clock::now();

	printf("SUCCESS : extract files %d/%d from %ws in %llu ms\n",
		extractCount,
		archiveInfo.entryCount,
		target.substr(target.rfind('\\') + 1).data(),
		std::chrono::duration_cast<std::chrono::milliseconds>(watchOfEnd - watchOfStart).count());

	return;
}