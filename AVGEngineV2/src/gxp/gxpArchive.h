#ifndef AVGENGINEV2_GXP_ARCHIVE_INFO_H
#define AVGENGINEV2_GXP_ARCHIVE_INFO_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0

constexpr char c_archiveHeader[4]{ 0x47 /*G*/ ,0x58 /*X*/,0x50 /*P*/,0x0 };

union SizeVar
{
	uint32_t size32;
	uint64_t size64 = 0;
};

#pragma pack(push,4)

struct GxpArchiveInfo
{
	uint32_t unknowInfo1{ 0x64 };
	uint32_t unknowInfo2{ 0x10203040 };
	uint32_t enableFlag{ TRUE };
	uint32_t unknowInfo3 {0};
	uint32_t encryptFlag{ TRUE };
	uint32_t entryCount = 0;
	uint32_t contentsSize = 0;
	SizeVar dataSectionSize;
	SizeVar infoSectionSize; //archiveHeader size + contents size	
};

struct GxpContentsEntryInfo
{
	uint32_t entrySize = 0;	//GxpContentsEntry size + unicodeFileName size
	SizeVar dataSize;
	uint32_t fileNameLength = 0;
	uint64_t unknowInfo = 0;
	SizeVar offsetInDataSection;
	//后接一个不定长的unicode filename
};

#pragma pack(pop)

static_assert(sizeof(GxpArchiveInfo) == 0x2C, "GxpArchiveInfo size error");

static_assert(sizeof(GxpContentsEntryInfo) == 0x20, "GxpContentsEntryInfo size error");

#endif // !AVGENGINEV2_GXP_ARCHIVE_INFO_H
