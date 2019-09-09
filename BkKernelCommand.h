#pragma once

#include <cstring>
#include "BkConnection.h"

#define BK_KERNEL_COMMAND_MAGIC 0x4D42
#define BK_KERNEL_COMMAND_VERSION 1

enum class BkKernelCommandId : uint16_t {
    GetCommProtocolVersion = 1,
    ResetSensor = 2,
    StartEnroll = 3,
    StartMatch = 4,
    SetTemplateListSU = 5,
    GetTemplateListSUSize = 6,
    GetTemplateListSU = 7,
    GetAlignmentData = 8,
    GetCaptureBuffer = 9,
    GetDebugImageData = 0xA,
    GetDebugImageData2 = 0xB,
    Cancel = 0xC,
    RemoveIdentity = 0xD,
    ContinueEnroll = 0xE,
    GetMaxIdentityCount = 0xF,
    GetProvisioningState = 0x10,
    GetNodeTopology = 0x11,
    NotifyDisplayPowerChanged = 0x14,
    RegisterDSID = 0x15,
    RegisterStoreToken = 0x16,
    GetCountersignedStoreToken = 0x17,
    GetCalibrationDataInfo = 0x1A,
    GetUserSerializedTemplateList = 0x1B,
    BumbCatacombCounter = 0x1C,
    GetSensorCalibrationStatus = 0x1D,
    GetUserSerializedTemplateListSize = 0x1F,
    SetCalibrationData = 0x20,
    GetModuleSerialNumber = 0x22,
    PullMatchPolicyInfoData = 0x23,
    LoadCustomPatch = 0x24,
    StartDetectFinger = 0x26,
    GetSKSLockState = 0x27,
    GetBiometricKitdInfo = 0x28,
    GetDiagnosticInfo = 0x29,
    GetLoggingType = 0x2A,
    ExtractStatusMessageData = 0x2B,
    SetBioLogState = 0x2C,
    SetUserDSID = 0x2D,
    GetProtectedConfiguration = 0x2E,
    SetProtectedConfiguration = 0x2F,
    GetEnabledForUnlock = 0x30,
    Unknown0x31 = 0x31,
    HasIdentity = 0x32,
    GetTimestampCollection = 0x33,
    ResetAppleConnectCounter = 0x34,
    GetSensorInfo = 0x35,
    GetIdentityUUID = 0x38,
    DropUnlockToken = 0x39,
    GetIdentityHash = 0x3A,
    GetCatacombState = 0x3C,
    GetUserSecureDataLength = 0x3D,
    GetUserSecureData = 0x3E,
    BumpUserSecureDataCounter = 0x3F,
    SetUserSecureData = 0x40,
    GetFreeIdentityCount = 0x41,
    GetUserTemplateList = 0x42,
    GetSystemProtectedConfiguration = 0x43,
    EnableBackgroundFdet = 0x45,
    NotifyTouchIdButtonPressed = 0x46,
    GetTemplateListCRC = 0x47,
    RemoveUser = 0x48,
    ForceBioLockout = 0x49,
    GetBioLockoutData = 0x4A,
    SetBioLockoutData = 0x4B,
    IsXartAvailable = 0x4C
};

struct BkKernelCommandHeader {
    uint16_t magic;
    BkKernelCommandId cmdId;
    uint16_t version;
    uint16_t inValue;
};
static_assert(sizeof(BkKernelCommandHeader) == 8, "bad kernel command header size");

#define BK_KERNEL_BIOMETRICKITD_INFO_SIZE 23
struct BkKernelBiometricKitdInfo {
    uint32_t maxTemplatesPerUser;
    uint32_t maxUsers;
    char filler[BK_KERNEL_BIOMETRICKITD_INFO_SIZE - 8];
};

struct BkKernelCatacombStateEntry {
    uint32_t userId;
    uint32_t unknown;
};

struct BkKernelAuthOptions {
    uint32_t isAuthToken; // CredentialSet otherwise
    uint32_t dataLength;
    char data[32];
};
struct BkKernelEnrollOptions {
    static constexpr uint32_t FLAG_IS_PURPLE_BUDDY = 1;
    uint32_t flags;
    uint32_t userId;
    BkKernelAuthOptions auth;
};
struct BkKernelProtectedConfig {
    uint touchIdUnlockEnabled;
    uint touchIdIdentificationEnabled;
    uint touchIdLoginEnabled;
    uint touchIdApplePayEnabled;
    uint effectiveTouchIdUnlockEnabled;
    uint effectiveTouchIdIdentificationEnabled;
    uint effectiveTouchIdLoginEnabled;
    uint effectiveTouchIdApplePayEnabled;
};
struct BkKernelSystemProtectedConfig {
    int touchIdUnlockTokenMaxLifetime; // > 0
    int pad[2];
    uint touchIdEnabled;
    uint touchIdUnlockEnabled;
    uint touchIdIdentificationEnabled;
    uint touchIdLoginEnabled;
};
enum class BkKernelCalibrationDataSource : uint16_t {
    FDR = 3
};

