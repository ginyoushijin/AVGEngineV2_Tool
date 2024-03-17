#include "gxpBinaryReader.h"

#include "gxpUtils.h"

#include <fstream>


GxpBinaryReader::GxpBinaryReader(const std::wstring_view target)
{
	std::ifstream script(target.data(), std::ios::binary | std::ios::in);

	if (!script.is_open()) return;

	script.seekg(0, std::ios::end);
	uint64_t scriptSize = script.tellg();
	this->_data.resize(scriptSize);
	script.seekg(0, std::ios::beg);
	script.read(reinterpret_cast<char*>(this->_data.data()), scriptSize);
}

void GxpBinaryReader::ReadBytesByLE(void* const buffer, const uint64_t readBytes)
{
	if (buffer == nullptr) return;

	memcpy(buffer, this->CurrentPosition(), readBytes);

	this->_pos += readBytes;
}

void GxpBinaryReader::ReadBytesByBE(void* const buffer, const uint64_t readBytes)
{
	if (buffer == nullptr) return;

	memcpy(buffer,this->CurrentPosition(),readBytes);

	ReverseEndianness(buffer, readBytes);

	this->_pos += readBytes;

}

void GxpBinaryReader::ReadGxMoaLableDefinition()
{

	this->SkipBytes(1);

	uint32_t length;

	this->ReadBytesByBE(&length, 4);
	this->SkipBytes(length + 12);
	this->ReadBytesByBE(&length, 4);
	this->SkipBytes(length + 6);

}

void GxpBinaryReader::ReadGxMoaVariableDefinition()
{
	this->SkipBytes(1);

	uint32_t length;

	this->ReadBytesByBE(&length, 4);
	this->SkipBytes(length + 17);
	this->ReadBytesByBE(&length, 4);
	this->SkipBytes(length + 6);

}

void GxpBinaryReader::ReadGxMoaBuiltinDefinition()
{
	this->SkipBytes(1);

	uint32_t length;

	this->ReadBytesByBE(&length, 4);
	this->SkipBytes(length + 16);
}

uint32_t GxpBinaryReader::TraverseScript(const std::wstring_view path)
{
	uint32_t extractCount = 0;
	uint32_t listSize = 0;

	this->SkipBytes(1);
	this->ReadBytesByBE(&listSize, 4);
	for (uint32_t i = 0; i < listSize; i++) this->ReadGxMoaLableDefinition();

	this->SkipBytes(2);
	this->ReadBytesByBE(&listSize, 4);
	for (uint32_t i = 0; i < listSize; i++) this->ReadGxMoaVariableDefinition();

	this->SkipBytes(2);
	this->ReadBytesByBE(&listSize, 4);
	for (uint32_t i = 0; i < listSize; i++) this->ReadGxMoaBuiltinDefinition();

	setlocale(LC_ALL, "");

	this->SkipBytes(2);
	this->ReadBytesByBE(&listSize, 4);
	for (uint32_t i = 0; i < listSize; i++)
	{
		if (!this->ProcessGxMoaScriptDefinition(path)) break;
		this->SkipBytes(1);
		extractCount++;
	}

	this->SkipBytes(1);

	return extractCount;
}