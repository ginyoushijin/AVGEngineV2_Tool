#ifndef AVGENGINEV2_INTERFACE_H
#define AVGENGINEV2_INTERFACE_H

#include <string>

void ExtractResourcesFromGxpArchive(const std::wstring_view target,const std::wstring_view exPath);

void CreateGxpArchiveImportResources(const std::wstring_view resPath, const std::wstring_view createArchive,bool encryptFlag);

void ExtractScenarioFromScript(const std::wstring_view script, const std::wstring_view extractPath);

void UpdateScenarioByOrigScript(const std::wstring_view origScript, const std::wstring_view scePath, const std::wstring_view newScript);

#endif // ! AVGENGINEV2_TOOL_INTERFACE_H
