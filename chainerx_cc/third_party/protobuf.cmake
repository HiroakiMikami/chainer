cmake_minimum_required(VERSION 3.1)

project(protobuf-download NONE)

include(ExternalProject)
ExternalProject_Add(protobuf
        GIT_REPOSITORY    https://github.com/protocolbuffers/protobuf.git
        GIT_TAG           "v${PROTOBUF_VERSION}"
        SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/protobuf-src"
        BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/protobuf-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
        TEST_COMMAND      ""
        )
