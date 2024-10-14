#include "gxpUtils.h"

constexpr static uint8_t keySize = 0x17;

static uint8_t keyMap[keySize] = { 0x40,0x21,0x28,0x38,0xA6,0x6E,0x43,0xA5,0x40,0x21,0x28,0x38,0xA6,0x43,0xA5,0x64,
									0x3E,0x65,0x24,0x20,0x46,0x6E,0x74 };

void DataTransformV2(uint8_t* const buffer,const uint32_t decSize,const uint32_t base)
{
	if (!decSize) return;

	uint8_t keyIndex = 0;
	uint8_t key = 0;

	for (uint64_t i = 0; i < decSize; ++i)
	{
		keyIndex = (i + base) % keySize;
		key = i + base;
		key ^= keyMap[keyIndex];
		buffer[i] ^= key;
	}

	return;
}

void DataTransfromV3(uint8_t* const buffer,const uint32_t decSize,const uint32_t base, const std::string_view& archiveName)
{
	if (!decSize) return;

	uint8_t key = 0;
	uint8_t archNameIndex = 0;

	for(uint32_t i = 0;i<decSize;++i)
	{
		key = archiveName[(i + base) % archiveName.size()];
		key ^= keyMap[(i+base) % keySize];
		key ^= i + base;
		buffer[i] ^= key;
	}

}

void CharReplaceInUnicode(wchar_t* buffer, const wchar_t souChar, const wchar_t repChar)
{
	while (*buffer != 0)
	{
		if (*buffer == souChar) *buffer = repChar;
		buffer++;
	}
}

void* ReverseEndianness(void* const buffer, const uint32_t bufferSize)
{
	uint8_t* _dst = reinterpret_cast<uint8_t*>(buffer);

	const uint32_t boundary = static_cast<uint32_t>(floorf(bufferSize / 2));

	for (uint32_t head = 0, tail = bufferSize - 1; head < boundary; head++, tail--)
	{
		_dst[head] ^= _dst[tail];
		_dst[tail] ^= _dst[head];
		_dst[head] ^= _dst[tail];
	}

	return buffer;
}

std::wstring AnsiToUnicode(const std::string_view& source,const int codePage)
{
	if (source.length() == 0) {
		return std::wstring();
	}
	if (source.length() > (size_t)std::numeric_limits<int>::max()) {
		return std::wstring();
	}
	int length = MultiByteToWideChar(codePage, 0, source.data(), (int)source.length(), NULL, 0);
	if (length <= 0) {
		return std::wstring();
	}
	std::wstring output(length, L'\0');
	if (MultiByteToWideChar(codePage, 0, source.data(), (int)source.length(), (LPWSTR)output.data(), (int)output.length() + 1) == 0) {
		return std::wstring();
	}
	return output;
}

std::string UnicodeToAnsi(const std::wstring_view& source,const int codePage)
{
	if (source.length() == 0) {
		return std::string();
	}
	if (source.length() > (size_t)std::numeric_limits<int>::max()) {
		return std::string();
	}
	int length = WideCharToMultiByte(codePage, 0, source.data(), (int)source.length(), NULL, 0, NULL, NULL);
	if (length <= 0) {
		return std::string();
	}
	std::string output(length, '\0');
	if (WideCharToMultiByte(codePage, 0, source.data(), (int)source.length(), (LPSTR)output.data(), (int)output.length() + 1, NULL, NULL) == 0) {
		return std::string();
	}
	return output;
}
