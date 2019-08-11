#include "chainerx/xrt/xrt_backend.h"

#include <cstring>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "chainerx/array.h"
#include "chainerx/context.h"
#include "chainerx/device.h"
#include "chainerx/routines/creation.h"
#include "chainerx/testing/threading.h"
#include "chainerx/native/native_backend.h"

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

void ExpectArraysEqual(const Array& expected, const Array& actual) {
    EXPECT_EQ(expected.dtype(), actual.dtype());
    EXPECT_EQ(expected.shape(), actual.shape());
    VisitDtype(expected.dtype(), [&expected, &actual](auto pt) {
        using T = typename decltype(pt)::type;
        int64_t total_size = expected.GetTotalSize();
        auto data1 = static_cast<const T*>(expected.data().get());
        auto data2 = static_cast<const T*>(actual.data().get());
        for (int64_t i = 0; i < total_size; ++i) {
            EXPECT_EQ(data1[i], data2[i]);
        }
    });
}

TEST(XrtBackendTest, GetDeviceCount) {
    Context ctx;
    // TODO(sonots): Get number of CPU cores
    EXPECT_EQ(4, XrtBackend{ctx}.GetDeviceCount());
}

TEST(XrtBackendTest, GetDeviceCountGetNameThreadSafe) {
    static constexpr size_t kThreadCount = 2;

    int expected_device_count = Context{}.GetBackend("xrt").GetDeviceCount();
    std::string expected_backend_name = Context{}.GetBackend("xrt").GetName();

    Context ctx{};
    Backend& backend = ctx.GetBackend("xrt");

    testing::RunThreads(kThreadCount, [&backend, &expected_device_count, &expected_backend_name]() {
        int device_count = backend.GetDeviceCount();
        std::string name = backend.GetName();
        EXPECT_EQ(expected_device_count, device_count);
        EXPECT_EQ(expected_backend_name, name);
    });
}

TEST(XrtBackendTest, GetDevice) {
    Context ctx;
    XrtBackend backend{ctx};
    {
        Device& device = backend.GetDevice(0);
        EXPECT_EQ(&backend, &device.backend());
        EXPECT_EQ(0, device.index());
    }
    {
        Device& device3 = backend.GetDevice(3);
        Device& device2 = backend.GetDevice(2);
        EXPECT_EQ(&backend, &device3.backend());
        EXPECT_EQ(3, device3.index());
        EXPECT_EQ(&backend, &device2.backend());
        EXPECT_EQ(2, device2.index());
    }
    {
        EXPECT_THROW(backend.GetDevice(-1), std::out_of_range);
        EXPECT_THROW(backend.GetDevice(backend.GetDeviceCount() + 1), std::out_of_range);
    }
}

TEST(XrtBackendTest, GetDeviceThreadSafe) {
    static constexpr int kDeviceCount = 2;
    static constexpr size_t kThreadCount = kDeviceCount;

    Context ctx{};
    Backend& backend = ctx.GetBackend("xrt");

    testing::RunThreads(kThreadCount, [&backend](size_t thread_index) {
        int device_index = thread_index;
        Device& device = backend.GetDevice(device_index);
        EXPECT_EQ(&backend, &device.backend());
        EXPECT_EQ(device_index, device.index());
    });
}

TEST(XrtBackendTest, GetName) {
    Context ctx;
    EXPECT_EQ("xrt", XrtBackend{ctx}.GetName());
}

TEST(XrtBackendTest, SupportsTransferThreadSafe) {
    /* TODO it is unclear whether we can transfer data from a replica to another replica */
    static constexpr size_t kThreadCount = 2;

    Context ctx0{};
    Context ctx1{};
    Backend& ctx0_backend = ctx0.GetBackend("xrt");
    Backend& ctx1_backend = ctx1.GetBackend("xrt");
    Device& ctx0_device0 = ctx0_backend.GetDevice(0);
    Device& ctx0_device1 = ctx0_backend.GetDevice(1);
    Device& ctx1_device = ctx1_backend.GetDevice(0);

    testing::RunThreads(kThreadCount, [&ctx0_backend, &ctx0_device0, &ctx0_device1, &ctx1_device]() {
        EXPECT_TRUE(ctx0_backend.SupportsTransfer(ctx0_device0, ctx0_device1));
        EXPECT_FALSE(ctx0_backend.SupportsTransfer(ctx0_device0, ctx1_device));
    });
}

TEST(XrtBackendIncompatibleTransferTest, SupportsTransferDifferentContexts) {
    Context ctx0;
    Context ctx1;
    XrtBackend backend0{ctx0};
    XrtBackend backend1{ctx1};
    Device& device0 = backend0.GetDevice(0);
    Device& device1 = backend1.GetDevice(0);
    EXPECT_FALSE(backend0.SupportsTransfer(device0, device1));
}

TEST(XrtBackendIncompatibleTransferTest, SupportsTransferNativeBackends) {
    Context ctx;
    XrtBackend xrt_backend{ctx};
    native::NativeBackend native_backend0{ctx};
    native::NativeBackend native_backend1{ctx};
    Device& device = xrt_backend.GetDevice(0);
    Device& device0 = native_backend0.GetDevice(0);
    Device& device1 = native_backend1.GetDevice(0);
    EXPECT_TRUE(xrt_backend.SupportsTransfer(device, device0));
    EXPECT_TRUE(xrt_backend.SupportsTransfer(device0, device));
    EXPECT_FALSE(xrt_backend.SupportsTransfer(device0, device1));
}

