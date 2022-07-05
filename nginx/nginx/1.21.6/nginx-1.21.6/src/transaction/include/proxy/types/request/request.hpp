//
// Created by cerussite on 2019/10/11.
//

#pragma once

#include <string>
#include <vector>

#include <boost/variant.hpp>
#include <proxy/types/peer.hpp>
#include <proxy/types/request/request_query.hpp>

namespace proxy {
    /*次の階層に渡すためのクラス
     * Requestクラスはrequest::RequestQuery(クエリの種類、Key、Value)とPeer(クライアントを識別するためのid)をメンバに持つ*/
    class Request {
    private:
        // std::vector<request::RequestQuery> _queries;
        // request::RequestQuery _query;
        mutable boost::variant<request::RequestQuery, std::vector<request::RequestQuery>> _queries;
        Peer _peer;

        mutable Aggr _aggregator;

        // private:
        //    static request::RequestQuery _torequest::RequestQuery(const std::string &s) { return
        //    request::RequestQuery(s); }
    private:
        Request(int, Aggr agr, Peer p,
                boost::variant<request::RequestQuery, std::vector<request::RequestQuery>> queries)
            : _queries(std::move(queries))
            , _aggregator(std::move(agr))
            , _peer(std::move(p)) {
        }

    public:
        Request(Aggr agr, Peer p, std::vector<request::RequestQuery> queries)
            : Request(0, agr, std::move(p), std::move(queries)) {}

        Request(Aggr agr, Peer p, request::RequestQuery query)
            : Request(0, agr, std::move(p), std::move(query)) {
        }


        Request(const Aggr &agr, const Peer &p,
                const request::RequestQuery::raw_packet_type &packet,
                const request::RequestQuery::raw_request_type &query)
            : Request(agr, p, request::RequestQuery{packet, query}) {}

    public:
       const boost::variant<request::RequestQuery, std::vector<request::RequestQuery>> &
        query() const & {
            return _queries;
        }

        boost::variant<request::RequestQuery, std::vector<request::RequestQuery>>
        query() const && {
            return std::move(_queries);
        }


        Request clone(request::RequestQuery query) const {
            return Request(_aggregator, peer(), std::move(query));
        }
        Request clone(std::vector<request::RequestQuery> queries) const {
            return Request(_aggregator, peer(), std::move(queries));
        }

        const Peer &peer() const { return _peer; }
        Peer &peer()  { return _peer; }

        mctx_t mtcpContext() const noexcept { return _aggregator.mctx(); }

        int currentSocket() const noexcept {
            return _aggregator.nodes()[_aggregator.current()].sockfd;
        }

        void updateStatus(bool isOk) const { _aggregator.update_stat(isOk); }

        short getNewStreamID(){return _aggregator.getNewStreamIDAndIncrement();}

        stream_client_id* getStreamIdClientIdTable(){return _aggregator.getStreamIdClientIdTable();}

        Aggr aggr(){return _aggregator;}

        struct var_printer : boost::static_visitor<void> {
            void operator()(request::RequestQuery req) const {
                std::cout << req.body_length() << "\n";
            }
        };
    };
} // namespace proxy