static_assert(sizeof(BkKernelEnrollOptions) == 48, "bad enroll command size");

struct BkKernelCommandExecutor {

private:
    using ErrorCallback = BkConnection::ErrorCallback;

    BkConnection &conn;
    unsigned int maxTemplatesPerUser = 0;
    unsigned int maxUsers = 0;

    static BkKernelCommandHeader buildHeader(BkKernelCommandId cmdId, uint16_t inValue = 0) {
        BkKernelCommandHeader header;
        header.magic = BK_KERNEL_COMMAND_MAGIC;
        header.version = BK_KERNEL_COMMAND_VERSION;
        header.cmdId = cmdId;
        header.inValue = inValue;
        return header;
    }

public:
    BkKernelCommandExecutor(BkConnection &conn) : conn(conn) {}

    void getCommProtocolVersion(std::function<void (int)> cb, ErrorCallback err) {
        struct {
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetCommProtocolVersion);
            uint32_t inVer = 1;
        } data;
        conn.performCommand(&data, sizeof(data), sizeof(int), [this, cb](void *data, size_t len) {
            cb(*((uint32_t *) data));
        }, err);
    }

    void resetSensor(std::function<void ()> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::ResetSensor);
        conn.performCommand(&header, sizeof(header), 0, std::bind(cb), err);
    }

    void startEnroll(BkKernelEnrollOptions opts, std::function<void ()> cb, ErrorCallback err) {
        struct {
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::StartEnroll, 1);
            BkKernelEnrollOptions eopts;
        } data;
        data.eopts = opts;
        conn.performCommand(&data, sizeof(data), 0, std::bind(cb), err);
    }

    void getTemplateListSU(std::function<void (void *data, size_t len)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetTemplateListSUSize);
        conn.performCommand(&header, sizeof(header), sizeof(uint32_t), [this, cb, err](void *data, size_t len) {
            uint32_t size = *(uint32_t*) data;
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetTemplateListSU);
            conn.performCommand(&header, sizeof(header), size, [this, cb](void *data, size_t len) {
                cb(data, len);
            }, err);
        }, err);
    }

    void setTemplateListSU(void *data, size_t dataSize, std::function<void ()> cb, ErrorCallback err) {
        uint8_t *cmdData = new uint8_t[sizeof(BkKernelCommandHeader) + dataSize];
        *((BkKernelCommandHeader *) cmdData) = buildHeader(BkKernelCommandId::SetTemplateListSU);
        memcpy(&cmdData[sizeof(BkKernelCommandHeader)], data, dataSize);
        conn.performCommand(cmdData, sizeof(BkKernelCommandHeader) + dataSize, 0, std::bind(cb), err);
        delete[] cmdData;
    }

    void getCaptureBuffer(std::function<void (void *data, size_t len)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetCaptureBuffer);
        conn.performCommand(&header, sizeof(header), 0x8000, std::move(cb), std::move(err));
    }

    void getDebugImageData(bool second, std::function<void (void *data, size_t len)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(
                second ? BkKernelCommandId::GetDebugImageData2 : BkKernelCommandId::GetDebugImageData);
        conn.performCommand(&header, sizeof(header), 0x5400, std::move(cb), std::move(err));
    }

    void cancel(std::function<void ()> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::Cancel);
        conn.performCommand(&header, sizeof(header), 0, std::bind(cb), err);
    }

    void getMaxIdentityCount(std::function<void (uint32_t)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetMaxIdentityCount);
        conn.performCommand(&header, sizeof(header), sizeof(uint32_t), [cb](void *data, size_t len) {
            cb(*(uint32_t *) data);
        }, std::move(err));
    }

    void notifyDisplayPowerChanged(bool displayOn, std::function<void ()> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::NotifyDisplayPowerChanged, displayOn ? 1 : 0);
        conn.performCommand(&header, sizeof(header), 0, std::bind(cb), err);
    }

    void setCalibrationData(BkKernelCalibrationDataSource source, void *data, size_t dataSize,
            std::function<void ()> cb, ErrorCallback err) {
        uint8_t *cmdData = new uint8_t[sizeof(BkKernelCommandHeader) + dataSize];
        *((BkKernelCommandHeader *) cmdData) = buildHeader(BkKernelCommandId::SetCalibrationData, (uint16_t) source);
        memcpy(&cmdData[sizeof(BkKernelCommandHeader)], data, dataSize);
        conn.performCommand(cmdData, sizeof(BkKernelCommandHeader) + dataSize, 0, std::bind(cb), err);
        delete[] cmdData;
    }

    void getSKSLockState(int userId, std::function<void (uint32_t)> cb, ErrorCallback err) {
        struct {
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetSKSLockState);
            int userId;
        } data;
        data.userId = userId;
        conn.performCommand(&data, sizeof(data), sizeof(uint32_t), [cb](void *data, size_t len) {
            cb(*((uint32_t *) data));
        }, err);
    }

    void getBiometricKitdInfo(std::function<void (BkKernelBiometricKitdInfo&)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetBiometricKitdInfo);
        conn.performCommand(&header, sizeof(header), BK_KERNEL_BIOMETRICKITD_INFO_SIZE,
                [this, cb](void *data, size_t len) {
                    auto info = (BkKernelBiometricKitdInfo *) data;
                    maxTemplatesPerUser = info->maxTemplatesPerUser;
                    maxUsers = info->maxUsers;
                    cb(*info);
                }, err);
    }

    void getLoggingType(std::function<void (uint8_t)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetLoggingType);
        conn.performCommand(&header, sizeof(header), sizeof(uint8_t), [cb](void *data, size_t len) {
            cb(*((uint8_t *) data));
        }, std::move(err));
    }

    void setBioLogState(int state, std::function<void ()> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::SetBioLogState, state);
        conn.performCommand(&header, sizeof(header), 0, std::bind(cb), std::move(err));
    }

    void getProtectedConfiguration(int userId, std::function<void (BkKernelProtectedConfig&)> cb, ErrorCallback err) {
        struct {
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetProtectedConfiguration);
            int userId;
        } data;
        data.userId = userId;
        conn.performCommand(&data, sizeof(data), sizeof(BkKernelProtectedConfig), [cb](void *data, size_t len) {
            cb(*((BkKernelProtectedConfig *) data));
        }, err);
    }

    void getEnabledForUnlock(std::function<void (bool)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetEnabledForUnlock);
        conn.performCommand(&header, sizeof(header), sizeof(uint8_t), [cb](void *data, size_t len) {
            cb((*(uint8_t *) data) != 0);
        }, std::move(err));
    }

    void call0x31(int userId, std::function<void ()> cb, ErrorCallback err) {
        struct {
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::Unknown0x31);
            int userId;
        } data;
        data.userId = userId;
        conn.performCommand(&data, sizeof(data), 0, std::bind(cb), err);
    }

    void getCatacombState(std::function<void (BkKernelCatacombStateEntry *, size_t)> cb, ErrorCallback err) {
        size_t entryCount = maxUsers;
        if (!entryCount)
            throw std::runtime_error("getCatacombState can only be called after getBiometricKitdInfo is finished");
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetCatacombState);
        conn.performCommand(&header, sizeof(header), sizeof(BkKernelCatacombStateEntry) * entryCount,
                [cb, entryCount](void *data, size_t len) {
                    cb((BkKernelCatacombStateEntry *) data, len / sizeof(BkKernelCatacombStateEntry));
                }, err);
    }

    void getFreeIdentityCount(int userId, std::function<void (int)> cb, ErrorCallback err) {
        struct {
            BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetFreeIdentityCount);
            int userId;
        } data;
        data.userId = userId;
        conn.performCommand(&data, sizeof(data), sizeof(int), [cb](void *data, size_t len) {
            cb(*((int *) data));
        }, err);
    }

    void getSystemProtectedConfiguration(std::function<void (BkKernelSystemProtectedConfig&)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::GetSystemProtectedConfiguration);
        conn.performCommand(&header, sizeof(header), sizeof(BkKernelSystemProtectedConfig),
                [cb](void *data, size_t len) {
                    cb(*((BkKernelSystemProtectedConfig *) data));
                }, err);
    }

    void isXartAvailable(std::function<void (bool)> cb, ErrorCallback err) {
        BkKernelCommandHeader header = buildHeader(BkKernelCommandId::IsXartAvailable);
        conn.performCommand(&header, sizeof(header), sizeof(uint8_t), [cb](void *data, size_t len) {
            cb(*((uint8_t *) data) != 0);
        }, std::move(err));
    }



};