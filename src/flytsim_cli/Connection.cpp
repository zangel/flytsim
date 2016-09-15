#include "Connection.hpp"
#include "Service.hpp"
#include "ResponseParser.hpp"
#include "Commands.hpp"

namespace cli {

  Connection::Connection()
    : m_Socket(Service::instance().ios())
    , m_GetBuffer()
    , m_PutBuffer()
    , m_Stream(this)
    , m_ProcessCommands(false)
    , m_ProcessCommandsStrand(Service::instance().ios())
    , m_ProcessCommandsYieldContext(nullptr)
    , m_CommandsBufferMutex()
    , m_CommandsBufferDeque()
    , m_CommandsBufferSignal(Service::instance().ios())
  {
    initBuffers();
  }

  Connection::~Connection()
  {

  }

  system::error_code Connection::connect(std::string const &address)
  {
    if(m_Socket.is_open())
      return make_error_code(system::errc::already_connected);

    asio::ip::tcp::endpoint remoteEP(asio::ip::address::from_string(address), SERVER_PORT);

    system::error_code err;
    m_Socket.connect(remoteEP, err);

    if(err)
      return err;

    initBuffers();
    startProcessingCommands();
    
    return system::error_code();
  }

  bool Connection::isConnected() const
  {
    return m_Socket.is_open();
  }

  void Connection::disconnect()
  {
    if(!m_Socket.is_open())
      return;

    system::error_code err;

    m_Socket.cancel(err);
    m_Socket.shutdown(asio::ip::tcp::socket::shutdown_both, err);
    m_Socket.close(err);
    stopProcessingCommands();
  }

  system::error_code Connection::asyncSendCommand(std::shared_ptr<cmd::Command> command)
  {
    if(!m_Socket.is_open())
      return make_error_code(system::errc::not_connected);

    std::lock_guard<std::mutex> lock(m_CommandsBufferMutex);
    m_CommandsBufferDeque.push_back(command);
    system::error_code err;
    m_CommandsBufferSignal.cancel(err);
    return system::error_code();
  }

  /*
  void Connection::asyncArm(CommandHandler handler)
  {
    if(!isConnected())
    {
      if(handler)
        handler(make_error_code(system::errc::not_connected));

      return;
    }

    Service::instance().ios().post([this, handler]()
    {
      std::size_t out_size0 = m_RequestBuffer.size();
      std::ostream request(&m_RequestBuffer);
      request << "arm\r\n";
      request.flush();
      std::size_t out_size1 = m_RequestBuffer.size();

      asio::async_write(
        m_Socket,
        m_RequestBuffer,
        boost::bind(
          &Connection::handleWriteCommandRequest, shared_from_this(),
          handler,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
        )
      );
    });
  }


  void Connection::asyncDisarm(CommandHandler handler)
  {
    if(!isConnected())
    {
      if(handler)
        handler(make_error_code(system::errc::not_connected));

      return;
    }

    Service::instance().ios().post([this, handler]()
    {
      std::ostream request(&m_RequestBuffer);
      request << "disarm\r\n";
      request.flush();

      asio::async_write(
        m_Socket,
        m_RequestBuffer,
        boost::bind(
          &Connection::handleWriteCommandRequest, shared_from_this(),
          handler,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
        )
      );
    });
  }

  void Connection::asyncTakeOff(float altitude, CommandHandler handler)
  {
    if(!isConnected())
    {
      if(handler)
        handler(make_error_code(system::errc::not_connected));

      return;
    }

    Service::instance().ios().post([this, handler, altitude]()
    {
      std::ostream request(&m_RequestBuffer);
      request << "take_off altitude:" << altitude << "\r\n";
      request.flush();

      asio::async_write(
        m_Socket,
        m_RequestBuffer,
        boost::bind(
          &Connection::handleWriteCommandRequest, shared_from_this(),
          handler,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
        )
      );
    });
  }

  void Connection::asyncLand(bool async, CommandHandler handler)
  {
    if(!isConnected())
    {
      if(handler)
        handler(make_error_code(system::errc::not_connected));

      return;
    }

    Service::instance().ios().post([this, async, handler]()
    {
      std::ostream request(&m_RequestBuffer);
      request << "land async:" << (async?"true":"false") << "\r\n";
      request.flush();

      asio::async_write(
        m_Socket,
        m_RequestBuffer,
        boost::bind(
          &Connection::handleWriteCommandRequest, shared_from_this(),
          handler,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
        )
      );
    });
  }

  void Connection::asyncPositionSetpoint(float x, float y, float z, optional<float> yaw, bool relative, bool body_frame, CommandHandler handler)
  {
    if(!isConnected())
    {
      if(handler)
        handler(make_error_code(system::errc::not_connected));

      return;
    }

    Service::instance().ios().post([this, x, y, z, yaw, relative, body_frame, handler]()
    {
      std::ostream request(&m_RequestBuffer);
      request << "position_setpoint position:{" << x << "," << y << "," << z << "}";
      if(yaw)
      {
        request << " yaw:" << yaw.get();
      }

      request <<
        " relative:" << (relative ? "true" : "false") <<
        " body_frame:" << (body_frame ? "true" : "false");

      request << "\r\n";


      request.flush();

      asio::async_write(
        m_Socket,
        m_RequestBuffer,
        boost::bind(
          &Connection::handleWriteCommandRequest, shared_from_this(),
          handler,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
        )
      );
    });
  }

  void Connection::asyncVelocitySetpoint(float x, float y, float z, optional<float> yaw_rate, bool relative, bool body_frame, CommandHandler handler)
  {
    if(!isConnected())
    {
      if(handler)
        handler(make_error_code(system::errc::not_connected));

      return;
    }

    Service::instance().ios().post([this, x, y, z, yaw_rate, relative, body_frame, handler]()
    {
      std::size_t out_size0 = m_RequestBuffer.size();
      std::ostream request(&m_RequestBuffer);
      request << "velocity_setpoint velocity:{" << x << "," << y << "," << z << "}";
      if(yaw_rate)
      {
        request << " yaw_rate:" << yaw_rate.get();
      }

      request <<
        " relative:" << (relative ? "true" : "false") <<
        " body_frame:" << (body_frame ? "true" : "false");

      request << "\r\n";


      request.flush();
      std::size_t out_size1 = m_RequestBuffer.size();

      asio::async_write(
        m_Socket,
        m_RequestBuffer,
        boost::bind(
          &Connection::handleWriteCommandRequest, shared_from_this(),
          handler,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
        )
      );
    });
  }
  */