template <int N>
class DerivedXrtBackend : public XrtBackend {
public:
    using XrtBackend::XrtBackend;
    std::string GetName() const override { return "derived" + std::to_string(N); }
};

TEST(XrtBackendIncompatibleTransferTest, SupportsTransferDifferentXrtBackends) {
    Context ctx;
    DerivedXrtBackend<0> backend0{ctx};
    DerivedXrtBackend<1> backend1{ctx};
    Device& device0 = backend0.GetDevice(0);
    Device& device1 = backend1.GetDevice(0);
    EXPECT_FALSE(backend0.SupportsTransfer(device0, device1));
}

// Data transfer test
class XrtBackendTransferTest : public ::testing::TestWithParam<::testing::tuple<std::string, std::string>> {};

INSTANTIATE_TEST_CASE_P(
        Devices,
        XrtBackendTransferTest,
        ::testing::Values(
                std::make_tuple("xrt:0", "xrt:0"),  // xrt:0 <-> xrt:0
                std::make_tuple("xrt:0", "xrt:1")));  // xrt:0 <-> xrt:1

TEST_P(XrtBackendTransferTest, SupportsTransfer) {
    Context ctx;
    Backend& backend = ctx.GetBackend("xrt");
    Device& device0 = ctx.GetDevice(::testing::get<0>(GetParam()));
    Device& device1 = ctx.GetDevice(::testing::get<1>(GetParam()));
    EXPECT_TRUE(backend.SupportsTransfer(device0, device1));
}

TEST_P(XrtBackendTransferTest, MemoryCopyFrom) {
    size_t size = 3;
    size_t bytesize = size * sizeof(float);
    float raw_data[] = {0, 1, 2};
    std::shared_ptr<void> src_orig(raw_data, [](const float* ptr) {
        (void)ptr;  // unused
    });

    Context ctx;
    Device& device0 = ctx.GetDevice(::testing::get<0>(GetParam()));
    Device& device1 = ctx.GetDevice(::testing::get<1>(GetParam()));

    std::shared_ptr<void> src = device1.FromHostMemory(src_orig, bytesize);
    std::shared_ptr<void> dst = device0.Allocate(bytesize);
    device0.MemoryCopyFrom(dst.get(), src.get(), bytesize, device1);
    ExpectDataEqual<float>(src, dst, size);
}

TEST_P(XrtBackendTransferTest, MemoryCopyTo) {
    size_t size = 3;
    size_t bytesize = size * sizeof(float);
    float raw_data[] = {0, 1, 2};
    std::shared_ptr<void> src_orig(raw_data, [](const float* ptr) {
        (void)ptr;  // unused
    });

    Context ctx;
    Device& device0 = ctx.GetDevice(::testing::get<0>(GetParam()));
    Device& device1 = ctx.GetDevice(::testing::get<1>(GetParam()));

    std::shared_ptr<void> src = device0.FromHostMemory(src_orig, bytesize);
    std::shared_ptr<void> dst = device1.Allocate(bytesize);
    device0.MemoryCopyTo(dst.get(), src.get(), bytesize, device1);
    ExpectDataEqual<float>(src, dst, size);
}

TEST_P(XrtBackendTransferTest, TransferDataFrom) {
    Context ctx;
    Device& device0 = ctx.GetDevice(::testing::get<0>(GetParam()));
    Device& device1 = ctx.GetDevice(::testing::get<1>(GetParam()));

    size_t bytesize = 5;
    auto data = device1.Allocate(bytesize);

    // Transfer
    std::shared_ptr<void> trans_data = device0.TransferDataFrom(device1, data, 0, bytesize);

    EXPECT_EQ(0, std::memcmp(data.get(), trans_data.get(), bytesize));
}

TEST_P(XrtBackendTransferTest, TransferDataTo) {
    Context ctx;
    Device& device0 = ctx.GetDevice(::testing::get<0>(GetParam()));
    Device& device1 = ctx.GetDevice(::testing::get<1>(GetParam()));

    size_t bytesize = 5;
    auto data = device0.Allocate(bytesize);

    // Transfer
    std::shared_ptr<void> trans_data = device0.TransferDataTo(device1, data, 0, bytesize);

    EXPECT_EQ(0, std::memcmp(data.get(), trans_data.get(), bytesize));
}

TEST_P(XrtBackendTransferTest, ArrayToDevice) {
    Context ctx;
    Device& device0 = ctx.GetDevice(::testing::get<0>(GetParam()));
    Device& device1 = ctx.GetDevice(::testing::get<1>(GetParam()));

    // Allocate the source array
    float data[] = {1.0f, 2.0f};
    auto nop = [](void* p) {
        (void)p;  // unused
    };
    Array a = FromContiguousHostData({2, 1}, Dtype::kFloat32, std::shared_ptr<float>(data, nop), device0);

    // Transfer
    Array b = a.ToDevice(device1);

    EXPECT_EQ(&b.device(), &device1);
    EXPECT_EQ(&a.device(), &device0);
    if (&device0 == &device1) {
        EXPECT_EQ(a.data().get(), b.data().get()) << "Array::ToDevice() must return alias when transferring to the same xrt device.";
    } else {
        EXPECT_NE(a.data().get(), b.data().get())
                << "Array::ToDevice() must not return alias when transferring to a different xrt device.";
    }
    ExpectArraysEqual(a, b);
}

}  // namespace
}  // namespace xrt
}  // namespace chainerx