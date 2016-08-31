
#include "LogManager.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <tinyxml2.h>
#include <Windows.h>

const unsigned char LOGFLAG_WRITE_TO_LOG_FILE = 1 << 0;
const unsigned char LOGFLAG_WRITE_TO_DEBUGGER = 1 << 1;

logger::LogManager::LogManager()
	: Singleton{}
	, m_szLogFilename{"error.log"}
{
	unsigned char l_ucFlags = LOGFLAG_WRITE_TO_LOG_FILE | LOGFLAG_WRITE_TO_DEBUGGER;
	SetOutputFlags("ERROR", l_ucFlags);
	SetOutputFlags("WARNING", l_ucFlags);
	SetOutputFlags("INFO", l_ucFlags);
}

void logger::LogManager::CreateInstance(const char * szConfigFilename)
{
	if (GetSingletonPtr() == nullptr)
	{
		LogManager* l_oLogManager = new LogManager{};
		l_oLogManager->Init(szConfigFilename);
	}
}


void logger::LogManager::Init(const char * szConfigFilename)
{
	if (szConfigFilename)
	{
		tinyxml2::XMLDocument l_oConfigXMLDoc;
		l_oConfigXMLDoc.LoadFile(szConfigFilename);
		if (!l_oConfigXMLDoc.Error())
		{
			const tinyxml2::XMLElement* l_pRootElem = l_oConfigXMLDoc.RootElement();
			if (!l_pRootElem)
				return;

			const tinyxml2::XMLElement* l_pLoggingElem = l_pRootElem->FirstChildElement("Logging");
			if (l_pLoggingElem)
			{
				std::string l_szLogFilenameTemplate = l_pLoggingElem->Attribute("filenameTemplate");
				CreateLogFilename(l_szLogFilenameTemplate);

				// Loop through each child element and load the component
				for (const tinyxml2::XMLElement* l_pNode = l_pLoggingElem->FirstChildElement("Log"); l_pNode; l_pNode = l_pNode->NextSiblingElement())
				{
					unsigned char l_ucFlags = 0;
					std::string l_szTag{ l_pNode->Attribute("tag") };

					int l_iDebugger = l_pNode->IntAttribute("debugger");
					if (l_iDebugger)
						l_ucFlags |= LOGFLAG_WRITE_TO_DEBUGGER;

					int l_iLogfile = l_pNode->IntAttribute("file");
					if (l_iLogfile)
						l_ucFlags |= LOGFLAG_WRITE_TO_LOG_FILE;

					SetOutputFlags(l_szTag, l_ucFlags);
				}
			}
		}
	}
}

void logger::LogManager::CreateLogFilename(std::string szLogFilenameTemplate)
{
	time_t     rawtime;
	struct tm timeinfo;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	std::stringstream l_oSS;
	l_oSS << szLogFilenameTemplate << "-";
	l_oSS << timeinfo.tm_year + 1900;
	l_oSS << std::setfill('0') << std::setw(2) << timeinfo.tm_mon + 1;
	l_oSS << std::setfill('0') << std::setw(2) << timeinfo.tm_mday;
	l_oSS << std::setfill('0') << std::setw(2) << timeinfo.tm_hour;
	l_oSS << std::setfill('0') << std::setw(2) << timeinfo.tm_min;
	l_oSS << std::setfill('0') << std::setw(2) << timeinfo.tm_sec;
	l_oSS << ".log";

	m_szLogFilename = l_oSS.str();

}

void logger::LogManager::Log(const std::string & szTag, const std::string & szMessage, const char * szFunctionName, const char * szSourceFile, unsigned int uiLineNum)
{
	TagsMap::iterator l_oFindIt = m_oTagsMap.find(szTag);
	if (l_oFindIt != m_oTagsMap.end())
	{
		std::string l_szOutputBuffer;
		CreateOutputBuffer(l_szOutputBuffer, szTag, szMessage, szFunctionName, szSourceFile, uiLineNum);
		WriteBufferToLogs(l_szOutputBuffer, l_oFindIt->second);
	}
}

void logger::LogManager::SetOutputFlags(const std::string & szTag, unsigned char ucFlags)
{
	if (ucFlags != 0)
	{
		TagsMap::iterator l_oFindIt = m_oTagsMap.find(szTag);
		if (l_oFindIt == m_oTagsMap.end())
		{
			m_oTagsMap.insert(std::make_pair(szTag, ucFlags));
		}
		else
		{
			l_oFindIt->second = ucFlags;
		}
	}
	else
	{
		m_oTagsMap.erase(szTag);
	}
}

void logger::LogManager::CreateOutputBuffer(std::string & szOutputBuffer, const std::string & szTag, const std::string & szMessage, const char * szFunctionName, const char * szSourceFile, unsigned int uiLineNum)
{
	if (!szTag.empty())
		szOutputBuffer = "[ " + szTag + " ] " + szMessage;
	else
		szOutputBuffer = szMessage;

	if (szFunctionName != nullptr)
	{
		szOutputBuffer += "\t";
		szOutputBuffer += szFunctionName;
	}

	if (szSourceFile != nullptr)
	{
		szOutputBuffer += " (";
		szOutputBuffer += szSourceFile;
		szOutputBuffer += ") ";
	}

	if (uiLineNum != 0)
	{
		szOutputBuffer += " (line ";
		char l_acLineNumeBuffer[50];
		_itoa_s(uiLineNum, l_acLineNumeBuffer, 10);
		szOutputBuffer += l_acLineNumeBuffer;
		szOutputBuffer += ")";
	}

	szOutputBuffer += "\n";
}

void logger::LogManager::WriteBufferToLogs(const std::string & szBuffer, unsigned char ucFlags)
{
	// Write the log to each destination based on the flags
	if ((ucFlags & LOGFLAG_WRITE_TO_LOG_FILE) > 0)  // log file
		WriteToLogFile(szBuffer);
	if ((ucFlags & LOGFLAG_WRITE_TO_DEBUGGER) > 0)  // debugger output window
		::OutputDebugStringA(szBuffer.c_str());
}

void logger::LogManager::WriteToLogFile(const std::string & szData)
{
	FILE* l_pLogFile = nullptr;
	fopen_s(&l_pLogFile, m_szLogFilename.c_str(), "a+");
	if (!l_pLogFile)
		return;

	fprintf_s(l_pLogFile, szData.c_str());

	fclose(l_pLogFile);
}
