cmake_minimum_required(VERSION 3.10)
project(transaction)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(libcpp17/include)

set(MTCP_ROOT $ENV{HOME}/new_mtcp/mtcp)

include_directories(${MTCP_ROOT}/mtcp/src/include)

include_directories(${MTCP_ROOT}/dpdk/x86_64-native-linuxapp-gcc/include)

add_executable(transaction
        main.cpp
        include/proxy/lock.hpp
        include/proxy/types/peer.hpp
        include/proxy/types/request/request.hpp
        include/proxy/types/response/response.hpp
        include/proxy/types/request/request_query.hpp
        include/proxy/detail/pipes.hpp
        include/proxy/detail/str.hpp
        include/proxy/transaction.hpp
        include/proxy/transaction/state.hpp
        include/proxy/detail/unique_id.hpp
        include/proxy/types/response/response_data.hpp
        include/proxy/system/hook.hpp
        include/proxy/types/detail/header.hpp
        include/proxy/kc.hpp
        include/proxy/types/response/enqueue_result.hpp
        include/proxy/types/detail/aggregator_types.h
        src/types/response/enqueue_result.cpp
        src/detail/unique_id.cpp
        src/detail/str.cpp
        include/proxy/detail/span.hpp include/proxy/detail/debug.hpp include/proxy/types/response/response_body.hpp include/proxy/detail/hash.hpp include/proxy/detail/unique_ptr.hpp include/proxy/detail/dpdk_hash_map.hpp include/proxy/detail/variant.hpp include/proxy/system/hook_context.hpp src/detail/hook.cpp include/proxy/detail/dpdk_hash_set.hpp)

