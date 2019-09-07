#pragma once

#include <unordered_map>
#include <functional>
#include <bridgexpc/connection.h>
#include <plistpp2/plist.h>

enum class BkEnvelopeType {
    PING = 0,
    MESSAGE = 1
};

class BkConnection {

public:
    using CompletionCallback = std::function<void (int status, plist::object const &data)>;

private:
    struct bridge_xpc_connection *conn;
    std::unordered_map<uint64_t, CompletionCallback> callbacks;
    uint64_t nextRequestId = 0;

    void onConnected();

    void onMessage(plist::object const &msg);

    void sendRaw(BkEnvelopeType type, bool isReply, std::string msgId, plist::object data);

public:
    BkConnection(struct bridge_xpc_connection *conn);

    void send(int cmd, plist::object data, CompletionCallback cb);

    void sendPing(std::function<void ()> cb);

};
