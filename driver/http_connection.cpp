#include "http_connection.h"
#include "log/log.h"
#include <sstream>

HttpConnection::HttpConnection(Poco::Net::HTTPRequest *request_, Poco::Net::HTTPClientSession *session_) :
        response(), request(request_), session(session_) {
    out = &session->sendRequest(*request);
}

bool HttpConnection::isClosed() const {
    return closed;
}

void HttpConnection::close() {
    if (!closed) {
        try {
            if(!in){
                in = &session->receiveResponse(response);
                std::stringstream os;
                os << in->rdbuf();
                error = os.str();
                errorReceived = !error.empty();
            }
            session->reset();
        } catch (const std::exception &e) {
            LOG("Close http connection error: " << e.what());
        }
        closed = true;
    }
}

void HttpConnection::send(std::string query) {
    *out << query << "\n";
    out->flush();
}

void HttpConnection::checkError() {
    if (errorReceived) {
        throw std::runtime_error(error);
    }
}