  void Connection::startProcessingCommands()
  {
    system::error_code err;
    m_CommandsBufferSignal.expires_at(asio::steady_timer::time_point::max(), err);
    m_CommandsBufferDeque.clear();

    m_ProcessCommands = true;
    asio::spawn(
      m_ProcessCommandsStrand,
      std::bind(
        &Connection::processCommands,
        shared_from_this(),
        std::placeholders::_1
      )
    );
  }

  void Connection::stopProcessingCommands()
  {
    m_ProcessCommands = false;
    system::error_code err;
    m_CommandsBufferSignal.cancel(err);
    while(m_ProcessCommandsYieldContext)
      std::this_thread::yield();
  }

  void Connection::processCommands(asio::yield_context yctx)
  {
    m_ProcessCommandsYieldContext = &yctx;
    while(m_ProcessCommands)
    {
      system::error_code err;
      m_CommandsBufferSignal.async_wait(yctx[err]);

      if(err && err == asio::error::operation_aborted)
      {
        while(true)
        {
          std::shared_ptr<cmd::Command> command;
          {
            std::lock_guard<std::mutex> lock(m_CommandsBufferMutex);
            if(m_CommandsBufferDeque.empty())
            {
              break;
            }
            else
            {
              command = m_CommandsBufferDeque.front();
            }
          }

          //here handle the command
          std::chrono::system_clock::time_point beginReq = std::chrono::system_clock::now();
          command->writeRequest(m_Stream);
          std::chrono::system_clock::time_point endReq1 = std::chrono::system_clock::now();
          float sec1 = std::chrono::duration_cast<std::chrono::microseconds>(endReq1 - beginReq).count() / 1.0e+6f;
          command->readResponseResult(m_Stream);
          std::chrono::system_clock::time_point endReq2 = std::chrono::system_clock::now();
          float sec2 = std::chrono::duration_cast<std::chrono::microseconds>(endReq2 - endReq1).count() / 1.0e+6f;
          command->readResponseData(m_Stream);
          std::chrono::system_clock::time_point endReq = std::chrono::system_clock::now();
          float sec3 = std::chrono::duration_cast<std::chrono::microseconds>(endReq - endReq2).count() / 1.0e+6f;
          float sec = std::chrono::duration_cast<std::chrono::microseconds>(endReq - beginReq).count() / 1.0e+6f;
          
          //std::ostringstream oss;
          //oss << "sec:" << sec << " sec3:" << sec3 << std::endl;

          std::lock_guard<std::mutex> lock(m_CommandsBufferMutex);
          m_CommandsBufferDeque.pop_front();
        }
      }

      std::lock_guard<std::mutex> lock(m_CommandsBufferMutex);
      m_CommandsBufferSignal.expires_at(asio::steady_timer::time_point::max(), err);
    }
    m_ProcessCommandsYieldContext = nullptr;
  }

