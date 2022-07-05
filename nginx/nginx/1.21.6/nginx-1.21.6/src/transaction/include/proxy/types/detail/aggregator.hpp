#ifndef _class_Aggr
#define _class_Aggr

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#include "aggregator_types.h"
#include <iostream>

namespace proxy {
    class Aggr {
    private:
        Aggregator *aggr;

    public:
        Aggr(Aggregator *arg)
            : aggr(arg) {}

    public:
        int done() const noexcept { return aggr->done; }

        mctx_t mctx() const noexcept { return aggr->mctx; }

        Node *nodes() noexcept { return aggr->nodes; }
        const Node *nodes() const noexcept { return aggr->nodes; }

        Node *begin() noexcept { return nodes(); }
        Node *end() noexcept { return begin() + num_nodes(); }

        int num_nodes() const noexcept { return aggr->num_nodes; }
        int n_node() const noexcept { return aggr->n_node; }

        int current() const noexcept { return aggr->current; }

        int order() const noexcept { return aggr->order; }

        short getNewStreamIDAndIncrement() {
            aggr->new_streamID++;
            if(aggr->new_streamID == SHRT_MAX) {
                aggr->new_streamID = 0;
            }
            return aggr->new_streamID;
        };

        stream_client_id* getStreamIdClientIdTable(){ return aggr->id_pair;}

        Aggregator* get_aggr(){ return aggr; }

        Aggregator::Stat &stat() noexcept { return aggr->stat; }
        const Aggregator::Stat &stat() const noexcept { return aggr->stat; }
        void update_stat(bool isOk) {
            if (likely(isOk)) {
                stat().n_transmitted++;
                stat().n_processed++;
            } else {
                stat().n_dropped++;
            }
        }
    };
} // namespace proxy
#endif