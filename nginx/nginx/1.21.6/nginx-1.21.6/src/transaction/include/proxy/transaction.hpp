//
// Created by cerussite on 2019/10/13.
//

#pragma once

#include <memory>
#include <unordered_map>

#include <sqlite3.h>

#include <cpp17/string_view.hpp>
#include <proxy/system/hook.hpp>
#include <proxy/system/hook_context.hpp>
#include <proxy/transaction/state.hpp>
#include <proxy/types/peer.hpp>
#include <proxy/types/request/request.hpp>
#include <proxy/types/response/response.hpp>
#include <regex>

namespace proxy {
    template <class NextF> class Transaction {
    private:
        std::unordered_map<Peer, transaction::State, PeerHash> _states; 

        NextF _next; // Kc

    public:
        explicit Transaction(NextF next)
            : _states()
            , _next(std::move(next)) {}

    private:
        void _begin(Request req, Response &res) {
            _states[req.peer()] = transaction::State(); // sqliteのデータベースを作成
            if(req.aggr().order() == 0) {
                Aggr aggr = req.aggr();
                res.end(std::move(req), Status::Ok, aggr);
            }
        }
        void _commit(Request req, Response &res) {
            auto &state = _states[req.peer()];
            auto queries = state.moveAllQueries();

            auto apply_req = req.clone(std::move(queries));

            _next(std::move(apply_req), res);
            _states.erase(req.peer());
            if(req.aggr().order() == 0){
                Aggr aggr = req.aggr();
                res.end(std::move(req), Status::Ok, aggr);
            }
        }
        void _rollback(const Request &req) {
            _states.erase(req.peer());
        }
        void _lwt(Request req, Response &res){
            std::vector<char> raw_select_query_vec = createRawSelectQuery(req);
            Span<char> raw_select_query(raw_select_query_vec.data(), raw_select_query_vec.size());

            Request lwt_select = req.clone(request::RequestQuery(nullptr, raw_select_query));
            lwt_select.peer().id(UINT_FAST32_MAX);

            int16_t streamID = *(reinterpret_cast<int16_t *>(raw_select_query.data()+1+1));
            auto raw_lwt_query_variant = static_cast<const Request&&>(req).query();

            auto rq = std::make_shared<request::RequestQuery>(std::move(boost::get<request::RequestQuery>(raw_lwt_query_variant)));
            auto lreq = std::make_shared<Request>(req.clone(std::vector<request::RequestQuery>{}));

            auto ft = [this, streamID, req = lreq,
                raw_lwt_query = rq,res]
                (HookContext &response) mutable {

              response.abort();

              if(response.empty()){
                  std::vector<char> raw_insert_query_vec = createRawInsertQuery(*raw_lwt_query, streamID);
                  Span<char> raw_insert_query(raw_insert_query_vec.data(), raw_insert_query_vec.size());

                  request::RequestQuery insert_request_query(nullptr, raw_insert_query);
                  _next(std::move(req->clone(std::move(insert_request_query))), res);
              }else{
                  // TODO すでにkeyがKVSに存在する場合、軽量トランザクション失敗のレスポンスを返さなければならない
                  std::cout << "lwt if failed" << std::endl;
              }
            };
            SingletonHook::AddH処理ook(req.aggr().order(), SingletonHook::CreateKey(req.peer(), streamID), std::move(ft));
            _next(std::move(lwt_select), res);
        }

        void _normal(Request req, Response& res) {
            auto itr = _states.find(req.peer());            
            if (itr == std::end(_states)) {
                return _next(std::move(req), res); //非トランザクション 
            }

            auto rq = static_cast<const Request&&>(req).query();
            itr->second.add(std::move(boost::get<request::RequestQuery>(rq))); // イテレータの2番目の値(state)にリクエストを送る
            Aggr aggr = req.aggr(); // aggregatorの取得
            res.end(std::move(req), Status::Ok, aggr); // レスポンスを取得して,結果をキューに追加
        }

        void _select(Request req, Response& res) {
            auto itr = _states.find(req.peer());  // states ->  std::unordered_map<Peer, transaction::State, PeerHash> _states; states内の(req.peer(id))を検索してイテレータを取得,ない場合はend(コンテナの最後の要素の次)を返す
            if (itr == std::end(_states)) { // 
                return _next(std::move(req), res); //非トランザクション // 
            }
            
            auto rq = static_cast<const Request&&>(req).query();
            auto result = itr->second.select(std::move(boost::get<request::RequestQuery>(rq))); 
            Aggr aggr = req.aggr(); // aggregatorの取得
            
            res.end(std::move(req), std::move(result), aggr); // 結果をキューに追加 
                
        }

    private:
        template<class Integral>
        static void AppendToCharVector(std::vector<char>& vec, Integral num) {
            static_assert(std::is_integral<Integral>::value, "value type of num must be integral type");

            auto ptr = reinterpret_cast<const char*>(&num);
            vec.insert(std::end(vec), ptr, ptr + sizeof(Integral));
        }

        std::vector<char> createRawSelectQuery(const Request& req){
            const auto & query = boost::get<request::RequestQuery>(req.query());

            std::vector<char>raw_query(query.raw().begin(), query.raw().begin() + 1 + 1 + 2 + 1);
            int16_t streamID;
            memcpy(&streamID, query.raw().data()+2, sizeof(streamID));
            streamID = ntohs(streamID);
            streamID++;
            streamID = htons(streamID);
            memcpy(raw_query.data()+2, &streamID, sizeof(streamID));

            lwtToSelect(raw_query, boost::get<request::RequestQuery>(req.query()).body(), 2 + 1);

            int16_t consistency = htons(0x0001); //one
            char flag = 0;

            int32_t clientID;
            memcpy(&clientID, query.raw().data() + query.raw().size() - 4, sizeof(int32_t));

            AppendToCharVector(raw_query, consistency);
            raw_query.emplace_back(flag);
            AppendToCharVector(raw_query, clientID);

            return raw_query;
        }

        std::vector<char> createRawInsertQuery(const request::RequestQuery& raw_lwt_query, int16_t streamID){
            std::vector<char>raw_insert_query_vec(raw_lwt_query.raw().begin(), raw_lwt_query.raw().begin() + 1 + 1 + 2 + 1);

            //streamID++;
            //memcpy(raw_insert_query_vec.data()+2, &streamID, sizeof(streamID));

            int32_t length = ntohl(*reinterpret_cast<const std::int32_t*>(raw_lwt_query.raw().data() + 5));
            length -= 13; // IF NOT EXISTS
            AppendToCharVector(raw_insert_query_vec, htonl(length));

            int32_t body_len = ntohl(*reinterpret_cast<const std::int32_t*>(raw_lwt_query.raw().data() + 9));
            body_len -= 13; // IF NOT EXISTS
            AppendToCharVector(raw_insert_query_vec, htonl(body_len));

            auto body_first = raw_lwt_query.raw().begin()+13; // 1 + 1 + 2 + 1 + 4 + 4;
            raw_insert_query_vec.insert(raw_insert_query_vec.end(), body_first, body_first+body_len);

            int16_t consistency = htons(0x0001); //one
            char flag = 0;

            AppendToCharVector(raw_insert_query_vec, consistency);
            raw_insert_query_vec.emplace_back(flag);

            int32_t clientID;
            memcpy(&clientID, raw_lwt_query.raw().data() + raw_lwt_query.raw().size() - 4, sizeof(int32_t));

            AppendToCharVector(raw_insert_query_vec, clientID);

            return raw_insert_query_vec;
        }
        void lwtToSelect(std::vector<char> &raw_query, cpp17::string_view lwt_query, int32_t parameter_length){
            static const std::regex INSERT_IF_NOT_EXISTS(
                R"(INSERT\s+INTO\s+([\w\d.]+)\s*\(([\w\d,\s]+)\)\s*VALUES\s*\(([\w\d,\s']+)\)\s+IF\s+NOT\s+EXISTS)",
                std::regex_constants::icase);
            std::cmatch cm;

            if(!std::regex_match(std::begin(lwt_query), std::end(lwt_query), cm, INSERT_IF_NOT_EXISTS)){
                return;
            }

            auto table = cpp17::string_view(cm[1].first, cm[1].length());

            auto keys_sv = cpp17::string_view(cm[2].first, cm[2].length());
            keys_sv = keys_sv.substr(0,keys_sv.find(','));

            auto values_sv = cpp17::string_view(cm[3].first, cm[3].length());
            values_sv = values_sv.substr(0,values_sv.find(','));

            auto select_query = "SELECT " + keys_sv + "FROM " + table + " WHERE " + keys_sv + "= " + values_sv + " PER PARTITION LIMIT 1";

            int32_t body_length = htonl(select_query.length());

            int32_t whole_length = htonl(sizeof(int32_t) + ntohl(body_length) + parameter_length);

            AppendToCharVector(raw_query, whole_length);
            AppendToCharVector(raw_query, body_length);
            raw_query.insert(raw_query.end(), select_query.begin(), select_query.end());
        }

    public:
        void operator()(Request req, Response &res) {
            const auto &query = boost::get<request::RequestQuery>(req.query());

            switch (query.type()) {
            case request::RequestQuery::Type::Begin:
                _begin(std::move(req), res);
                break;
            case request::RequestQuery::Type::Commit:
                _commit(std::move(req),res);
                break;
            case request::RequestQuery::Type::Rollback:
                _rollback(req);
                break;
            case request::RequestQuery::Type::Lwt:
                _lwt(std::move(req), res);
                break;
            case request::RequestQuery::Type::Unknown:
                _next(std::move(req), res);
                break;
            case request::RequestQuery::Type::Select:
                _select(std::move(req), res);
                break;
            default:
                _normal(std::move(req), res);
            }
        }
    };
} // namespace proxy
