//
// Created by miwa on 2020/03/12.
//
#include <parse.h>
#include <proxy/system/hook.hpp>
#include <proxy/system/hook_context.hpp>

#ifdef __cplusplus
namespace proxy {
extern "C" {
#endif
    void print_results(const cql_header *, Aggregator *, char *);

    void CallHook(char *raw_response, int len, cql_header header,
                  Aggregator *aggr) { // hookに登録しているstreamIDはnew_streamID
                  
        // fprintf(stderr, "test from detailのhookだお");
        HookContext response(Span<char>(raw_response, len));

        uint32_t client_id = *(reinterpret_cast<uint32_t*>(&aggr->id_pair[header.streamID].clientID));
        Peer peer(client_id);

        int16_t original_streamID = *(reinterpret_cast<uint32_t*>(&aggr->id_pair[header.streamID].streamID));

        SingletonHook::CallAndRemove(aggr->order, SingletonHook::CreateKey(peer, original_streamID), response);

        if (!response.continueResponse()) {
            return;
        }

        print_results(&header, aggr, raw_response);
    }
#ifdef __cplusplus
    }
} // namespace proxy
#endif
