#pragma once

#include <memory>
#include <string>

#include "chainerx/backend.h"
#include "chainerx/device.h"

namespace chainerx {
namespace xrt {

class XrtDevice;
class XrtBackend;

namespace xrt_internal {

// Creates a device instance.
// This function is meant to be used from the backend class. Never use it for other purpose.
// This is defined in internal namespace in order to make it a friend of XrtDevice
// class.
XrtDevice* CreateDevice(XrtBackend& backend, int index);

}  // namespace xrt_internal

class XrtBackend : public Backend {
public:
    static constexpr const char* kDefaultName = "xrt";

    using Backend::Backend;

    std::string GetName() const override;

    int GetDeviceCount() const override;

    bool SupportsTransfer(Device& src_device, Device& dst_device) override;

private:
    std::unique_ptr<Device> CreateDevice(int index) override;
};

}  // namespace xrt
}  // namespace chainerx
