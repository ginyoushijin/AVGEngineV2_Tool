#include "gxpScriptUpdater.h"

#include "gxpUtils.h"
#include "../gxpType.h"

static std::list<std::string> GetTranslatedTexts(std::ifstream& stream)
{
	std::list<std::string> result;

	constexpr char mark[] = "\xE2\x98\x85";	//utf8 : "★"

	std::string line;

	if (!stream.is_open()) return result;

	while (std::getline(stream, line))
	{
		if (memcmp(line.c_str(), mark, 3) != 0) continue;

		result.emplace_back(line.c_str() + 11);
	}

	return result;
}

GxpScriptUpdater::GxpScriptUpdater(const std::wstring_view origScript, const std::wstring_view newScript) : GxpBinaryReader(origScript)
{
	this->_stream.open(newScript.data(), std::ios::binary | std::ios::out);
}

 void GxpScriptUpdater::SkipBytes(uint64_t length)
 {
	 this->_stream.write(reinterpret_cast<char*>(this->CurrentPosition()),length);

	 this->_pos += length;
 }

void GxpScriptUpdater::ReadBytesByLE(void* const buffer, const uint64_t readBytes)
{

	if (buffer == nullptr) return;

	memcpy(buffer, this->CurrentPosition(), readBytes);

	this->_stream.write(reinterpret_cast<char*>(buffer), readBytes);

	this->_pos += readBytes;
}

void GxpScriptUpdater::ReadBytesByBE(void* const buffer, const uint64_t readBytes)
{
	if (buffer == nullptr) return;

	memcpy(buffer, this->CurrentPosition(), readBytes);

	this->_stream.write(reinterpret_cast<char*>(buffer), readBytes);

	ReverseEndianness(buffer, readBytes);

	this->_pos += readBytes;

}

void GxpScriptUpdater::UpdateScriptSelectText(std::list<std::string>& translatedTexts, uint32_t& tokenCounter)
{
	this->SkipBytes(19);
	tokenCounter++;

	std::string text = std::move(translatedTexts.front());

	translatedTexts.pop_front();

	do
	{
		this->SkipBytes(6);

		GxpTokenType tokenType;
		this->ReadBytesByBE(&tokenType, 4);

		if (tokenType == GxpTokenType::ParamterEnd) break;

		if (tokenType == GxpTokenType::Paramter)
		{
			uint32_t textLength = 0;

			this->SkipBytes(1);
			GxpBinaryReader::ReadBytesByBE(&textLength, 4);
			GxpBinaryReader::SkipBytes(textLength + 1);

			std::string line;
			size_t slashPos = text.find('|');

			if (slashPos != std::string::npos)
			{
				line.resize(slashPos + 1, 0);
				memcpy(line.data(), text.data(), slashPos);
				text.erase(0, slashPos + 1);
			}
			else
			{
				line.resize(text.size() + 1, 0);
				memcpy(line.data(), text.data(), text.size());
			}

			uint32_t lineStrSize = line.size() - 1;

			uint32_t lineStrSizeBE = lineStrSize;

			this->_stream.write(reinterpret_cast<char*>(ReverseEndianness(&lineStrSizeBE, 4)), 4);
			this->_stream.write(line.data(), lineStrSize + 1);
		}

		tokenCounter++;

	} while (true);

	tokenCounter++;
}

