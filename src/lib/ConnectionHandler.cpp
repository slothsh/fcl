#include <array>
#include <memory>
#include <print>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <unistd.h>
#include "Scheduler.hpp"

namespace detail {
    using SchedulerType = typename ConnectionHandler::SchedulerType;
    using HandleType = typename ConnectionHandler::HandleType;
    using StringType = typename ConnectionHandler::StringType;
}

ConnectionHandler::ConnectionHandler()
    : m_scheduler{std::make_shared<Scheduler>(ConnectionHandler::MAX_WORKER_THREADS)}
{
    m_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (m_socket == -1) {
        std::println("socket failed");
        return;
    }

    unlink(ConnectionHandler::SOCKET_PATH.data());

    m_socket_address = {
        .sun_family = AF_LOCAL,
        .sun_path = SOCKET_PATH_STR
    };

    m_socket_address_handle = bind(m_socket, reinterpret_cast<sockaddr*>(&m_socket_address), sizeof(m_socket_address));
    if (m_socket_address_handle == -1) {
        std::println("socket address failed");
        return;
    }
}

ConnectionHandler::~ConnectionHandler() {
    if (m_socket) {
        close(m_socket);
    }
    unlink(ConnectionHandler::SOCKET_PATH.data());
}

void ConnectionHandler::listen() {
    auto listen_result = ::listen(m_socket, ConnectionHandler::MAX_LISTEN_QUEUE);
    if (listen_result == -1) {
        std::println("listen failed");
        return;
    }

    while (true) {
        auto client_handle = accept(m_socket, nullptr, nullptr);

        if (client_handle == -1) {
            std::println("Client error");
            continue;
        }

        this->handleConnection(client_handle);
    }
}

void ConnectionHandler::handleConnection(detail::HandleType client_handle) noexcept {
    using String = detail::StringType;
    String message{};
    std::array<char, 1024> buffer{};

    ssize_t bytes_received = 0;
    do {
        std::println("Received: {}", bytes_received);
        message.append(buffer.begin(), bytes_received);
        bytes_received = recv(client_handle, buffer.data(), buffer.size(), 0);
    } while (bytes_received > 0);

    // msgpack::object_handle object_handle = msgpack::unpack(message.data(), message.size());
    //
    // std::println("Client message: {}", object_handle.get().as<std::string_view>());

    close(client_handle);
    message.clear();
}
