
#include "String.h"

void utilities::SplitString(const std::string& szStr, std::vector<std::string>& vTokens, char cDelimiter)
{
	vTokens.clear();
	size_t l_strLen = szStr.size();
	if (l_strLen == 0)
		return;

	size_t l_uiStartIndex = 0;
	size_t l_uiIndexOfDel = szStr.find_first_of(cDelimiter, l_uiStartIndex);
	while (l_uiIndexOfDel != std::string::npos)
	{
		vTokens.push_back(szStr.substr(l_uiStartIndex, l_uiIndexOfDel - l_uiStartIndex));
		l_uiStartIndex = l_uiIndexOfDel + 1;
		if (l_uiStartIndex >= l_strLen)
			break;
		l_uiIndexOfDel = szStr.find_first_of(cDelimiter, l_uiStartIndex);
	}
	if (l_uiStartIndex < l_strLen)
		vTokens.push_back(szStr.substr(l_uiStartIndex));
}

size_t utilities::HashString(const std::string& szStr)
{
	std::hash<std::string> l_oStrHash;
	return l_oStrHash(szStr);
}

std::string utilities::ToStr(int num, int base)
{
	char str[MAX_DIGITS_IN_INT];
	memset(str, 0, MAX_DIGITS_IN_INT);
	_itoa_s(num, str, MAX_DIGITS_IN_INT, base);
	return (std::string(str));
}

std::string utilities::ToStr(unsigned int num, int base)
{
	char str[MAX_DIGITS_IN_INT];
	memset(str, 0, MAX_DIGITS_IN_INT);
	_ultoa_s((unsigned long)num, str, MAX_DIGITS_IN_INT, base);
	return (std::string(str));
}

std::string utilities::ToStr(unsigned long num, int base)
{
	char str[MAX_DIGITS_IN_INT];
	memset(str, 0, MAX_DIGITS_IN_INT);
	_ultoa_s(num, str, MAX_DIGITS_IN_INT, base);
	return (std::string(str));
}

std::string utilities::ToStr(float num)
{
	char str[64];
	memset(str, 0, 64);
	_sprintf_p(str, 64, "%f", num);
	return (std::string(str));
}

std::string utilities::ToStr(double num)
{
	char str[64];
	memset(str, 0, 64);
	_sprintf_p(str, 64, "%fL", num);
	return (std::string(str));
}

std::string utilities::ToStr(bool val)
{
	return (std::string((val == true ? "true" : "false")));
}
