#pragma once

#include <string>

class LongRunningConnection {
public:
    virtual void send(std::string query) = 0;

    virtual void checkError() = 0;

    virtual void close() = 0;

    virtual bool isClosed() const = 0;
};
