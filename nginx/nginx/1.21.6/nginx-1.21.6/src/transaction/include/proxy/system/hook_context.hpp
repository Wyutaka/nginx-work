//
// Created by miwa on 2020/03/12.
//

#ifndef TRANSACTION_HOOK_CONTEXT_HPP
#define TRANSACTION_HOOK_CONTEXT_HPP
#include <cstring>
#include <iostream>
#include <proxy/detail/debug.hpp>
#include <proxy/detail/span.hpp>
#include <stdint-gcc.h>

namespace proxy {
    class HookContext{
    private:
        bool continue_response_ = true;
        Span<char> raw_response_;

    public:
        explicit HookContext(Span<char> res) : raw_response_(res) {}

    public:
        HookContext(const HookContext &) = default;
        HookContext(HookContext &&) = default;

        HookContext &operator=(const HookContext &) = default;
        HookContext &operator=(HookContext &&) = default;

    public:
        bool continueResponse() const noexcept {return continue_response_;}

        void abort() noexcept {continue_response_ = false;}

        const Span<char>& data() {return raw_response_;}

        bool empty() const noexcept {
            char result_kind = *(raw_response_.data()+12);
            switch(result_kind){
            case 0x01: // void
                return true;
            case 0x02:
                return row_count() == 0;
            default:
                return false;
            }
        }

        int32_t row_count() const noexcept {
            int32_t row_count;
            memcpy(&row_count, raw_response_.data() + 46, sizeof(int32_t));
            return row_count;
        }
    };
} // namespace proxy

#endif // TRANSACTION_HOOK_CONTEXT_HPP
