#include "Service.hpp"

namespace cli {
  
  Service::Service()
    : m_IOS()
    , m_Work(m_IOS)
    , m_Worker([this]() { m_IOS.run(); })
  {
    m_Worker.detach();
  }

  Service::~Service()
  {
    m_IOS.stop();
  }

  Service& Service::instance()
  {
    static Service _instance;
    return _instance;
  }

  asio::io_service& Service::ios()
  {
    return m_IOS;
  }
} //namespace cli
