#include <event2/event.h>
#include <event2/dns.h>
#include <bridgexpc/connection_libevent.h>
#include "BkConnection.h"
#include "BkKernelCommand.h"

int main(int argn, const char **argv) {
    if (argn < 3) {
        printf("bkitd <remote-ip> <remote-port>\n");
        return 1;
    }

    event_base *evbase;
    evdns_base *evdns;
    bridge_xpc_libevent_connection *conn;
    BkConnection *bkConn;

    evbase = event_base_new();
    if (!evbase) {
        fprintf(stderr, "event_base_new failed\n");
        return -ENOMEM;
    }
    evdns = evdns_base_new(evbase, 1);

    conn = bridge_xpc_libevent_connection_create(evbase);
    bkConn = new BkConnection(&conn->conn);
    BkKernelCommandExecutor bkKCE (*bkConn);

    bkConn->setConnectedCallback([bkConn, &bkKCE]() {
        bkKCE.getCommProtocolVersion([](int v) {
            printf("bkitd: remote comm protocol version: %i\n", v);
        }, nullptr);

        bkKCE.getBiometricKitdInfo([bkConn, &bkKCE](BkKernelBiometricKitdInfo &kd) {
            printf("bkitd: remote max templates per user: %u\n", kd.maxTemplatesPerUser);
            printf("bkitd: remote max users: %u\n", kd.maxUsers);
            bkKCE.getCatacombState([](BkKernelCatacombStateEntry *ent, size_t cnt) {
                for (size_t i = 0; i < cnt; i++)
                    printf("bkitd: remote catacomb: #%li userId=%i unknown=%i\n", i, ent[i].userId, ent[i].unknown);
            }, nullptr);
        }, nullptr);

        bkConn->getCalibrationDataFromFDR([&bkKCE](void *data, size_t len) {
            bkKCE.setCalibrationData(BkKernelCalibrationDataSource::FDR, data, len, []() {
                printf("bkitd: loaded calibration data\n");
            }, nullptr);
        }, nullptr);
    });

    if (bufferevent_socket_connect_hostname(conn->bev, evdns, AF_UNSPEC, argv[1], atoi(argv[2]))) {
        fprintf(stderr, "bufferevent_socket_connect failed\n");
        return -ENOMEM;;
    }

    event_base_loop(evbase, 0);

    delete bkConn;

    evdns_base_free(evdns, 1);
    event_base_free(evbase);

    return 0;
}