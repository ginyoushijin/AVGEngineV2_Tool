#ifndef AVGENGINEV2_UTILS_BINARYWRITER_H
#define AVGENGINEV2_UTILS_BINARYWRITER_H

#include "gxpBinaryReader.h"

#include <fstream>
#include <string>
#include <list>

#include <stdint.h>

class GxpScriptUpdater : public GxpBinaryReader
{
public:
	 
	GxpScriptUpdater(const std::wstring_view origScript, const std::wstring_view newScript);

	bool CheckSuccess() const override
	{
		return this->_data.size() != 0 && this->_stream.is_open();
	}

private:
	GxpScriptUpdater() = delete;
	GxpScriptUpdater(GxpScriptUpdater&) = delete;

	bool ProcessGxMoaScriptDefinition(const std::wstring_view scePath) override;

	void UpdateScriptSelectText(std::list<std::string>& translatedTexts,uint32_t& tokenCounter);

	void UpdateScriptShowText(std::list<std::string>& translatedTexts, uint32_t& lineCounter, uint32_t& tokenCounter);

	void SkipBytes(uint64_t length) override;

	void ReadBytesByLE(void* const buffer, const uint64_t readBytes) override;

	void ReadBytesByBE(void* const buffer, const uint64_t readBytes) override;

protected:
	std::ofstream _stream;
};

#endif //AVGENGINEV2_UTILS_BINARYWRITER_H