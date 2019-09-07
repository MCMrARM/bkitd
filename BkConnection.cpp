#include <cstdio>
#include "BkConnection.h"

BkConnection::BkConnection(struct bridge_xpc_connection *conn) : conn(conn) {
    struct bridge_xpc_connection_callbacks cbs;
    cbs.connected = +[](struct bridge_xpc_connection *, void *userdata) {
        ((BkConnection *) userdata)->onConnected();
    };
    cbs.message_received = +[](struct bridge_xpc_connection *, plist_t data, void *userdata) {
        // HACK: the code uses parent only to decide whether to free the plist, do just pass it some value to make it
        // happy
        PList::Dictionary tmp_dict;
        ((BkConnection *) userdata)->onMessage(PList::Node::FromPlist(data, &tmp_dict));
    };
    bridge_xpc_connection_set_callbacks(conn, &cbs, this);
}

void BkConnection::onConnected() {
    printf("BkConnection::onConnected\n");
}

void BkConnection::onMessage(PList::Node *node) {
    printf("BkConnection::onMessage\n");
}