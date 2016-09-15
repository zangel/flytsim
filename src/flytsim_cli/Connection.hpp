#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Config.hpp"

namespace cli {

  namespace cmd { class Command; }

  class Connection
    : public std::enable_shared_from_this<Connection>
    , protected std::streambuf
  {
  public:

    static std::uint16_t const SERVER_PORT = 12321;

    typedef std::function<void(system::error_code)> CommandHandler;

    Connection();
    ~Connection();

    system::error_code connect(std::string const &address);
    bool isConnected() const;
    void disconnect();

    
    system::error_code asyncSendCommand(std::shared_ptr<cmd::Command> command);
    std::size_t pendingCommandsCount() { return m_CommandsBufferDeque.size(); }
    /*void asyncArm(CommandHandler handler = CommandHandler());
    void asyncDisarm(CommandHandler handler = CommandHandler());
    void asyncTakeOff(float altitude, CommandHandler handler = CommandHandler());
    void asyncLand(bool async = true, CommandHandler handler = CommandHandler());
    void asyncPositionSetpoint(float x, float y, float z, optional<float> yaw = optional<float>(), bool relative = false, bool body_frame = false, CommandHandler handler = CommandHandler());
    void asyncVelocitySetpoint(float x, float y, float z, optional<float> yaw_rate = optional<float>(), bool relative = false, bool body_frame = false, CommandHandler handler = CommandHandler());*/

  private:
    void startProcessingCommands();
    void stopProcessingCommands();
    void processCommands(asio::yield_context yctx);
    

    //void handleWriteCommandRequest(CommandHandler handler, system::error_code const &e, std::size_t bytes);
    //void handleReadCommandResponseResult(CommandHandler handler, system::error_code const &e, std::size_t bytes);
    //void handleReadCommandResponseData(CommandHandler handler, system::error_code const &e, std::size_t bytes);

    system::error_code readLine(std::string &line);

    //std::streambuf implementation
    void initBuffers();
    int_type underflow();
    int_type overflow(int_type c);
    int sync();


  private:
    asio::ip::tcp::socket m_Socket;
    
    enum { PUTBACK_MAX = 8 };
    enum { BUFFER_SIZE = 8192 };

    asio::detail::array<char, BUFFER_SIZE> m_GetBuffer;
    asio::detail::array<char, BUFFER_SIZE> m_PutBuffer;

    std::iostream m_Stream;

    bool m_ProcessCommands;
    asio::strand m_ProcessCommandsStrand;
    asio::yield_context *m_ProcessCommandsYieldContext;

    std::mutex m_CommandsBufferMutex;
    std::deque< std::shared_ptr<cmd::Command> > m_CommandsBufferDeque;
    asio::steady_timer m_CommandsBufferSignal;
  };
  
} //namespace cli

#endif //CONNECTION_HPP
