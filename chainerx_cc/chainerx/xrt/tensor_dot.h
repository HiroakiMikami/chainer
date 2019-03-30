#pragma once

#include "chainerx/array.h"
#include "chainerx/axes.h"

namespace chainerx {
namespace xrt {

Array TensorDot(const Array& a, const Array& b, const Axes& a_axis, const Axes& b_axis, Dtype out_dtype);

}  // namespace xrt
}  // namespace chainerx
