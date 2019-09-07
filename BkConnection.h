#pragma once

#include <unordered_map>
#include <functional>
#include <bridgexpc/connection.h>
#include <plistpp2/plist.h>

enum class BkEnvelopeType {
    PING = 0,
    MESSAGE = 1
};
enum class BkCommand {
    GetBridgeVersion = 0,
    GetServiceOpened = 1,
    GetSystemBootTime = 2,
    PerformCommand = 3,
    SetIORegistryProperty = 4,
    GetCalibrationDataFromEEPROM = 5,
    MachContinousTime = 6,
    GetMachTimebaseInfo = 7,
    GetOSVersion = 8,
    SetBridgeClientVersion = 10,
    GetCalibrationDataFromFDR = 11
};

enum class BkErrorCode {
    Success = 0,
    // TODO: This isn't a real error, map real device errors?
    InvalidValue = -1
};

class BkConnection {

public:
    using ConnectedCallback = std::function<void ()>;

    using CompletionCallback = std::function<void (plist::object const &data)>;

    using ErrorCallback = std::function<void (BkErrorCode)>;

private:
    struct bridge_xpc_connection *conn;
    ConnectedCallback connectedCallback;
    std::unordered_map<uint64_t, CompletionCallback> callbacks;
    uint64_t nextRequestId = 0;

    void onConnected();

    void onMessage(plist::object const &msg);

    void sendRaw(BkEnvelopeType type, bool isReply, std::string msgId, plist::object data);

    void getCalibrationData(BkCommand cmd, std::function<void (void *data, size_t len)> cb, ErrorCallback err);

public:
    BkConnection(struct bridge_xpc_connection *conn);

    void setConnectedCallback(ConnectedCallback cb) { connectedCallback = std::move(cb); }

    void send(BkCommand cmd, plist::object data, CompletionCallback cb);

    void sendPing(std::function<void ()> cb);


    void getBridgeVersion(std::function<void (uint64_t version)> cb, ErrorCallback err);

    void performCommand(void *data, size_t len, size_t replySize,
            std::function<void (void *data, size_t len)> cb, ErrorCallback err);

    void getCalibrationDataFromEEPROM(std::function<void (void *data, size_t len)> cb, ErrorCallback err);

    void getCalibrationDataFromFDR(std::function<void (void *data, size_t len)> cb, ErrorCallback err);

};
