#pragma once

#include "../core/Singleton.h"

#include <map>
#include <string>
#include <sstream>

#ifdef _DEBUG

#define LOG_ERROR(str) \
	do \
	{ \
		std::string s((str)); \
		logger::LogManager::GetSingleton().Log("ERROR", s, __FUNCTION__, __FILE__, __LINE__); \
	} \
	while (0) \

#define LOG_WARNING(str) \
	do \
	{ \
		std::string s((str)); \
		logger::LogManager::GetSingleton().Log("WARNING", s, __FUNCTION__, __FILE__, __LINE__); \
	} \
	while (0) \

#define LOG_INFO(str) \
	do \
	{ \
		std::string s((str)); \
		logger::LogManager::GetSingleton().Log("INFO", s, __FUNCTION__, __FILE__, __LINE__); \
	} \
	while (0) \

#define LOG(tag, str) \
	do \
	{ \
		std::string s((str)); \
		std::string t((tag)); \
		logger::LogManager::GetSingleton().Log(t, s, __FUNCTION__, __FILE__, __LINE__); \
	} \
	while (0) \

#else
#define LOG_ERROR(str) do { (void)sizeof(str); } while(0) 
#define LOG_WARNING(str) do { (void)sizeof(str); } while(0) 
#define LOG_INFO(str) do { (void)sizeof(str); } while(0) 
#define LOG(tag, str) do { (void)sizeof(tag); (void)sizeof(str); } while(0) 

#endif // _DEBUG

namespace logger
{
	class LogManager : public core::Singleton<LogManager>
	{
	public:
		static void CreateInstance(const char* szConfigFilename);

		~LogManager() = default;
		void Log(const std::string& szTag, const std::string& szMessage, const char* szFunctionName, const char* szSourceFile, unsigned int uiLineNum);
	private:
		LogManager();
		LogManager(const LogManager&) = delete;
		LogManager& operator=(const LogManager&) = delete;
		LogManager(LogManager&&) = delete;
		LogManager& operator=(LogManager&&) = delete;

		void Init(const char* szConfigFilename);
		void CreateLogFilename(std::string szLogFilenameTemplate);
		void SetOutputFlags(const std::string& szTag, unsigned char ucFlags);
		void CreateOutputBuffer(std::string& szOutputBuffer, const std::string& szTag, const std::string& szMessage, const char* szFunctionName, const char* szSourceFile, unsigned int uiLineNum);
		void WriteBufferToLogs(const std::string& szBuffer, unsigned char ucFlags);
		void WriteToLogFile(const std::string& szData);

		typedef std::map<std::string, unsigned char> TagsMap;
		TagsMap m_oTagsMap;
		std::string m_szLogFilename;
	};
}