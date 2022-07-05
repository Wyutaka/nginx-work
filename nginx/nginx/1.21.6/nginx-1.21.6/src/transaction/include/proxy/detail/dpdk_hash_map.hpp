//
// Created by miwa on 2019/12/10.
//

#ifndef TRANSACTION_DPDK_HASH_MAP_HPP
#define TRANSACTION_DPDK_HASH_MAP_HPP

#include <cstdint>
#include <random>
#include <string>
//#include <rte_hash.h>
//#include <rte_jhash.h>

extern "C" {
struct rte_hash *rte_hash_create(const struct rte_hash_parameters *params);

void rte_hash_set_cmp_func(struct rte_hash *h, rte_hash_cmp_eq_t func);

struct rte_hash *rte_hash_find_existing(const char *name);

void rte_hash_free(struct rte_hash *h);

void rte_hash_reset(struct rte_hash *h);

int32_t rte_hash_count(const struct rte_hash *h);

int rte_hash_add_key_data(const struct rte_hash *h, const void *key, void *data);

int32_t rte_hash_add_key_with_hash_data(const struct rte_hash *h, const void *key, hash_sig_t sig,
                                        void *data);

int32_t rte_hash_add_key(const struct rte_hash *h, const void *key);

int32_t rte_hash_add_key_with_hash(const struct rte_hash *h, const void *key, hash_sig_t sig);

int32_t rte_hash_del_key(const struct rte_hash *h, const void *key);

int32_t rte_hash_del_key_with_hash(const struct rte_hash *h, const void *key, hash_sig_t sig);

int rte_hash_get_key_with_position(const struct rte_hash *h, const int32_t position, void **key);

int rte_hash_free_key_with_position(const struct rte_hash *h, const int32_t position);

int rte_hash_lookup_data(const struct rte_hash *h, const void *key, void **data);

int rte_hash_lookup_with_hash_data(const struct rte_hash *h, const void *key, hash_sig_t sig,
                                   void **data);

int32_t rte_hash_lookup(const struct rte_hash *h, const void *key);

int32_t rte_hash_lookup_with_hash(const struct rte_hash *h, const void *key, hash_sig_t sig);

hash_sig_t rte_hash_hash(const struct rte_hash *h, const void *key);

int rte_hash_lookup_bulk_data(const struct rte_hash *h, const void **keys, uint32_t num_keys,
                              uint64_t *hit_mask, void *data[]);

int rte_hash_lookup_bulk(const struct rte_hash *h, const void **keys, uint32_t num_keys,
                         int32_t *positions);

int32_t rte_hash_iterate(const struct rte_hash *h, const void **key, void **data, uint32_t *next);

uint32_t rte_jhash(const void *, uint32_t, uint32_t);

// int rte_socket_id();
};

namespace proxy {
#define Table_Entry 0x8000
    template <class K, class V> class dpdk_hash_map {
        static_assert(std::is_trivially_copyable<K>::value,
            "key type must be trivially copyable");
        static_assert(std::is_trivially_copyable<V>::value,
                      "value type must be trivially copyable");
        static_assert(sizeof(V) == 8, "value type must be 8 bytes");

    public:
        struct rte_hash *_lock_manage_table;

    public:
        dpdk_hash_map() {
            struct rte_hash_parameters params = {0};
            params.entries = Table_Entry;
            params.key_len = sizeof(K);
            params.hash_func = rte_jhash;
            params.socket_id = rte_socket_id();
            params.extra_flag = RTE_HASH_EXTRA_FLAGS_RW_CONCURRENCY_LF;

            static constexpr char kChars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            std::mt19937 mt(std::random_device{}());
            std::uniform_int_distribution<> rand(0, sizeof(kChars) / sizeof(kChars[0]) - 1);

            std::string name = "lmt-"; // lock manage table
            for(std::size_t i = 0; i < 16; ++i) {
                name += kChars[rand(mt)];
            }

            params.name = name.c_str();
            _lock_manage_table = rte_hash_create(&params);
        }

        dpdk_hash_map(const dpdk_hash_map &) = delete;
        dpdk_hash_map(dpdk_hash_map &&) = default;
        dpdk_hash_map &operator=(const dpdk_hash_map &) = delete;
        dpdk_hash_map &operator=(dpdk_hash_map &&) = default;

    public:
        int get_data(K key, V *value) {
            union {
                V v;
                void* p;
            } vv;
            const auto res = rte_hash_lookup_data(get_table(), reinterpret_cast<void *>(&key),
                                        &vv.p);
            *value = vv.v;
            return res;
        }
        int add_key_value(K key, V value) {
            union {
                V v;
                void* p;
            } vv;
            vv.v = value;
            return rte_hash_add_key_data(get_table(), reinterpret_cast<void *>(&key),
                                         vv.p);
        }
        void del_key(K key){
            rte_hash_del_key(get_table(), reinterpret_cast<void *>(&key));
        }


    public:
        rte_hash *get_table() { return _lock_manage_table; }
    };
} // namespace proxy
#endif // TRANSACTION_DPDK_HASH_MAP_HPP
