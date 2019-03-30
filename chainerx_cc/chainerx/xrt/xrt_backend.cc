#include "chainerx/xrt/xrt_backend.h"

#include <memory>
#include <stdexcept>
#include <string>

#include "chainerx/xrt/xrt_device.h"
#include "chainerx/native/native_backend.h"

namespace chainerx {
namespace xrt {

constexpr const char* XrtBackend::kDefaultName;

namespace xrt_internal {

gsl::owner<XrtDevice*> CreateDevice(XrtBackend& backend, int index) { return new XrtDevice{backend, index}; }

}  // namespace xrt_internal

std::string XrtBackend::GetName() const { return kDefaultName; }

// TODO(sonots): Returns number of CPU cores
int XrtBackend::GetDeviceCount() const { return 4; }

std::unique_ptr<Device> XrtBackend::CreateDevice(int index) {
    int device_count = GetDeviceCount();
    if (index >= device_count) {
        throw std::out_of_range{"The index number (= " + std::to_string(index) +
                                ") is not less than the device count (= " + std::to_string(device_count) + ')'};
    }
    return std::unique_ptr<XrtDevice>(xrt_internal::CreateDevice(*this, index));
}

bool XrtBackend::SupportsTransfer(Device& src_device, Device& dst_device) {
    auto is_native = [](Backend& backend) -> bool {
        return nullptr != dynamic_cast<native::NativeBackend*>(&backend);
    };
    Backend& src_backend = src_device.backend();
    Backend& dst_backend = dst_device.backend();
    // TODO(niboshi): Make clearner interface to check whether a backend is native, like dst_backend.is_native()
    if (&src_backend == this) {
        return &dst_backend == this || is_native(dst_backend);
    }
    if (&dst_backend == this) {
        return &src_backend == this || is_native(src_backend);
    }
    return false;
}

}  // namespace xrt
}  // namespace chainerx
