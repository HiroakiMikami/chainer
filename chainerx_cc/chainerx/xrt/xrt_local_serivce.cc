#include <sstream>

#include "chainerx/macro.h"
#include "chainerx/xrt/xrt_local_service.h"

namespace chainerx {
namespace xrt {

XrtLocalService::XrtLocalService(
        const std::unordered_map<std::string, std::vector<Server>> &jobs,
        const std::string &job_name, int task_index) {
    // TODO It is unclear whether I have to specify other jobs
    tensorflow::ServerDef options;
    options.set_protocol("grpc");
    options.set_job_name(job_name);
    options.set_task_index(task_index);

    size_t num_tasks = 0;
    auto cluster = options.mutable_cluster();
    for (const auto &job : jobs) {
        auto job_def = cluster->add_job();
        job_def->set_name(job.first);
        std::ostringstream s;
        for (size_t i = 0; i < job.second.size(); ++i) {
            const auto &server = job.second[i];
            s.clear();
            s << server.host << ":" << server.port;
            (*job_def->mutable_tasks())[i] = s.str();
        }

        if (job_name == options.job_name()) {
            num_tasks = job.second.size();
        }
    }

    CHAINERX_ASSERT(num_tasks != 0);
    CHAINERX_ASSERT(options.task_index() < static_cast<int>(num_tasks));

    std::unique_ptr<tensorflow::ServerInterface> server;
    TF_CHECK_OK(tensorflow::NewServer(options, &server));

    /*
     * TODO:
     * GrpcServer of Tensorflow does not implement the Stop method.
     * The destructor of GrpcServer thus abort the process.
     */
    std::shared_ptr<tensorflow::ServerInterface> s(server.get(), [](tensorflow::ServerInterface *server) {
        CHAINERX_ASSERT(server != nullptr);
    });
    server.release();
    this->server_ = s;
}

void XrtLocalService::Start() {
    TF_CHECK_OK(this->server_->Start());
}

} // namespace xrt
} // namespace chainerx