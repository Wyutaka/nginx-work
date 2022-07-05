//
// Created by cerussite on 2019/10/13.
//

#include <proxy/detail/unique_id.hpp>
#include <random>

namespace {
    std::string ToRadix64(std::uint64_t p) {
        int i;
        if (!p)
            return "0";
        char t[64] = {};
        for (i = 62; p; --i) {
            t[i] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-"[p % 64];
            p /= 64;
        }
        return std::string(t + i + 1);
    }
} // namespace

namespace proxy {
    std::string GenerateUniqueId() {
        static std::mt19937_64 mt64(std::random_device{}());
        static std::uniform_int_distribution<std::uint64_t> rand64;

        std::uint64_t _1 = rand64(mt64), _2 = rand64(mt64);

        return ToRadix64(_1) + ToRadix64(_2);
    }
} // namespace proxy
