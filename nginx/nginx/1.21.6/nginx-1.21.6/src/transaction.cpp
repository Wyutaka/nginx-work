#include <proxy/types/detail/aggregator.hpp>
#include <proxy/detail/dpdk_hash_map.hpp>

#include <proxy/kc.hpp>
#include <proxy/lock.hpp>
#include <proxy/transaction.hpp>

#include <proxy/detail/span.hpp>
#include <boost/variant.hpp>

#include <algorithm>

#include <thread>

#include <atomic>
#include <vector>

#include <climits>

#include <cassert>

#include <cql_const.h>
#include <mtcp.h>
#include <parse.h>
#include <cpp17/span.hpp>

#include <rte_ring.h>

#define MBUF_MAX 1024

// mtcpと自前の実装の境界線

extern "C"
void handle_startup(proxy::Aggregator*, proxy::Node*);

extern "C"
void poll_ring_queue(void* arg)noexcept{
  auto agg = static_cast<proxy::Aggregator*>(arg);
  proxy::Aggr aggr(agg);

  auto current = aggr.current();
  auto nodes = aggr.nodes();

  if(aggr.done()){
    for(const auto& node : aggr){
      mtcp_close(aggr.mctx(), node.sockfd);
    }

    return;
  }

  if(nodes[current].state != proxy::Node::CONNECTED){
    //DebugPrint("normal sockfd = " << nodes[current].sockfd);
    handle_startup(agg, &nodes[current]);
    return;
  }

  auto ring = nodes[current].ring;

  rte_mbuf* mb[MBUF_MAX];
  //auto count = 1;//集約しないようにするため、キューから取り出すクエリ数を1にする
  auto count = rte_ring_count(ring);
  if (count > MBUF_MAX)
    count = MBUF_MAX;

  //org    | ver(1) | flags(1) | sID(2) | opc(1) | len(4) | body |
  //agr(a) | ver(1) | flags(1) | sID(2) | opc(1) | cliID(4) | length(4) | body |
  //agr(b) | ver(1) | flags(1) | sID(2) | opc(1) | len(4) | cliID(4) | body |
  //agr(c) | ver(1) | flags(1) | sID(2) | opc(1) | len(4) | body | cliID(4) |

  auto ret = rte_ring_dequeue_bulk(ring, reinterpret_cast<void**>(mb), count, nullptr);

  for (int i = 0; i < ret; i++)
  {
    auto pkt = mb[i];

    auto len = pkt->pkt_len;
    auto buf = rte_pktmbuf_mtod(pkt, char *);

    //short streamID = 0;
    //memcpy(&streamID, buf+2, sizeof(streamID));

    //thread_local short new_streamID = 0;

    //new_streamID = htons(new_streamID);
    //memcpy(buf + 2, &new_streamID, sizeof(new_streamID));

    proxy::cql_client_id cid;
    memcpy(&cid, buf + len - 4, 4);

    assert(cid.core_id < CONFIG.num_client_cores);


    //new_streamID = ntohs(new_streamID);

    //agg->id_pair[new_streamID].streamID = streamID;
    //agg->id_pair[new_streamID].clientID = cid;

    proxy::Span<char> raw_request(buf, len); //mtcp_write()で送信するためにchar*である必要がある
    uint32_t client_id;
    memcpy(&client_id, buf+len-4, sizeof(uint32_t));

    //thread_local proxy::Kc kvsConnector;
    thread_local proxy::Lock<proxy::Transaction<proxy::Kc>> lock{proxy::Transaction<proxy::Kc>{proxy::Kc{}}};
    proxy::Request req(aggr, proxy::Peer(client_id), pkt, raw_request);
    proxy::Peer p(client_id);
    proxy::Response res;
    //kvsConnector(std::move(req), res);
    lock(std::move(req), res);


    //if(new_streamID == SHRT_MAX - 1){
    //  new_streamID = 0;
    //}else{
    //  new_streamID++;
    //}
  }
  agg->current = (current+1) % agg->num_nodes;
}
