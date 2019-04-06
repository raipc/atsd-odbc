#include "web_socket_connection.h"
#include "log/log.h"

WebSocketConnection::WebSocketConnection(Poco::Net::HTTPRequest *request_, Poco::Net::HTTPClientSession *session_) :
        response(), request(request_), session(session_), webSocket(*session, *request, response) {}

WebSocketConnection::~WebSocketConnection() {
    LOG("WebSocketConnection destroyed");
}

bool WebSocketConnection::isClosed() const {
    return closed;
}

void WebSocketConnection::close() {
    if (!closed) {
        try {
            webSocket.close();
        } catch (const std::exception &e) {
            LOG("Close websocket error: " << e.what());
        }
        closed = true;
    }
}

void WebSocketConnection::send(std::string query) {
    webSocket.sendFrame(query.data(), (int) query.size(), Poco::Net::WebSocket::FRAME_TEXT);
    queryCount++;
}

void WebSocketConnection::checkError() {
    std::string error;
    bool first = queryCount == 1;
    while (first || webSocket.available() > 0) {
        first = false;
        char buffer[1024] = {};
        int flags;
        int n = webSocket.receiveFrame(buffer, sizeof(buffer), flags);
        if (n > 1) { // error message received
            std::string e(buffer, n);
            LOG("Error received: " << e);
            if (error.empty()) { // show only first error
                error = e;
            }
        }
    }
    if (!error.empty()) {
        throw std::runtime_error(error);
    }
}