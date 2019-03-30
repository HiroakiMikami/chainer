#include "chainerx/xrt/xrt_device.h"

#include <cmath>
#include <cstdint>

#include "chainerx/array.h"
#include "chainerx/device.h"
#include "chainerx/dtype.h"
#include "chainerx/xrt/elementwise.h"
#include "chainerx/numeric.h"

namespace chainerx {
namespace xrt {

void XrtDevice::Sqrt(const Array& x, const Array& out) {
    CheckDevicesCompatible(x, out);
    const Array& x_cast = x.dtype() == out.dtype() ? x : x.AsType(out.dtype());
    VisitFloatingPointDtype(out.dtype(), [&](auto pt) {
        using T = typename decltype(pt)::type;
        struct Impl {
            void operator()(int64_t /*i*/, T x, T& out) { out = chainerx::Sqrt(x); }
        };
        Elementwise<const T, T>(Impl{}, x_cast, out);
    });
}

void XrtDevice::IsNan(const Array& x, const Array& out) {
    CheckDevicesCompatible(x, out);
    VisitDtype(x.dtype(), [&](auto pt) {
        using T = typename decltype(pt)::type;
        struct Impl {
            void operator()(int64_t /*i*/, T x, bool& out) { out = chainerx::IsNan(x); }
        };
        Elementwise<const T, bool>(Impl{}, x, out);
    });
}

void XrtDevice::IsInf(const Array& x, const Array& out) {
    CheckDevicesCompatible(x, out);
    VisitDtype(x.dtype(), [&](auto pt) {
        using T = typename decltype(pt)::type;
        struct Impl {
            void operator()(int64_t /*i*/, T x, bool& out) { out = chainerx::IsInf(x); }
        };
        Elementwise<const T, bool>(Impl{}, x, out);
    });
}

}  // namespace xrt
}  // namespace chainerx
