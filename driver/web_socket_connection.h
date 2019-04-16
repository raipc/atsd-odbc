#pragma once

#include "long_running_connection.h"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/WebSocket.h>
#include <memory>
#include <string>

class WebSocketConnection : public LongRunningConnection {
public:
    WebSocketConnection(Poco::Net::HTTPRequest *request, Poco::Net::HTTPClientSession *session);

    virtual void send(std::string query);

    virtual void checkError();

    virtual void close();

    virtual bool isClosed() const;

private:
    Poco::Net::HTTPResponse response;
    std::unique_ptr <Poco::Net::HTTPRequest> request;
    std::unique_ptr <Poco::Net::HTTPClientSession> session;
    Poco::Net::WebSocket webSocket;
    bool closed = false;
    int queryCount = 0;
};
