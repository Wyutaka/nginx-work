#pragma once

#include <pthread.h>
// #include <rte_ring.h>
// #include <rte_mbuf.h>

// #include <rte_hash.h>
// #include <rte_jhash.h>
// #include "mtcp_api.h"

#include <limits.h>

#ifdef __cplusplus
namespace proxy {
    extern "C"{
#endif
    struct cql_client_id{
        int32_t id:24 , core_id:8;
        //int id ;
        //int core_id;
    } __attribute__((packed));

    struct stream_client_id{
        struct cql_client_id clientID;
        short streamID;
    };

    typedef struct
    {
        unsigned int address;
        struct rte_ring *ring;
        int sockfd;
        enum
        {
            STARTUP = 0,
            STARTUP_SENT,
            REGISTER,
            REGISTER_SENT,
            USE_YCSB,
            USE_YCSB_SENT,
            CONNECTED
        } state;
    } Node;

    typedef struct
    {
        int core;
        int order;
        pthread_t tid;
        int done;

        // mctx_t mctx; // int cpuのみ
        int cpu;

        Node *nodes;

        int num_nodes;
        int n_node;
        int current;

        struct rte_hash_parameters params;
        struct rte_hash *streamID_clientID_table;

        struct Stat
        {
            int n_processed;
            int n_dropped;
            int n_transmitted;
            int n_received;
        } stat;
        int timer;

        struct stream_client_id id_pair[SHRT_MAX];

        short new_streamID;

    } Aggregator;
#ifdef __cplusplus
    }
} // namespace proxy
#endif
