//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <type_traits>

#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_lcore.h>

namespace txpx2 {

template <class KeyT, class ValueT>
class DpdkHashMap {
  static_assert(std::is_trivially_copyable<KeyT>::value,
                "key type must be trivially copyable");
  static_assert(std::is_trivially_copyable<ValueT>::value,
                "value type must be trivially copyable");

 private:
  static constexpr int kTableEntry = 0x800000;

 private:
  rte_hash* hash_;

 public:
  explicit DpdkHashMap(const char* name) noexcept {
    rte_hash_parameters hash_params = {};
    hash_params.entries = kTableEntry;
    hash_params.key_len = sizeof(KeyT);
    hash_params.hash_func = rte_jhash;
    hash_params.socket_id = rte_socket_id();
    hash_params.extra_flag = RTE_HASH_EXTRA_FLAGS_RW_CONCURRENCY_LF;
    hash_params.name = name;
    hash_ = rte_hash_create(&hash_params);
  }

  // trivially copyable
  DpdkHashMap(const DpdkHashMap&) = default;
  DpdkHashMap(DpdkHashMap&&) = default;

  DpdkHashMap& operator=(const DpdkHashMap&) = default;
  DpdkHashMap& operator=(DpdkHashMap&&) = default;

  ~DpdkHashMap() = default;

 public:
  /// release resources in rte_hash
  void release() noexcept {
    rte_hash_free(hash_);
    hash_ = nullptr;
  }

  /// get value
  /// \param key key
  /// \param buffer value buffer
  /// \return true => successfully, false => failed
  bool get(const KeyT& key, ValueT* buffer) const noexcept {
    return rte_hash_lookup_data(hash_, &key,
                                reinterpret_cast<void**>(&buffer)) >= 0;
  }

  /// check contains specified key
  /// \param key key
  /// \return true => contains, false => not in hash table
  bool exists(const KeyT& key) const noexcept {
    return rte_hash_lookup(hash_, &key) >= 0;
  }

  /// set value for specified key
  /// \param key key
  /// \param value value
  /// \return true => successfully, false => failed
  bool set(const KeyT& key, ValueT value) noexcept {
    return rte_hash_add_key_data(hash_, &key, &value) >= 0;
  }

  /// check hash table has no contents
  /// \return true => no contents, false => contains data
  bool empty() const noexcept { return rte_hash_count(hash_) == 0; }

  /// remove key
  /// \param key key want to remove
  /// \return true => successfully, false => failed
  bool remove(const KeyT& key) noexcept {
    return rte_hash_del_key(hash_, &key) >= 0;
  }

  /// remove keys that has specified value
  /// \param value value want to remove
  /// \return true
  bool removeByValue(const ValueT& value) noexcept {
    std::uint32_t next = 0;
    const void* key;
    void* data;

    while (rte_hash_iterate(hash_, &key, &data, &next) >= 0) {
      if (memcmp(data, &value, sizeof(ValueT)) == 0) {
        rte_hash_del_key(hash_, key);
      }
    }
    return true;
  }
};

template <class KeyT>
class DpdkHashSet {
  static_assert(std::is_trivially_copyable<KeyT>::value,
                "key type must be trivially copyable");

 private:
  static constexpr int kTableEntry = 0x800000;

 private:
  rte_hash* hash_;

 public:
  explicit DpdkHashSet(const char* name) noexcept {
    rte_hash_parameters hash_params = {};
    hash_params.entries = kTableEntry;
    hash_params.key_len = sizeof(KeyT);
    hash_params.hash_func = rte_jhash;
    hash_params.socket_id = rte_socket_id();
    hash_params.extra_flag = RTE_HASH_EXTRA_FLAGS_RW_CONCURRENCY_LF;
    hash_params.name = name;
    hash_ = rte_hash_create(&hash_params);
  }

  // trivially copyable
  DpdkHashSet(const DpdkHashSet&) = default;
  DpdkHashSet(DpdkHashSet&&) = default;

  DpdkHashSet& operator=(const DpdkHashSet&) = default;
  DpdkHashSet& operator=(DpdkHashSet&&) = default;

  ~DpdkHashSet() = default;

 public:
  /// release resources in rte_hash
  void release() noexcept {
    rte_hash_free(hash_);
    hash_ = nullptr;
  }

  /// check contains specified key
  /// \param key key
  /// \return true => contains, false => not in hash table
  bool has(const KeyT& key) const noexcept {
    return rte_hash_lookup(hash_, &key) >= 0;
  }

  /// set value for specified key
  /// \param key key
  /// \param value value
  /// \return true => successfully, false => failed
  bool set(const KeyT& key) noexcept {
    return rte_hash_add_key(hash_, &key) >= 0;
  }

  /// check hash table has no contents
  /// \return true => no contents, false => contains data
  bool empty() const noexcept { return rte_hash_count(hash_) == 0; }

  /// remove key
  /// \param key key want to remove
  /// \return true => successfully, false => failed
  bool remove(const KeyT& key) noexcept {
    return rte_hash_del_key(hash_, &key) >= 0;
  }
};

}  // namespace txpx2
