#ifndef AVGENGINEV2_UTILS_FUNCTION_H
#define AVGENGINEV2_UTILS_FUNCTION_H

#include <string>

#include <stdint.h>
#include <windows.h>

#undef min
#undef max

void DataTransformV2(uint8_t* const buffer,const uint32_t decSize,const uint32_t base);

void DataTransfromV3(uint8_t* const buffer,const uint32_t decSize,const uint32_t base, const std::string_view& archiveName);

void CharReplaceInUnicode(wchar_t* buffer, const wchar_t souChar, const wchar_t repChar);

void* ReverseEndianness(void* const buffer,const uint32_t bufferSize);

std::wstring AnsiToUnicode(const std::string_view& source,const int codePage);

std::string UnicodeToAnsi(const std::wstring_view& source,const int codePage);

#endif // !AVGENGINEV2_UTILS_FUNCTION_H
