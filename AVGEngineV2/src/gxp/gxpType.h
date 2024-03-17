#ifndef AVGENGINEV2_GXP_TYPE_H

#include <stdint.h>

enum class GxpTypeCode :uint8_t
{
	S32 = 0x5,	//signed int32
	U32 = 0x6,  //unsigned int32
	S64 = 0x7, //signed int64
	Bit32 = 0x15,
	StringSJIS = 0x18,
	StringUTF8 = 0x1A,
	ListBegin = 0x20,
	ListEnd = 0x21,
};

enum GxpTokenFlags
{
	HaveTypeInfo = 0b0001,
	HaveRunPos = 0b0010,
	HaveString = 0b0100,
};

enum class GxpTokenType
{
	Command = 0x1,
	Paramter = 0x5,
	ShowText = 0x8,
	LinkMark = 0x23,
	ParamterBegin = 0x28,
	ParamterEnd = 0x29,
};

enum class GxpValueType : int64_t
{
	Variable = -1,
	Text = 0x38E,
	TextEnd = 0x38F,
	SelectText = 0x7D0,
};


#endif // !AVGENGINEV2_GXP_VM_H
