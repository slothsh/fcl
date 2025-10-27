module;

#include "Common.hpp"

export module Networking:Async.ConnectionHandler;

import :Async.Scheduler;
import Memory;
import std;

export class ConnectionHandler {
public:
    static constexpr std::string_view SOCKET_PATH = SOCKET_PATH_STR;
    static constexpr std::size_t CONNECTION_BUFFER_SIZE = 1024;
    static constexpr int MAX_LISTEN_QUEUE = 4;
    static constexpr std::size_t MAX_WORKER_THREADS = 4;

    using CharType = char;
    using HandleType = int;
    using SocketAddressType = sockaddr_un;
    using SchedulerType = std::shared_ptr<Scheduler>;
    using BufferType = std::array<CharType, CONNECTION_BUFFER_SIZE>;
    using StringType = std::basic_string<CharType, std::char_traits<CharType>, ArenaAllocator<CharType>>;

    ConnectionHandler();
    ~ConnectionHandler();

    void listen();
    void handleConnection(HandleType client_handle) noexcept;

private:
    SchedulerType m_scheduler;
    HandleType m_socket;
    SocketAddressType m_socket_address;
    HandleType m_socket_address_handle;
};
