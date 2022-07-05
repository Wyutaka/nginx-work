//
// Created by cerussite on 2019/10/19.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "hook_context.hpp"
#include <proxy/types/peer.hpp>
#include <proxy/types/response/response.hpp>

namespace proxy {
    template <class KeyT, class... Args> class Hook {
    public:
        using KeyType = KeyT;
        using FunctionType = std::function<void(Args...)>;

    private:
        std::unordered_map<KeyType, std::vector<FunctionType>> _hooks;

    public:
        void addHook(const KeyType &key, FunctionType func) {
            if (_hooks.find(key) == std::end(_hooks)) {
                _hooks[key] = {};
            }
            _hooks[key].emplace_back(std::move(func));
        }

    private:
        static void CallImpl(const std::vector<FunctionType> &func, Args... args) {
            for (const auto &f : func) {
                f(args...);
            }
        }

        static void CallImpl(
            typename std::unordered_map<KeyType, std::vector<FunctionType>>::const_iterator itr,
            Args... args) {
            CallImpl(itr->second, args...);
        }

    public:
        bool call(const KeyType &key, Args... args) const {
            auto itr = _hooks.find(key);
            if (itr == std::end(_hooks)) {
                return false;
            }

            CallImpl(itr, args...);
            return true;
        }

        bool callAndRemove(const KeyType &key, Args... args) {
            auto itr = _hooks.find(key);
            if (itr == std::end(_hooks)) {
                return false;
            }
            CallImpl(itr, args...);
            _hooks.erase(itr);

            return true;
        }

    public:
        bool remove(const KeyType &key) { _hooks.erase(key); }
    };

    class SingletonHook {
    public:
        using HookType = Hook<std::uint_fast64_t , HookContext&>;
        using KeyType = typename HookType::KeyType;
        using FunctionType = typename HookType::FunctionType;

    private:
        static HookType &GetInstance(int thread_num) {
            static HookType hook[10];
            return hook[thread_num];
        }

    public:
        static void AddHook(int thread_num, const KeyType &key, FunctionType func) {
            GetInstance(thread_num).addHook(key, std::move(func));
        }

        static bool CallAndRemove(int thread_num, const KeyType &key, HookContext& res) {
            return GetInstance(thread_num).callAndRemove(key, res);
        }

    public:
        static std::uint_fast64_t CreateKey(const Peer& peer, std::uint_fast64_t streamID) noexcept {
            return (streamID << 32 | peer.id());
        }
    };

} // namespace proxy
