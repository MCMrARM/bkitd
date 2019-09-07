#pragma once

#include <bridgexpc/connection.h>
#include <plistpp2/plist.h>

enum class BkEnvelopeType {
    PING = 0,
    MESSAGE = 1
};

class BkConnection {

private:
    struct bridge_xpc_connection *conn;

    void onConnected();

    void onMessage(plist::object const &msg);

    void sendRaw(BkEnvelopeType type, bool isReply, std::string msgId, plist::object data);

public:
    BkConnection(struct bridge_xpc_connection *conn);

};
