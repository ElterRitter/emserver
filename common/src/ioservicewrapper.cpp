#include "ioservicewrapper.h"
#include <boost/thread/thread.hpp>

#include <smlog.h>

IoServiceWrapper::IoServiceWrapper() :
    m_service{ new boost::asio::io_service() }
    , m_ioWork{ new boost::asio::io_service::work(*m_service) }
    , m_workThread{ new std::thread(boost::bind(&boost::asio::io_service::run, m_service)) }
{
}

IoServiceWrapper::~IoServiceWrapper()
{
    delete m_ioWork;
    m_workThread->join();
    delete m_workThread;
    delete m_service;

}
