//
// Created by miwa on 2019/10/28.
//

#ifndef TRANSACTION_HEADER_HPP
#define TRANSACTION_HEADER_HPP

#include <cstdint>

namespace proxy {
    namespace detail {
        struct Header {
            std::uint8_t version;
            std::uint8_t flags;
            std::uint16_t stream;
            std::uint8_t opcode;
            std::uint32_t length;
        } __attribute__((packed));
    } // namespace detail
} // namespace proxy

#endif //TRANSACTION_HEADER_HPP
