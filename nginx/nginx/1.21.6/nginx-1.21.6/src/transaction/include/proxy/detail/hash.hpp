//
// Created by miwa on 2019/12/05.
//

#ifndef TRANSACTION_HASH_HPP
#define TRANSACTION_HASH_HPP

#include <functional>

namespace proxy {
    struct string_view_hash {
        std::size_t operator()(cpp17::string_view key) const {
            return std::_Hash_bytes(key.data(), key.size(), static_cast<size_t>(0xc70f6907UL));
        }
    };
} // namespace proxy
#endif // TRANSACTION_HASH_HPP
