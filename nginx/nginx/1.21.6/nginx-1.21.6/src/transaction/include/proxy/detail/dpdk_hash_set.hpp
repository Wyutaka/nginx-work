//
// Created by miwa on 2020/06/29.
//

#ifndef TRANSACTION_DPDK_HASH_SET_HPP
#define TRANSACTION_DPDK_HASH_SET_HPP

#include <rte_hash.h>
#include <rte_jhash.h>
#include <type_traits>

template <class Key>
class dpdk_hash_set {
    static_assert(std::is_trivially_copyable<Key>::value,
                  "key type must be trivially copyable");
    private:
  static constexpr int kTableEntry = 16777216;

 private:
  rte_hash* hash_;
public:
    explicit dpdk_hash_set() noexcept {
        rte_hash_parameters hash_params = {};
        hash_params.entries = kTableEntry;
        hash_params.key_len = sizeof(Key);
        hash_params.hash_func = rte_jhash;
        hash_params.socket_id = rte_socket_id();
        hash_params.extra_flag = RTE_HASH_EXTRA_FLAGS_RW_CONCURRENCY_LF;
        static constexpr char kChars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::mt19937 mt(std::random_device{}());
        std::uniform_int_distribution<> rand(0, sizeof(kChars) / sizeof(kChars[0]) - 1);

        std::string name = "tx-"; // lock manage table
        for(std::size_t i = 0; i < 16; ++i) {
            name += kChars[rand(mt)];
        }

        hash_params.name = name.c_str();
        hash_ = rte_hash_create(&hash_params);
    }

    // trivially copyable
    dpdk_hash_set(const dpdk_hash_set &) = delete;
    dpdk_hash_set(dpdk_hash_set &&) = default;

    dpdk_hash_set & operator=(const dpdk_hash_set &) = delete;
    dpdk_hash_set & operator=(dpdk_hash_set &&) = default;

    ~dpdk_hash_set() = default;

public:
    /// release resources in rte_hash
    void release() noexcept {
        rte_hash_free(hash_);
        hash_ = nullptr;
    }

    /// check contains specified key
    /// \param key key
    /// \return true => contains, false => not in hash table
    bool has(const Key& key) const noexcept {
        return rte_hash_lookup(hash_, &key) >= 0;
    }

    /// set value for specified key
    /// \param key key
    /// \param value value
    /// \return true => successfully, false => failed
    bool set(const Key& key) noexcept {
        return rte_hash_add_key(hash_, &key) >= 0;
    }

    /// check hash table has no contents
    /// \return true => no contents, false => contains data
    bool empty() const noexcept {
        return rte_hash_count(hash_) == 0;
    }

    /// remove key
    /// \param key key want to remove
    /// \return true => successfully, false => failed
    bool remove(const Key& key) noexcept {
        return rte_hash_del_key(hash_, &key) >= 0;
    }
};


#endif // TRANSACTION_DPDK_HASH_SET_HPP
