//
// Created by miwa on 2019/10/18.
//

#ifndef TRANSACTION_RESPONSE_DATA_HPP
#define TRANSACTION_RESPONSE_DATA_HPP

#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <proxy/detail/unique_ptr.hpp>
#include <proxy/types/detail/header.hpp>
#include <proxy/types/response/response.hpp>
#include <proxy/types/response/response_body.hpp>
#include <string>
#include <vector>

namespace proxy {
    enum class Status { Ok, Invalid, AlreadyLocked };

    namespace response {
        class ResponseData {
            using raw_response_data = Span<char>;

        private:
            detail::unique_ptr<char, std::default_delete<char>> _response_data_area;
            raw_response_data _rawResponse;

        public:
            ResponseData() = default;

            // explicit ResponseData(std::string query)
            //    : _rawResponse(std::move(query))
            //    ,
            //    //_header(_getheader(_rawResponse)),
            //    //_cliID(_getcliID(_rawResponse)),
            //    _body(_getbody(_rawResponse)) {}

            ResponseData(const ResponseData &) = delete;

            ResponseData(ResponseData &&) = default;

            ResponseData &operator=(const ResponseData &) = delete;

            ResponseData &operator=(ResponseData &&) = default;

            ~ResponseData() = default;

            ResponseData(const request::RequestQuery &req, Span<char> response_body) // resposedataを作るだけ
                : _response_data_area(new char[9 + response_body.size_bytes() + 4]),
                 _rawResponse(_response_data_area.get(),
                               9 + response_body.size_bytes() + 4) {

                memcpy(_response_data_area.get(), req.raw().data(), 9);
                // fprintf(stderr, "reqenst_query_type is ::%d", req.type());
                _response_data_area[0] = 0x84; // response version
                
                if(req.type() == request::RequestQuery::Type::Begin || req.type() == request::RequestQuery::Type::Commit || req.type() == request::RequestQuery::Type::Rollback) {
                  _response_data_area[4] = 0x65; // resend   
                } else  {
                    //TODO select以外の挙動
                  _response_data_area[4] = 0x08; // response version
                }
                // _response_data_area[4] = 0x02; // flags 

                  _response_data_area[4] = 0x65; // resend 苦肉の策 下を通らないっぽい
                uint32_t length = response_body.size_bytes();
                length = htonl(length);
                memcpy(_response_data_area.get() + 5, &length, sizeof(uint32_t)); // stream ID

                memcpy(_response_data_area.get() + 9, response_body.data(), response_body.size_bytes()); // opcode

                memcpy(_response_data_area.get() + 9 + response_body.size_bytes(), req.raw().end() - sizeof(uint32_t), sizeof(uint32_t)); // body(reqはいる？)
            }

            ResponseData(const request::RequestQuery &req, Status status)
               : ResponseData(req, status_body(status)){ // l.49のResponseDataを読んでる
            }

        private:
            static Span<char> status_body(Status s) {
                switch (s) {
                case Status::Invalid:
                    return {invalid, sizeof(invalid)};
                case Status::AlreadyLocked:
                    return {lock_failed, sizeof(lock_failed)};
                case Status::Ok:
                    return {ok, sizeof(ok)};
                }
            }

        public:
            const raw_response_data &get() const { return _rawResponse; }
        };
    } // namespace response
} // namespace proxy



#endif // TRANSACTION_RESPONSE_DATA_HPP
