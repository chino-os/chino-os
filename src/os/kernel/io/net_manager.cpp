// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../ps/task/process.h"
#include "io_manager.h"
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ob.h>
#include <lwip/api.h>
#include <lwip/ip.h>
#include <lwip/tcpip.h>
#include <sys/socket.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace std::string_view_literals;

namespace {
void io_tcpip_init_done(void *arg) {
    auto done_event = reinterpret_cast<os::event *>(arg);
    done_event->notify_one();
}

class net_device : public device {
  public:
    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "net"sv;
    }

    result<void> fast_open(file &file, std::string_view path, create_disposition create_disposition) noexcept override {
        netconn_type netc_type;
        u8_t netc_proto = 0;

        if (path == "tcp") {
            netc_type = NETCONN_TCP;
        } else if (path == "udp") {
            netc_type = NETCONN_UDP;
        } else if (path == "raw") {
            netc_type = NETCONN_RAW;

            auto proto_delim = path.find_first_of('/');
            if (proto_delim == path.npos)
                return err(error_code::invalid_path);

            auto proto = path.substr(proto_delim + 1);
            if (proto == "icmp")
                netc_proto = IP_PROTO_ICMP;
            else
                return err(error_code::invalid_path);
        } else {
            return err(error_code::invalid_path);
        }

        auto netc = netconn_new_with_proto_and_callback(netc_type, netc_proto, nullptr);
        if (!netc)
            return err(error_code::out_of_memory);
        file.data(netc);
        return ok();
    }

    result<void> close(file &file) noexcept override {
        return netconn_delete(file.data<netconn>()) == ERR_OK ? ok() : err(error_code::fail);
    }
};
} // namespace

constinit static net_device net_device_;

result<void> io::initialize_net_manager() noexcept {
    os::event done_event;
    tcpip_init(io_tcpip_init_done, &done_event);
    done_event.wait().expect("Failed to wait for tcpip init.");
    return io::attach_device(net_device_);
}
