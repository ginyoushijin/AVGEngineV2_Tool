#ifndef  AVGENGINEV2_UTILS_BINARYREADER_H
#define  AVGENGINEV2_UTILS_BINARYREADER_H

#include <vector>
#include <string>

#include <stdint.h>

class GxpBinaryReader
{
protected:

	void ReadGxMoaLableDefinition();

	void ReadGxMoaVariableDefinition();

	void ReadGxMoaBuiltinDefinition();

	virtual void ReadBytesByBE(void* const buffer,const uint64_t readBytes);

	virtual void ReadBytesByLE(void* const buffer,const uint64_t readBytes);

	virtual bool ProcessGxMoaScriptDefinition(const std::wstring_view) = 0;

	uint8_t* CurrentPosition()
	{
		return &(this->_data.data()[this->_pos]);
	}

	virtual void SkipBytes(uint64_t length)
	{
		this->_pos += length;
	}

private:
	GxpBinaryReader() = delete;
	GxpBinaryReader(GxpBinaryReader&) = delete;

public : 

	uint32_t TraverseScript(const std::wstring_view path);

	GxpBinaryReader(const std::wstring_view target);

	virtual bool CheckSuccess() const
	{
		return this->_data.size() != 0;
	}


protected:
	std::vector<uint8_t> _data;
	uint64_t _pos = 0;

};

#endif // ! AVGENGINEV2_UTILS_BINARYREADER_H
