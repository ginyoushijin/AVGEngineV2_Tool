#include "gxpInterface.h"

#include "utils/gxpScriptParser.h"

#include <ShlObj.h>
#include <chrono>

void ExtractScenarioFromScript(const std::wstring_view& script,const std::wstring_view& extractPath)
{
	SHCreateDirectoryExW(NULL,extractPath.data(),nullptr);

	GxpScriptParser parser(script);

	if (!parser.CheckSuccess())
	{
		printf("ERROR : moacode.mwb open failed\n");
		return;
	}

	auto watchOfStart = std::chrono::steady_clock::now();

	printf("loading...\n");

	uint32_t extractCount = parser.TraverseScript(extractPath);

	auto watchOfEnd = std::chrono::steady_clock::now();

	printf("SUCCESS : extract %d files in %llu ms\n",
		extractCount,
		std::chrono::duration_cast<std::chrono::milliseconds>(watchOfEnd-watchOfStart).count());

}