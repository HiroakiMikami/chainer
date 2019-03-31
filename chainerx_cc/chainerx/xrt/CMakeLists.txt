install(FILES
    xrt_device.h
    xrt_backend.h
    data_type.h
    elementwise.h
    reduce.h
    col2im.h
    im2col.h
    tensor_dot.h
    xrt_local_service.h
    DESTINATION include/chainerx/xrt
    )

add_library(chainerx_xrt STATIC
    xrt_device.cc
    xrt_device/activation.cc
    xrt_device/arithmetic.cc
    xrt_device/conv.cc
    xrt_device/copy.cc
    xrt_device/comparison.cc
    xrt_device/dot.cc
    xrt_device/exp_log.cc
    xrt_device/fill.cc
    xrt_device/indexing.cc
    xrt_device/memory.cc
    xrt_device/misc.cc
    xrt_device/pool.cc
    xrt_device/reduction.cc
    xrt_backend.cc
    col2im.cc
    im2col.cc
    tensor_dot.cc
    xrt_local_serivce.cc)

include_directories(${TENSORFLOW_INCLUDE_DIRS})
target_link_libraries(chainerx_xrt ${TENSORFLOW_LIBRARIES})

if(${CHAINERX_BUILD_TEST})
  add_executable(chainerx_xrt_test
      xrt_backend_test.cc
      xrt_device_test.cc
      xrt_local_service_test.cc
  )
  target_link_libraries(chainerx_xrt_test
      chainerx
      chainerx_xrt
      chainerx_testing
      gtest)
  add_test(NAME chainerx_xrt_test COMMAND chainerx_xrt_test)
endif()
