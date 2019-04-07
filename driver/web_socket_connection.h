#pragma once

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/WebSocket.h>
#include <memory>
#include <string>

class WebSocketConnection {
public:
    WebSocketConnection(Poco::Net::HTTPRequest *request, Poco::Net::HTTPClientSession *session);

    void send(std::string query);

    void checkError();

    void close();

    bool isClosed() const;

private:
    Poco::Net::HTTPResponse response;
    std::unique_ptr <Poco::Net::HTTPRequest> request;
    std::unique_ptr <Poco::Net::HTTPClientSession> session;
    Poco::Net::WebSocket webSocket;
    bool closed = false;
    int queryCount = 0;
};
