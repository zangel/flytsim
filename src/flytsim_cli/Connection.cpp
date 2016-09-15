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
          command->writeRequest(m_Stream);
          command->readResponseResult(m_Stream);
          command->readResponseData(m_Stream);
          
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

} //namespace cli
