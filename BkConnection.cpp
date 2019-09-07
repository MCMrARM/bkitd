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

void BkConnection::send(int cmd, plist::object data, std::function<void(int, const plist::object &)> cb) {
    uint64_t reqId = nextRequestId++;
    callbacks.emplace(reqId, std::move(cb));
    data.insert(0, (uint64_t) cmd);
    sendRaw(BkEnvelopeType::MESSAGE, false, "R" + std::to_string(reqId), data);
}

void BkConnection::sendPing(std::function<void ()> cb) {
    uint64_t reqId = nextRequestId++;
    callbacks.emplace(reqId, std::bind(cb));
    sendRaw(BkEnvelopeType::PING, false, "R" + std::to_string(reqId), plist::integer(0));
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
    } else if (type == BkEnvelopeType::MESSAGE) {
        if (isReply) {
            if (msgId.length() < 2 || msgId[0] != 'R')
                throw std::runtime_error("got invalid message: reply id must start with R");
            auto cb = callbacks.find(std::strtol(&msgId[1], nullptr, 10));
            if (cb == callbacks.end())
                throw std::runtime_error("got invalid message: reply id not found");
            int result = -1;
            if (data.type() == PLIST_ARRAY && data.size() > 0)
                result = (int) (uint64_t) data.at(0).get<uint64_t>();
            cb->second(result, data);
            callbacks.erase(cb);
        }
    }
}