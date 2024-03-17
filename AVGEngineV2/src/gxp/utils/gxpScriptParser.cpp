#include "gxpScriptParser.h"

#include "../gxpType.h"
#include "gxpUtils.h"

#include <fstream>
#include <iomanip>

#include <locale.h>

static void WriteTextByUTF8(std::ofstream& scenario,const char const* text, uint32_t& extractLineCounter)
{
	scenario << "\xE2\x98\x86" << std::setw(5) << std::setfill('\x30') << extractLineCounter << "\xE2\x98\x86" << text << "\n";
	scenario << "\xE2\x98\x85" << std::setw(5) << std::setfill('\x30') << extractLineCounter << "\xE2\x98\x85" << text << "\n";
	scenario << "\n";
	extractLineCounter++;
}

GxpScriptParser::GxpScriptParser(const std::wstring_view target) :GxpBinaryReader(target) 
{
	
}

void GxpScriptParser::ExtractScenarioSelectText(std::ofstream& scenario,uint32_t& extractLineCounter,uint32_t& tokenCounter)
{
	char textBuffer[512]{ 0 };
	char* textPos = textBuffer;

	this->SkipBytes(19);
	tokenCounter++;

	do
	{

		this->SkipBytes(6);

		GxpTokenType tokenType;
		this->ReadBytesByBE(&tokenType, 4);

		if (tokenType == GxpTokenType::ParamterEnd) break;

		if (tokenType == GxpTokenType::Paramter)
		{
			uint32_t textLength;

			this->SkipBytes(1);
			this->ReadBytesByBE(&textLength, 4);
			this->ReadBytesByLE(textPos, textLength + 1);
			textPos += textLength;
		}
		else
		{
			strcpy(textPos, "|");
			textPos++;
		}

		tokenCounter++;

	} while (true);

	WriteTextByUTF8(scenario, textBuffer, extractLineCounter);
	tokenCounter++;

}

void GxpScriptParser::ExtractScenarioShowText(std::ofstream& scenario, uint32_t& extractLineCounter,uint32_t& lineCounter ,uint32_t& tokenCounter)
{
	char textBuffer[1024]{0};
	char* textPos = textBuffer;
	uint32_t textLength;

	this->SkipBytes(10);	//1A 00 00 00 04 74 65 78 74 00
	tokenCounter++;

	GxpTypeCode typecode;
	this->ReadBytesByLE(&typecode, 1);

	if (typecode != GxpTypeCode::ListEnd)	//没有end说明有人名(语音调用)参数
	{
		this->SkipBytes(20);
		this->ReadBytesByBE(&textLength, 4);
		if (textLength)
		{
			this->ReadBytesByLE(textBuffer, textLength + 1);	//获得人名
			this->SkipBytes(6);
			tokenCounter += 2;

			WriteTextByUTF8(scenario,textBuffer, extractLineCounter);

			memset(textBuffer, 0, sizeof(textBuffer));

			GxpTokenType tokenType;
			this->ReadBytesByBE(&tokenType, 4);
			if (tokenType == GxpTokenType::LinkMark)	//检测是否有语音调用
			{
				this->SkipBytes(11);
				this->ReadBytesByBE(&textLength , 4);
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
			this->SkipBytes(6);
			tokenCounter += 2;
		}
	}

	uint32_t listSize = 0;

	do
	{
		this->SkipBytes(1);
		this->ReadBytesByBE(&listSize, 4);

		if(listSize==2)
		{
			break;
		}
		else if(listSize==0)
		{
			this->SkipBytes(1);
			lineCounter++;
			continue;
		}

		this->SkipBytes(11);
		this->ReadBytesByBE(&textLength, 4);
		this->ReadBytesByLE(textPos, textLength + 1);
		textPos += textLength;
		strcpy(textPos, "\\n");
		textPos += 2;
		this->SkipBytes(1);
		lineCounter++;
	} while (true);

	strcpy(textPos - 2, "");
	WriteTextByUTF8(scenario, textBuffer, extractLineCounter);

	this->SkipBytes(42);

	lineCounter++;
}

bool GxpScriptParser::ProcessGxMoaScriptDefinition(const std::wstring_view extractPath)
{
	this->SkipBytes(1);

	uint32_t length;

	this->ReadBytesByBE(&length, 4);

	std::string fileName;
	fileName.resize(length + 1);
	this->ReadBytesByLE(fileName.data(), length + 1);

	std::wstring wFileName = AnsiToUnicode(fileName, CP_UTF8);

	size_t slashPos = wFileName.rfind('/');
	if (slashPos != std::string::npos) wFileName.replace(slashPos, 1,L"-");

	std::wstring fullPath;
	fullPath.reserve(extractPath.length() + length + 2);
	fullPath.append(extractPath);
	fullPath.push_back(L'\\');
	fullPath.append(wFileName);

	std::ofstream scenario(fullPath, std::ios::out | std::ios::trunc);

	if (!scenario.is_open())
	{
		wprintf(L"ERROR : scenario %ws extract failed\n", wFileName.c_str());
		return false;
	}

	this->SkipBytes(1);
	this->ReadBytesByBE(&length, 4);

	for (uint32_t i = 0; i < length; i++) this->ReadGxMoaLableDefinition();

	this->SkipBytes(2);
	this->ReadBytesByBE(&length, 4);

	for (uint32_t i = 0; i < length; i++) this->ReadGxMoaVariableDefinition();

	uint32_t lineCount = 0;

	this->SkipBytes(2);
	this->ReadBytesByBE(&lineCount, 4);

	for (uint32_t lineCounter = 0, extractLineCounter = 1; lineCounter < lineCount; lineCounter++)
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
				&&tokenType == GxpTokenType::Command
				&&valueType == GxpValueType::SelectText)
			{
				this->ExtractScenarioSelectText(scenario,extractLineCounter,tokenCounter);
			}
			else if (tokenFlags & GxpTokenFlags::HaveTypeInfo 
					 &&valueType == GxpValueType::Text)
			{
				this->ExtractScenarioShowText(scenario, extractLineCounter, lineCounter, tokenCounter);
			}
			else if (tokenFlags & GxpTokenFlags::HaveString)
			{
				this->SkipBytes(1);
				this->ReadBytesByBE(&length, 4);
				this->SkipBytes(length + 1);
			}

		}

		this->SkipBytes(1);	//skip list end
	}

	scenario.close();

	return true;

}