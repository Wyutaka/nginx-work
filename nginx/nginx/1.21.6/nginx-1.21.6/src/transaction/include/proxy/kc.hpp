//
// Created by miwa on 2019/10/28.
//

#ifndef TRANSACTION_KC_HPP
#define TRANSACTION_KC_HPP

#include <cstdint>
// #include <murmur.h>
#include <parse.h>
#include <TokenToAddress.h>
#include <proxy/detail/debug.hpp>
#include <proxy/types/request/request.hpp>
#include <proxy/types/response/response.hpp>
// #include "../../debug.hpp"

namespace proxy {
    class Kc {
    public:
        Kc() = default;

    private:
        void registerStreamIdClientId(Request &req){
            auto& raw_query = boost::get<request::RequestQuery>(req.query()).raw();

            short original_streamID;
            memcpy(&original_streamID, raw_query.data() + 2, sizeof(original_streamID));

            cql_client_id cid;
            memcpy(&cid, raw_query.data() + raw_query.size() - 4, sizeof(int32_t));

            short new_streamID = req.getNewStreamID();

            auto table = req.getStreamIdClientIdTable();
            table[new_streamID].streamID = original_streamID;
            table[new_streamID].clientID = cid;

            new_streamID = htons(new_streamID);
            memcpy(raw_query.data() + 2, &new_streamID, sizeof(new_streamID));
        }

        void registerStreamIdsClientIds(Request &req){
            auto& queries = boost::get<std::vector<request::RequestQuery>>(req.query());
            for(auto& query : queries){
                auto& raw = query.raw();
                short original_streamID;
                memcpy(&original_streamID, raw.data() + 2, sizeof(original_streamID));

                cql_client_id cid;
                memcpy(&cid, raw.data() + raw.size() - 4, sizeof(int32_t));

                short new_streamID = req.getNewStreamID();

                auto table = req.getStreamIdClientIdTable();
                table[new_streamID].streamID = original_streamID;
                table[new_streamID].clientID = cid;

                new_streamID = htons(new_streamID);
                memcpy(raw.data() + 2, &new_streamID, sizeof(new_streamID));
            }

        }

    public:
        void operator()(Request req, Response &res) { 
            fprintf(stdout, "!!!! Kc operator is called \n");
            if (req.query().which() == 0) { // クエリの個数が1の時
                registerStreamIdClientId(req);

                const auto& raw = boost::get<request::RequestQuery>(req.query()).raw();

                auto n = mtcp_write(req.mtcpContext(), req.currentSocket(), raw.data(),
                raw.size_bytes() - sizeof(int32_t));

                req.updateStatus(n == raw.size_bytes());
            } else { // クエリの個数が複数の時
                registerStreamIdsClientIds(req);
                const auto& queries = boost::get<std::vector<request::RequestQuery>>(req.query());

                for(const auto &query : queries){
                    const auto& raw = query.raw();
                    // fprintf("---------------query is : %d", query);
                    auto n = mtcp_write(req.mtcpContext(), req.currentSocket(), raw.data(),raw.size_bytes() - sizeof(int32_t));
                }
            }
        }
    };
} // namespace proxy

#endif // TRANSACTION_KC_HPP
