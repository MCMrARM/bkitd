#include <cstdio>
#include <cstring>
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

void BkConnection::send(BkCommand cmd, plist::object data, CompletionCallback cb) {
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
    if (connectedCallback)
        connectedCallback();
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
            cb->second(data);
            callbacks.erase(cb);
        }
    }
}


void BkConnection::getBridgeVersion(std::function<void (uint64_t version)> cb, ErrorCallback err) {
    send(BkCommand::GetBridgeVersion, plist::array(), [cb, err](plist::object data) {
        if (data.size() != 2)
            err(BkErrorCode::InvalidValue);
        else if (data.at(0).get<uint64_t>() != 0)
            err((BkErrorCode) data.at(0).get<uint64_t>());
        else
            cb(data.at(1).get<uint64_t>());
    });
}

void BkConnection::performCommand(void *data, size_t len, size_t replySize,
        std::function<void(void *data, size_t len)> cb, BkConnection::ErrorCallback err) {
    auto args = plist::array();
    args.append(plist::integer(0));
    args.append(plist::data(data, len));
    args.append(plist::integer(replySize));
    send(BkCommand::PerformCommand, args, [cb, err](plist::object data) {
        if (data.size() != 2)
            err(BkErrorCode::InvalidValue);
        else if (data.at(0).get<uint64_t>() != 0)
            err((BkErrorCode) data.at(0).get<uint64_t>());
        else if (data.at(1).type() == PLIST_STRING && !strcmp(data.at(1).get<const char *>(), BK_NULL))
            cb(NULL, 0);
        else if (data.at(1).type() != PLIST_DATA)
            err(BkErrorCode::InvalidValue);
        else
            cb(data.at(1).data_ptr(), data.at(1).size());
    });
}

void BkConnection::getCalibrationData(BkCommand cmd, std::function<void(void *data, size_t len)> cb,
        BkConnection::ErrorCallback err) {
    send(cmd, plist::array(), [cb, err](plist::object data) {
        if (data.size() != 1)
            err(BkErrorCode::InvalidValue);
        else if (data.at(0).type() == PLIST_STRING && !strcmp(data.at(0).get<const char *>(), BK_NULL))
            cb(NULL, 0);
        else if (data.at(0).type() != PLIST_DATA)
            err(BkErrorCode::InvalidValue);
        else
            cb(data.at(0).data_ptr(), data.at(0).size());
    });
}

void BkConnection::getCalibrationDataFromEEPROM(std::function<void(void *data, size_t len)> cb,
        BkConnection::ErrorCallback err) {
    getCalibrationData(BkCommand::GetCalibrationDataFromEEPROM, std::move(cb), std::move(err));
}

void BkConnection::getCalibrationDataFromFDR(std::function<void(void *data, size_t len)> cb,
        BkConnection::ErrorCallback err) {
    getCalibrationData(BkCommand::GetCalibrationDataFromFDR, std::move(cb), std::move(err));
}