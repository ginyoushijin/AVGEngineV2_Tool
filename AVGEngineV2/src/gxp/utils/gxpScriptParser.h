#ifndef  AVGENGINEV2_UTILS_SCRIPTPARSER_H
#define  AVGENGINEV2_UTILS_SCRIPTPARSER_H

#include "gxpBinaryReader.h"

#include <string>
#include <vector>

#include <stdint.h>

class GxpScriptParser : public GxpBinaryReader
{
public:

	GxpScriptParser(const std::wstring_view target);

private:

	GxpScriptParser() = delete;

	GxpScriptParser(GxpScriptParser&) = delete;

	void ExtractScenarioSelectText(std::ofstream& scenario, uint32_t& extractLineCounter, uint32_t& tokenCounter);

	void ExtractScenarioShowText(std::ofstream& scenario, uint32_t& extractLineCounter,uint32_t& lineCounter ,uint32_t& tokenCounter);

	bool ProcessGxMoaScriptDefinition(const std::wstring_view extractPath) override;

};

#endif // ! AVGENGINEV2_UTILS_SCRIPTPARSER_H