  system::error_code Connection::readLine(std::string &line)
  {
    return cmd::Command::readLine(m_Stream, line);
  }

  void Connection::initBuffers()
  {
    setg(&m_GetBuffer[0], &m_GetBuffer[0] + PUTBACK_MAX, &m_GetBuffer[0] + PUTBACK_MAX);
    setp(&m_PutBuffer[0], &m_PutBuffer[0] + m_PutBuffer.size());
  }

  Connection::int_type Connection::underflow()
  {
    BOOST_ASSERT(m_ProcessCommandsYieldContext);
    if(!m_ProcessCommands)
      return traits_type::eof();

    if(gptr() == egptr())
    {
      system::error_code err;
      std::size_t bytes = m_Socket.async_read_some(
        asio::buffer(asio::buffer(m_GetBuffer) + PUTBACK_MAX),
        (*m_ProcessCommandsYieldContext)[err]
      );

      if(err)
      {
        if(!bytes)
          return traits_type::eof();
      }

      setg(&m_GetBuffer[0], &m_GetBuffer[0] + PUTBACK_MAX, &m_GetBuffer[0] + PUTBACK_MAX + bytes);
      return traits_type::to_int_type(*gptr());
    }
    else
    {
      return traits_type::eof();
    }
  }

  Connection::int_type Connection::overflow(int_type c)
  {
    BOOST_ASSERT(m_ProcessCommandsYieldContext);

    if(!m_ProcessCommands)
      return traits_type::eof();

    asio::const_buffer buffer = asio::buffer(pbase(), pptr() - pbase());

    while(asio::buffer_size(buffer) > 0)
    {

      system::error_code err;
      std::size_t bytes = asio::async_write(
        m_Socket,
        asio::buffer(buffer),
        (*m_ProcessCommandsYieldContext)[err]
      );

      if(err)
      {
        if(!bytes)
          return traits_type::eof();
      }

      buffer = buffer + bytes;
    }
    setp(&m_PutBuffer[0], &m_PutBuffer[0] + m_PutBuffer.size());

    if(traits_type::eq_int_type(c, traits_type::eof()))
      return traits_type::not_eof(c);

    *pptr() = traits_type::to_char_type(c);
    pbump(1);
    return c;
  }

  int Connection::sync()
  {
    return overflow(traits_type::eof());
  }

  /*
  void Connection::handleWriteCommandRequest(CommandHandler handler, system::error_code const &e, std::size_t bytes)
  {
    if(e || !bytes)
    {
      if(handler)
        handler(make_error_code(system::errc::io_error));

      return;
    }
    std::size_t out_size2 = m_RequestBuffer.size();

    std::size_t in_size0 = m_ResponseBuffer.size();
    asio::async_read_until(
      m_Socket,
      m_ResponseBuffer,
      "\r\n",
      boost::bind(
        &Connection::handleReadCommandResponseResult, shared_from_this(),
        handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred
      )
    );
  }

  void Connection::handleReadCommandResponseResult(CommandHandler handler, system::error_code const &e, std::size_t bytes)
  {
    if(e || bytes < 2)
    {
      if(handler)
        handler(make_error_code(system::errc::io_error));

      return;
    }

    std::size_t in_size1 = m_ResponseBuffer.size();
    std::string line;
    line.resize(bytes - 2);
    m_ResponseBuffer.sgetn(const_cast<char*>(line.c_str()), bytes - 2);
    std::size_t in_size2 = m_ResponseBuffer.size();
    m_ResponseBuffer.consume(2);
    std::size_t in_size3 = m_ResponseBuffer.size();
    
    response::Result result;
    
    if(system::error_code pe = response::parseResult(line, result))
    {
      if(handler)
        handler(pe);
      return;
    }

    if(handler)
      handler(make_error_code(system::errc::errc_t(result.result)));

    asio::async_read_until(
      m_Socket,
      m_ResponseBuffer,
      "\r\n",
      boost::bind(
        &Connection::handleReadCommandResponseData, shared_from_this(),
        handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred
      )
    );
  }

  void Connection::handleReadCommandResponseData(CommandHandler handler, system::error_code const &e, std::size_t bytes)
  {
    if(e || bytes < 2)
    {
      return;
    }

    std::size_t in_size4 = m_ResponseBuffer.size();
    std::string line;
    line.resize(bytes - 2);
    m_ResponseBuffer.sgetn(const_cast<char*>(line.c_str()), bytes - 2);
    std::size_t in_size5 = m_ResponseBuffer.size();
    m_ResponseBuffer.consume(2);
    std::size_t in_size6 = m_ResponseBuffer.size();

  }
  */

} //namespace cli
