#pragma once
#include <string>

enum class eParseRetType
{
	kSuccess,
	kFileNotExists,
	kEyeKeywordFormatError,
	kMissingKeywords,
};


class FileReaderBase
{
public:
	FileReaderBase(){}
	FileReaderBase(const std::string& _fileName)
		: m_fileName(_fileName) {}
	virtual eParseRetType parseFile() = 0;

	
protected:

	std::string m_fileName;

};