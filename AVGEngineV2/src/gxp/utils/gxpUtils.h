#ifndef AVGENGINEV2_UTILS_FUNCTION_H
#define AVGENGINEV2_UTILS_FUNCTION_H

#include <string>

#include <stdint.h>
#include <windows.h>

#undef min
#undef max


void DataTransform(uint8_t* buffer, uint64_t decSize, uint8_t base);

void CharReplaceInUnicode(wchar_t* buffer, const wchar_t souChar, const wchar_t repChar);

void* ReverseEndianness(void* const buffer, uint32_t bufferSize);

std::wstring AnsiToUnicode(const std::string& source, int codePage);

#endif // !AVGENGINEV2_UTILS_FUNCTION_H
