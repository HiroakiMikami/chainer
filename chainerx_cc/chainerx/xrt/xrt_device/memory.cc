#include "chainerx/xrt/xrt_device.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

#include "chainerx/device.h"
#include "chainerx/macro.h"

namespace chainerx {
namespace xrt {

std::shared_ptr<void> XrtDevice::Allocate(size_t bytesize) {
    if (bytesize == 0) {
        return std::shared_ptr<void>{nullptr};
    }
    return std::shared_ptr<uint8_t>{new uint8_t[bytesize], std::default_delete<uint8_t[]>()};
}

void XrtDevice::MemoryCopyFrom(void* dst, const void* src, size_t bytesize, Device& src_device) {
    CHAINERX_ASSERT(nullptr != dynamic_cast<XrtDevice*>(&src_device) && "Xrt device only supports copy between xrt devices");
    std::memcpy(dst, src, bytesize);
}

void XrtDevice::MemoryCopyTo(void* dst, const void* src, size_t bytesize, Device& dst_device) {
    CHAINERX_ASSERT(nullptr != dynamic_cast<XrtDevice*>(&dst_device) && "Xrt device only supports copy between xrt devices");
    std::memcpy(dst, src, bytesize);
}

std::shared_ptr<void> XrtDevice::TransferDataFrom(
        Device& src_device, const std::shared_ptr<void>& src_ptr, size_t offset, size_t bytesize) {
    std::shared_ptr<void> dst_ptr = Allocate(bytesize);
    MemoryCopyFrom(dst_ptr.get(), &(static_cast<int8_t*>(src_ptr.get())[offset]), bytesize, src_device);
    return dst_ptr;
}

std::shared_ptr<void> XrtDevice::TransferDataTo(
        Device& dst_device, const std::shared_ptr<void>& src_ptr, size_t offset, size_t bytesize) {
    return dst_device.TransferDataFrom(*this, src_ptr, offset, bytesize);
}

std::shared_ptr<void> XrtDevice::FromHostMemory(const std::shared_ptr<void>& src_ptr, size_t bytesize) {
    (void)bytesize;  // unused
    return src_ptr;
}

}  // namespace xrt
}  // namespace chainerx
