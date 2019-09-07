#include <cstdio>
#include "BkConnection.h"

#define BK_NULL "d4161201-daf5-4bbd-ae4f-9bf319fabbe0"

BkConnection::BkConnection(struct bridge_xpc_connection *conn) : conn(conn) {
    struct bridge_xpc_connection_callbacks cbs;
    cbs.connected = +[](struct bridge_xpc_connection *, void *userdata) {
        ((BkConnection *) userdata)->onConnected();
    };
    cbs.message_received = +[](struct bridge_xpc_connection *, plist_t data, void *userdata) {
        plist::object o (data);
        ((BkConnection *) userdata)->onMessage(o);
        o.release(); // don't free it
    };
    bridge_xpc_connection_set_callbacks(conn, &cbs, this);
}

void BkConnection::sendRaw(BkEnvelopeType type, bool isReply, std::string msgId, plist::object data) {
    auto arr = plist::array();
    arr.append((uint64_t) type);
    arr.append(isReply);
    arr.append(msgId);
    arr.append(data);
    bridge_xpc_connection_send(conn, arr.plist_ptr());
}

void BkConnection::onConnected() {
    printf("BkConnection::onConnected\n");
}

void BkConnection::onMessage(plist::object const &msg) {
    printf("BkConnection::onMessage\n");
    if (msg.type() != PLIST_ARRAY)
        throw std::runtime_error("root container not an array");
    if (msg.size() != 4)
        throw std::runtime_error("root array not of size 4");

    auto type = (BkEnvelopeType) msg.at(0).get<uint64_t>();
    bool isReply = msg.at(1).get<bool>();
    auto msgId = msg.at(2).get<std::string>();
    auto data = msg.at(3);

    if (type == BkEnvelopeType::PING) {
        if (isReply)
            throw std::runtime_error("got invalid message: ping can't be a reply");
        if (msgId != BK_NULL) {
            sendRaw(BkEnvelopeType::MESSAGE, true, msgId, plist::integer(1));
        }
    }
}