//
// Created by cerussite on 2019/10/11.
//

#pragma once

// #include <cpp17/any.hpp>
#include <cstdint>
#include <proxy/types/request/request.hpp>
#include <proxy/types/request/request_query.hpp>
#include <proxy/types/response/enqueue_result.hpp>
#include <proxy/types/response/response_data.hpp>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <proxy/types/response/result.hpp>
#include <boost/variant/variant.hpp>

namespace proxy {
    // enum class Status {
    //    AlreadyLocked
    //};

    class Response {
    private:

    public:
        Response() = default;                 //デフォルトコンストラクタ
        Response(const Response &) = default; //コピーコンストラクタ
        Response(Response &&) = default;      //ムーブコンストラクタ

        Response &operator=(const Response &) = default; //コピー代入演算子
        Response &operator=(Response &&) = default;      //ムーブ代入演算子

    public:

    public:

        void end(Request req_query, Status s, Aggr aggr) {
            response::ResponseData one_response(boost::get<request::RequestQuery>(req_query.query()), s); // レスポンスの作成
            EnqueueResult(one_response, aggr); // リザルトに入れる クライアントに返す(多分)
        }

        struct respoinse_visitor : boost::static_visitor<const std::vector<unsigned char>&> {

            const std::vector<unsigned char>& operator()(response::Result result, int) const { return result.bytes();}
            // const std::vector<unsigned char>& operator()(response::GetWhse warehouse, float) { return warehouse.bytes();} 
            const std::vector<unsigned char>& operator()(response::GetWhse district) const { return district.bytes();} 
        };

        void end(Request req_query, response::Result res, Aggr aggr) {
            auto& bytes = res.bytes();
            response::ResponseData one_response(boost::get<request::RequestQuery>(req_query.query()), {static_cast<char*>(static_cast<void*>(bytes.data())), bytes.size()}); // レスポンスの作成
            EnqueueResult(one_response, aggr); 
        }

        void end(Request req_query, response::GetWhse res, Aggr aggr) {
            auto& bytes = res.bytes();
            response::ResponseData one_response(boost::get<request::RequestQuery>(req_query.query()), {static_cast<char*>(static_cast<void*>(bytes.data())), bytes.size()}); // レスポンスの作成
            EnqueueResult(one_response, aggr); 
        }        

        void end(Request req_query, boost::variant<response::Result, response::GetDist, response::GetWhse, response::GetCust, response::GetItem, response::GetStock> res, Aggr aggr) {
            std::vector<unsigned char> *bytes = nullptr;
            if (res.type() == typeid(response::Result)) {
                bytes = &boost::get<response::Result>(res).bytes();
            } else if (res.type() == typeid(response::GetDist)) {
                bytes = &boost::get<response::GetDist>(res).bytes();
            } else if (res.type() == typeid(response::GetWhse)) {
                bytes = &boost::get<response::GetWhse>(res).bytes();
            } else if (res.type() == typeid(response::GetCust)) {
                bytes = &boost::get<response::GetCust>(res).bytes();
            } else if (res.type() == typeid(response::GetItem)) {
                bytes = &boost::get<response::GetItem>(res).bytes();
            } else if (res.type() == typeid(response::GetStock)) {
                bytes = &boost::get<response::GetStock>(res).bytes();
            }
            
            response::ResponseData one_response(boost::get<request::RequestQuery>(req_query.query()), {static_cast<char*>(static_cast<void*>(bytes->data())), bytes->size()}); // レスポンスの作成
            EnqueueResult(one_response, aggr); 
        }        
    };
} // namespace proxy
