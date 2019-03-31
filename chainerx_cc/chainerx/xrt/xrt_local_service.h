#pragma once

// This class is based on XrtLocalService class of torch_xla

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include <tensorflow/core/distributed_runtime/server_lib.h>

namespace chainerx {
namespace xrt {

class XrtLocalService {
public:
    struct Server {
        std::string host;
        int port;
    };
    XrtLocalService(
            const std::unordered_map<std::string, std::vector<Server>> &jobs,
            const std::string &job_name, int task_index);

    // Starts the service
    void Start();

private:
    std::shared_ptr<tensorflow::ServerInterface> server_;
};

} // namespace xrt
} // namespace chainerx
