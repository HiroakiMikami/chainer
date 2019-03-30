#include "chainerx/xrt/xrt_device.h"

#include <cstdint>

#include "chainerx/array.h"
#include "chainerx/device.h"
#include "chainerx/dtype.h"
#include "chainerx/xrt/elementwise.h"
#include "chainerx/numeric.h"

namespace chainerx {
namespace xrt {

void XrtDevice::Exp(const Array& x, const Array& out) {
    CheckDevicesCompatible(x, out);
    const Array& x_cast = x.dtype() == out.dtype() ? x : x.AsType(out.dtype());
    VisitFloatingPointDtype(out.dtype(), [&x_cast, &out](auto pt) {
        using T = typename decltype(pt)::type;
        struct Impl {
            void operator()(int64_t /*i*/, T x, T& out) { out = chainerx::Exp(x); }
        };
        Elementwise<const T, T>(Impl{}, x_cast, out);
    });
}

void XrtDevice::Log(const Array& x, const Array& out) {
    CheckDevicesCompatible(x, out);
    const Array& x_cast = x.dtype() == out.dtype() ? x : x.AsType(out.dtype());
    VisitFloatingPointDtype(out.dtype(), [&x_cast, &out](auto pt) {
        using T = typename decltype(pt)::type;
        struct Impl {
            void operator()(int64_t /*i*/, T x, T& out) { out = chainerx::Log(x); }
        };
        Elementwise<const T, T>(Impl{}, x_cast, out);
    });
}

}  // namespace xrt
}  // namespace chainerx
