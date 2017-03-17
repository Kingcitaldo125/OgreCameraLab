#include <stdafx.h>
#include <LogManager.h>

// Template specialization to initialize and declare the singleton pointer for LogManager
template<>
ssurge::LogManager * ssurge::Singleton<ssurge::LogManager>::msSingleton = nullptr;

ssurge::LogManager::LogManager(std::string fname) : mLogMask(LL_DEBUG | LL_ERROR | LL_NORMAL | LL_WARNING),
        mFile(fname)
{
	// Intentionally left empty.
}

ssurge::LogManager::~LogManager()
{
	if (mFile.is_open())
		mFile.close();
}

void ssurge::LogManager::log(std::string message, unsigned int lvl)
{
	if (mFile.is_open() && (lvl & mLogMask))
	{
		// Reference: http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
		time_t t = time(NULL);
		struct tm now;
		localtime_s(&now, &t);
		mFile << "[" << now.tm_mday << "/" << now.tm_mon + 1 << "/" << now.tm_year + 1900;
		mFile << "@" << now.tm_hour << ":" << now.tm_min << ":" << now.tm_sec << "] ";
		switch (lvl)
		{
			case LL_DEBUG:		mFile << "[DEBUG] ";	break;
			case LL_ERROR:		mFile << "[ERROR] ";	break;
			case LL_NORMAL:								break;
			case LL_WARNING:	mFile << "[WARNING] ";	break;
			default:			mFile << "[???] ";		break;
		}
		mFile << message << std::endl;
	}
}

void ssurge::LogManager::setLogMask(unsigned int lvl_mask)
{
	mLogMask = lvl_mask;
}