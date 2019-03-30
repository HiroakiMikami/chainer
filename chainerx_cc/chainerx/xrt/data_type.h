#pragma once

#include <cstdint>
#include <type_traits>

#include "chainerx/dtype.h"

namespace chainerx {
namespace xrt {
namespace xrt_internal {

template <typename T>
using StorageType = TypeToDeviceStorageType<T>;

template <typename T>
T& StorageToDataType(StorageType<T>& x) {
    return *reinterpret_cast<T*>(&x);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}
template <typename T>
StorageType<T>& DataToStorageType(T& x) {
    return *reinterpret_cast<StorageType<T>*>(&x);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}
template <typename T>
T StorageToDataType(StorageType<T>&& x) {
    return *reinterpret_cast<T*>(&x);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}
template <typename T>
StorageType<T> DataToStorageType(T&& x) {
    return *reinterpret_cast<StorageType<T>*>(&x);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

}  // namespace xrt_internal

// This function is used from outside of xrt namespace.
template <typename T>
T& StorageToDataType(xrt_internal::StorageType<T>& x) {
    return xrt_internal::StorageToDataType<T>(x);
}

// This function is used from outside of xrt namespace.
template <typename T>
T StorageToDataType(xrt_internal::StorageType<T>&& x) {
    return xrt_internal::StorageToDataType<T>(x);
}

}  // namespace xrt
}  // namespace chainerx
