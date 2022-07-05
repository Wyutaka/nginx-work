#include <iostream>
#include <memory>

#include <proxy/detail/span.hpp>
#include <proxy/kc.hpp>

#include <cassert>
#include <cpp17/detail/utility.hpp>
#include <proxy/lock.hpp>

extern "C" struct rte_mbuf { char data[100]; };

extern "C" void rte_pktmbuf_free(rte_mbuf *mbuf) { delete mbuf; }

extern "C" ssize_t mtcp_write(mctx_t, int, const char *, std::size_t l) { return l; }

extern "C" int print_cql(char *, int n) { return n; }
int main() {
#if 1
#define QUERY "INSERT INTO ycsb.usertable (y_id,field0) VALUES ('key','value')"
#elif 0
#define QUERY "BEGIN ycsb.usertable key1 key2 key3 key4 key5"
#else
#define QUERY "SELECT y_id,field0 FROM ycsb.usertable WHERE y_id = key"
#endif
    constexpr std::size_t BODY_LENGTH_OFFSET = 1 + 1 + 2 + 1 + 4 + 4;
    constexpr std::size_t BODY_OFFSET = BODY_LENGTH_OFFSET + 4;
    const auto QUERY_LENGTH = static_cast<std::uint32_t>(cpp17::size(QUERY) - 1);
    const auto QUERY_LENGTH_BIGENDIAN = htonl(QUERY_LENGTH);

    proxy::Aggregator aggregator;
    proxy::Node nodes[1] = {};
    aggregator.nodes = nodes;
    aggregator.num_nodes = 1;
    aggregator.current = 0;

    proxy::Aggr aggr(&aggregator);

#if 1
    constexpr int COUNT = 10000000;
#else
    constexpr int COUNT = 2;
#endif

    for (int i = 0; i < COUNT; ++i) {
        auto pkt = new rte_mbuf;

        auto len = BODY_OFFSET + QUERY_LENGTH + 3;
        assert(len < sizeof(rte_mbuf));
        memcpy(pkt->data + BODY_OFFSET, QUERY, QUERY_LENGTH);
        memcpy(pkt->data + BODY_LENGTH_OFFSET, &QUERY_LENGTH_BIGENDIAN, 4);
        auto buf = pkt->data;

        proxy::Span<char> raw_request(buf, len);

        // static proxy::Kc kvsConnector;
        static proxy::Lock<proxy::Kc> lock(proxy::Kc{});
        proxy::Request req(aggr, proxy::Peer(i), pkt, raw_request);
        proxy::Response res;
        // kvsConnector(req, res);
        lock(req, res);
    }

    return 0;
}