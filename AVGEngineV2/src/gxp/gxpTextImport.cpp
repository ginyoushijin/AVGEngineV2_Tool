#include "gxpInterface.h"

#include "utils/gxpScriptUpdater.h"

#include <chrono>

void UpdateScenarioByOrigScript(const std::wstring_view origScript,const std::wstring_view scePath,const std::wstring_view newScript)
{
	GxpScriptUpdater updater(origScript, newScript);

	if(!updater.CheckSuccess())
	{
		printf("ERROR : origin script open failed or new script create failed\n");
		return;
	}

	auto watchOfStart = std::chrono::steady_clock::now();

	printf("loading...\n");

	updater.TraverseScript(scePath);

	auto watchOfEnd = std::chrono::steady_clock::now();

	printf("SUCCESS : update script to new script in %llu ms\n",
			std::chrono::duration_cast<std::chrono::milliseconds>(watchOfEnd - watchOfStart).count());
}