#pragma once

#if defined USE_LIB_LOGGING
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>

class SimpleSmallLogger
{
public:
    log4cpp::Category& catRoot() { return log4cpp::Category::getRoot(); }
    static SimpleSmallLogger instance();

    void setLoggingLevel(log4cpp::Priority::PriorityLevel lvl);

    void appendCategory(const std::string catName);
    log4cpp::Category category(const std::string &catName);

    void addFileAppender(const std::string& fileNamePath);
private:
    SimpleSmallLogger();
};

//#define LOG_ADD_CATEGORY SimpleSmallLogger::instance().

#define LOG_ERROR SimpleSmallLogger::instance().catRoot().errorStream()
#define LOG_WARN  SimpleSmallLogger::instance().catRoot().warnStream()
#define LOG_INFO SimpleSmallLogger::instance().catRoot().infoStream()
#define LOG_DEBUG SimpleSmallLogger::instance().catRoot().debugStream()
#else
#include <iostream>

class LoggerEndl
{
public:
    LoggerEndl(std::ostream& systemStreambuf);
    ~LoggerEndl();
    std::ostream& operator()() const;

private:
    std::ostream& _stream;
};

#define LOG_ERROR LoggerEndl(std::cout)()
#define LOG_INFO LoggerEndl(std::cout)()
#define LOG_WARN LoggerEndl(std::cerr)()
#define LOG_DEBUG LoggerEndl(std::cerr)()
#endif


//#define LOG_CAT_ERROR(x) SimpleSmallLogger::instance.category(x).errorStream
