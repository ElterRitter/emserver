#include "smlog.h"

#if defined USE_LIB_LOGGING
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/PatternLayout.hh>

#if defined(OS_WIN)
#if defined(DEBUG_BUILD)
#include <log4cpp/Win32DebugAppender.hh>
#endif
#endif

SimpleSmallLogger SimpleSmallLogger::instance()
{
    static SimpleSmallLogger instance;
    return instance;
}

void SimpleSmallLogger::setLoggingLevel(log4cpp::Priority::PriorityLevel lvl)
{
    auto &root = log4cpp::Category::getRoot();
    root.setPriority(lvl);
}

void SimpleSmallLogger::addFileAppender(const std::string &fileNamePath)
{
    auto &root = log4cpp::Category::getRoot();
    auto currentAppender = root.getAppender("DebugFileAppender");
    if(currentAppender != nullptr) // already exists
        return;

    log4cpp::Appender *pFileAppender = new log4cpp::FileAppender("DebugFileAppender", fileNamePath, true);
    auto pla = new log4cpp::PatternLayout();
    pla->setConversionPattern("%d %c [%p] %m%n");
    pFileAppender->setLayout(pla);
    root.addAppender(pFileAppender);
}

SimpleSmallLogger::SimpleSmallLogger()
{
    auto &root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::DEBUG);

    log4cpp::Appender *console_appender = new log4cpp::OstreamAppender("console", &std::cout);
    auto pla = new log4cpp::PatternLayout();
    pla->setConversionPattern("%d %c [%p] %m%n");
    console_appender->setLayout(pla);
    root.addAppender(console_appender);

#if defined(OS_WIN)
#if defined(DEBUG_BUILD)
    log4cpp::Appender *debug_appender = new log4cpp::Win32DebugAppender("w32debug");
    pla = new log4cpp::PatternLayout();
    pla->setConversionPattern("%d %c [%p] %m%n");
    debug_appender->setLayout(pla);
    root.addAppender(debug_appender);
#endif
#endif
}
#else
#include <chrono>
#include <iomanip>

LoggerEndl::LoggerEndl(std::ostream& systemStreambuf) :
    _stream(systemStreambuf)
{
    std::chrono::system_clock::time_point tpoint = std::chrono::system_clock::now();
    time_t tmp = std::chrono::system_clock::to_time_t(tpoint);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(tpoint.time_since_epoch() % std::chrono::seconds(1));
    _stream << "EmserverProxy " << std::put_time(std::localtime(&tmp), "%F %T") << "." << milliseconds.count();
}

LoggerEndl::~LoggerEndl()
{
    _stream << std::endl;
}

std::ostream& LoggerEndl::operator()() const
{
    return _stream;
}
#endif
