#include "chainerx/xrt/xrt_device.h"

#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "chainerx/context.h"
#include "chainerx/xrt/xrt_backend.h"
#include "chainerx/testing/threading.h"

namespace chainerx {
namespace xrt {
namespace {

template <typename T>
void ExpectDataEqual(const std::shared_ptr<void>& expected, const std::shared_ptr<void>& actual, size_t size) {
    auto expected_raw_ptr = static_cast<const T*>(expected.get());
    auto actual_raw_ptr = static_cast<const T*>(actual.get());
    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(expected_raw_ptr[i], actual_raw_ptr[i]);
    }
}

XrtDevice& GetXrtDevice(Context& ctx, int device_index) {
    // Using dynamic_cast to ensure it's actually XrtDevice
    return dynamic_cast<XrtDevice&>(ctx.GetDevice({"xrt", device_index}));
}

TEST(XrtDeviceTest, Allocate) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);

    size_t bytesize = 3;
    std::shared_ptr<void> ptr = device.Allocate(bytesize);
    EXPECT_NE(nullptr, ptr);
}

TEST(XrtDeviceTest, AllocateZero) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);

    std::shared_ptr<void> ptr = device.Allocate(size_t{0});
    EXPECT_EQ(ptr, nullptr);
}

TEST(XrtDeviceTest, AllocateFreeThreadSafe) {
    static constexpr size_t kNumThreads = 2;
    static constexpr size_t kNumLoopsPerThread = 1;
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);

    // Allocate-and-free loop
    auto func = [&device](size_t size) {
        for (size_t j = 0; j < kNumLoopsPerThread; ++j) {
            std::shared_ptr<void> ptr = device.Allocate(size);
            (void)ptr;  // unused
        }
    };

    // Launch threads
    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);
    for (size_t i = 0; i < kNumThreads; ++i) {
        threads.emplace_back(func, i);
    }

    // Join threads
    for (size_t i = 0; i < kNumThreads; ++i) {
        threads[i].join();
    }
}

TEST(XrtDeviceTest, MakeDataFromForeignPointer) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);

    size_t bytesize = 3;
    std::shared_ptr<void> data = device.Allocate(bytesize);
    EXPECT_EQ(data.get(), device.MakeDataFromForeignPointer(data).get());
}

TEST(XrtDeviceTest, FromHostMemory) {
    size_t size = 3;
    size_t bytesize = size * sizeof(float);
    float raw_data[] = {0, 1, 2};
    std::shared_ptr<void> src(raw_data, [](const float* ptr) {
        (void)ptr;  // unused
    });

    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);

    std::shared_ptr<void> dst = device.FromHostMemory(src, bytesize);
    EXPECT_EQ(src.get(), dst.get());
}

TEST(XrtDeviceTest, Synchronize) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);
    device.Synchronize();  // no throw
}

TEST(XrtDeviceTest, GetBackendMultiThread) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);
    Backend& expected_backend = device.backend();

    testing::RunThreads(2, [&device, &expected_backend]() {
        Backend& backend = device.backend();
        EXPECT_EQ(&expected_backend, &backend);
    });
}

TEST(XrtDeviceTest, GetIndexMultiThread) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);
    int expected_index = device.index();

    testing::RunThreads(2, [&device, &expected_index]() {
        int index = device.index();
        EXPECT_EQ(expected_index, index);
    });
}

TEST(XrtDeviceTest, GetNameMultiThread) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);
    std::string expected_name = device.name();

    testing::RunThreads(2, [&device, &expected_name]() {
        std::string name = device.name();
        EXPECT_EQ(expected_name, name);
    });
}

TEST(XrtDeviceTest, GetContextMultiThread) {
    Context ctx;
    XrtDevice& device = GetXrtDevice(ctx, 0);

    testing::RunThreads(2, [&ctx, &device]() {
        Context& context = device.context();
        EXPECT_EQ(&ctx, &context);
    });
}

}  // namespace
}  // namespace xrt
}  // namespace chainerx
