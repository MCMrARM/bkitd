#pragma once

#include <bridgexpc/connection.h>
#include <plist/plist++.h>

class BkConnection {

private:
    struct bridge_xpc_connection *conn;

    void onConnected();

    void onMessage(PList::Node *node);

public:
    BkConnection(struct bridge_xpc_connection *conn);

};
