#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "Config.hpp"

namespace cli {

  class Service
  {
  public:
    ~Service();

  protected:
    Service();

  public:
    static Service& instance();

    asio::io_service& ios();
        
  private:
    asio::io_service m_IOS;
    asio::io_service::work m_Work;
    std::thread m_Worker;
  };

} //namespace cli

#endif //SERVICE_HPP