void GxpScriptUpdater::UpdateScriptShowText(std::list<std::string>& translatedTexts, uint32_t& lineCounter, uint32_t& tokenCounter)
{
	uint32_t textLength;

	this->SkipBytes(10);
	tokenCounter++;

	GxpTypeCode typecode;
	this->ReadBytesByLE(&typecode, 1);

	if (typecode != GxpTypeCode::ListEnd)
	{
		this->SkipBytes(20);
		GxpBinaryReader::ReadBytesByBE(&textLength, 4);

		if (textLength)
		{
			std::string charaName = std::move(translatedTexts.front());
			translatedTexts.pop_front();

			charaName.push_back('\x0');

			GxpBinaryReader::SkipBytes(textLength + 1);

			textLength = charaName.size() - 1;

			this->_stream.write(reinterpret_cast<char*>(ReverseEndianness(&textLength, 4)), 4);
			this->_stream.write(charaName.data(), charaName.size());

			this->SkipBytes(6);

			tokenCounter += 2;

			GxpTokenType tokenType;
			this->ReadBytesByBE(&tokenType, 4);

			if (tokenType == GxpTokenType::LinkMark)
			{
				this->SkipBytes(11);
				this->ReadBytesByBE(&textLength, 4);
				this->SkipBytes(textLength + 1);
				this->SkipBytes(11);
				tokenCounter += 3;
			}
			else
			{
				this->SkipBytes(1);
				tokenCounter++;
			}


		}
		else
		{
			this->_stream.write(reinterpret_cast<char*>(&textLength), 4);
			this->SkipBytes(6);
			tokenCounter += 2;
		}

	}

	std::string text = std::move(translatedTexts.front());
	translatedTexts.pop_front();

	uint32_t listSize = 0;

	do
	{
		this->SkipBytes(1);
		this->ReadBytesByBE(&listSize, 4);

		if (listSize == 2)
		{
			break;
		}
		else if (listSize == 0)
		{
			this->SkipBytes(1);
			lineCounter++;
			continue;
		}

		this->SkipBytes(11);

		std::string line;
		size_t slashPos = text.find("\\n");

		if (slashPos != std::string::npos)
		{
			line.resize(slashPos + 1, 0);
			memcpy(line.data(), text.data(), slashPos);
			text.erase(0, slashPos + 2);
		}
		else
		{
			line.resize(text.size() + 1, 0);
			memcpy(line.data(), text.data(), text.size());
			text.erase(0, slashPos + 2);
		}

		uint32_t lineStrSize = line.size() - 1;

		this->_stream.write(reinterpret_cast<char*>(ReverseEndianness(&lineStrSize, 4)), 4);
		this->_stream.write(line.data(), line.size());

		GxpBinaryReader::ReadBytesByBE(&textLength, 4);
		GxpBinaryReader::SkipBytes(textLength + 1);
		this->SkipBytes(1);

		lineCounter++;

	} while (true);

	this->SkipBytes(42);

	lineCounter++;
}

bool GxpScriptUpdater::ProcessGxMoaScriptDefinition(const std::wstring_view scePath)
{
	this->SkipBytes(1);

	uint32_t length;

	this->ReadBytesByBE(&length, 4);

	std::string fileName;
	fileName.resize(length + 1);
	this->ReadBytesByLE(fileName.data(), length + 1);

	std::wstring wFileName = AnsiToUnicode(fileName, CP_UTF8);

	size_t slashPos = wFileName.rfind('/');
	if (slashPos != std::string::npos) wFileName.replace(slashPos, 1, L"-");

	std::wstring fullPath;
	fullPath.reserve(scePath.length() + length + 2);
	fullPath.append(scePath);
	fullPath.push_back(L'\\');
	fullPath.append(wFileName);

	std::ifstream text{ fullPath,std::ios::in };

	if (!text.is_open())
	{
		wprintf(L"ERROR : %ws open failed\n", wFileName.c_str());
	}

	std::list<std::string> translatedTexts = GetTranslatedTexts(text);

	this->SkipBytes(1);
	this->ReadBytesByBE(&length, 4);

	for (uint32_t i = 0; i < length; i++) this->ReadGxMoaLableDefinition();

	this->SkipBytes(2);
	this->ReadBytesByBE(&length, 4);

	for (uint32_t i = 0; i < length; i++) this->ReadGxMoaVariableDefinition();

	uint32_t lineCount = 0;

	this->SkipBytes(2);
	this->ReadBytesByBE(&lineCount, 4);

	for (uint32_t lineCounter = 0; lineCounter < lineCount; lineCounter++)
	{
		uint32_t tokenCount = 0;
		GxpTokenFlags tokenFlags;
		GxpValueType valueType;

		this->SkipBytes(1);
		this->ReadBytesByBE(&tokenCount, 4);

		for (uint32_t tokenCounter = 0; tokenCounter < tokenCount; tokenCounter++)
		{
			this->SkipBytes(1);
			this->ReadBytesByBE(&tokenFlags, 4);
			this->SkipBytes(1);

			GxpTokenType tokenType;
			this->ReadBytesByBE(&tokenType, 4);

			if (tokenFlags & GxpTokenFlags::HaveTypeInfo)
			{
				this->SkipBytes(1);
				this->ReadBytesByBE(&valueType, 8);
			}

			if (tokenFlags & GxpTokenFlags::HaveRunPos)
			{
				this->SkipBytes(40);
			}

			if (tokenFlags & GxpTokenFlags::HaveTypeInfo
				&& tokenType == GxpTokenType::Command
				&& valueType == GxpValueType::SelectText)
			{
				this->UpdateScriptSelectText(translatedTexts, tokenCounter);
			}
			else if (tokenFlags & GxpTokenFlags::HaveTypeInfo
				&& valueType == GxpValueType::Text)
			{
				this->UpdateScriptShowText(translatedTexts, lineCounter, tokenCounter);
			}
			else if (tokenFlags & GxpTokenFlags::HaveString)
			{
				this->SkipBytes(1);
				this->ReadBytesByBE(&length, 4);
				this->SkipBytes(length + 1);
			}

		}

		this->SkipBytes(1);
	}

	return true;
}