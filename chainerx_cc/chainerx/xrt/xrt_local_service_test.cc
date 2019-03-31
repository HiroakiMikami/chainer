#include "chainerx/xrt/xrt_local_service.h"

#include <gtest/gtest.h>

namespace chainerx {
namespace xrt {
namespace {

TEST(XrtLocalServiceTest, Constructor) {
    // TODO find an available port
    XrtLocalService service(
            {{"job", {XrtLocalService::Server{"localhost", 50000}}}},
            "job", 0);
    service.Start();
}

}  // namespace
}  // namespace xrt
}  // namespace chainerx
