//
// Created by cerussite on 2019/10/12.
//

#pragma once

#include <regex>
#include <string>
#include <vector>

// #include <cpp17/optional.hpp>
#include <string_view>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <proxy/detail/debug.hpp>
#include <proxy/detail/pipes.hpp>
#include <proxy/detail/span.hpp>
#include <proxy/detail/str.hpp>
#include <proxy/types/detail/aggregator.hpp>
#include <proxy/types/detail/header.hpp>

extern "C" struct rte_mbuf;
extern "C" void rte_pktmbuf_free(struct rte_mbuf *m);

namespace proxy {

    struct mbuf_delete {
        constexpr mbuf_delete() noexcept = default;

        ~mbuf_delete() = default;

        void operator()(rte_mbuf *ptr) const { rte_pktmbuf_free(ptr); };
    };

    namespace request {
        class RequestQuery {
        public:
            enum class Type {
                Unknown,
                Select,
                Insert,
                Lwt,
                Update,
                Begin,
                Commit,
                Rollback,
            };

            using Header = detail::Header;
            using raw_request_type = Span<char>;
            using raw_packet_type = rte_mbuf *;

        private:
            raw_request_type _rawRequest; //要求となるクエリ本体
            raw_packet_type _packet;

        private:
            // Header _header;
            // uint32_t _cliID;
            /*long string*/
            uint32_t _body_length;
            cpp17::string_view _body;

            Type _type; //クエリのタイプ

            // cpp17::optional<std::string> _table; //クエリが指定するtable
            cpp17::optional<cpp17::string_view> _key;
            cpp17::optional<std::vector<cpp17::string_view>> _keys;
            //クエリが指定するkey・value

        private:
            static Type _getType(
                const cpp17::string_view &body) { //クエリがどのタイプであるかを判断するメンバ関数
                //print_buffer((uchar*)body.data(), 1);
                // auto q = body | tolower;

                if(body.empty()){
                    DebugPrint("");
                    return Type::Unknown;
                }
                switch (body.front()) {
                case 's':
                case 'S':
                    //DebugPrint("select");
                    return Type::Select;
                case 'i':
                case 'I':
                    //DebugPrint("insert");
                    if (body.back() == 'S' || body.back() == 's')
                        return Type::Lwt;
                    else
                        return Type::Insert;
                case 'u':
                case 'U':
                    //DebugPrint("update");
                    return Type::Update;
                case 'b':
                case 'B':
                    return Type::Begin;
                case 'c':
                case 'C':
                    //DebugPrint("commit");
                    return Type::Commit;
                case 'r':
                case 'R':
                    //DebugPrint("rollback");
                    return Type::Rollback;
                default:
                    DebugPrint("unknown");
                    return Type::Unknown;
                }
            }
            cpp17::string_view insert_query_parse_key(const char *body, size_t body_len) {
                int i = 0; //最初の'の位置
                int j = 0; //二番目の'の位置

                while (true) { /*body内の最初の'を探索*/
                    if (unlikely(body[i++] == '\'')) {
                        j = i + 1;
                        break;
                    }
                    if(unlikely(i > body_len)){
                        return{nullptr, 0};
                    }
                }
                while (true) { /*body内の二番目の'を探索*/
                    if (unlikely(body[j++] == '\'')) {
                        break;
                    }
                    if(unlikely(j > body_len)){
                        return{nullptr, 0};
                    }
                }

                return {body + i, static_cast<std::size_t>(j - i - 1)};
            }

            cpp17::string_view update_query_parse_key(const char *body, size_t body_len){
                int pos = 0; // keyの先頭位置
                int key_len = 0;
                int i = 0;

                while(true){
                    if(unlikely(body[i++] == '\'')) break;
                    if(unlikely(i > body_len)){
                        return{nullptr, 0};
                    }
                }

                while(true){
                    if(unlikely(body[i++] == '\'')) break;
                    if(unlikely(i > body_len)){
                        return{nullptr, 0};
                    }
                }

                while(true){
                    if(unlikely(body[i++] == '\'')) break;
                    if(unlikely(i > body_len)){
                        return{nullptr, 0};
                    }
                }

                pos = i;

                while(true){
                    if(unlikely(body[i++] == '\'')) break;
                    if(unlikely(i > body_len)){
                        return{nullptr, 0};
                    }
                    key_len++;
                }
                return {body + pos, static_cast<std::size_t>(key_len)};
            }

            cpp17::string_view select_query_parse_key(const char *body, size_t body_len){
                int pos = 0; // keyの先頭位置
                int key_len = 0;
                int i = 0;

                while(true){
                    if(unlikely(body[i++] == '\'')) break;
                    if(unlikely(i > body_len)){
                        return {nullptr, 0};
                    }
                }

                pos = i;

                while(true){
                    if(unlikely(body[i++] == '\'')) break;
                    key_len++;
                }

                return {body + pos, static_cast<std::size_t>(key_len)};
            }

            std::vector<cpp17::string_view>
            begin_query_parse_key(const cpp17::string_view &body_sv) {
                std::vector<cpp17::string_view> &keys = _keys.emplace();
                size_t pre_space_pos = 0;
                size_t next_space_pos = 0;

                while (true) {
                    next_space_pos = body_sv.find(' ', pre_space_pos);
                    if (unlikely(next_space_pos == cpp17::string_view::npos))
                        break;

                    keys.emplace_back(
                        body_sv.substr(pre_space_pos, next_space_pos - pre_space_pos));
                    pre_space_pos = next_space_pos + 1;
                }
                keys.emplace_back(body_sv.substr(pre_space_pos));
                return keys;
            }

        private:
            void _setSelectTable() {
               /*SELECT * from usetable where y_id = 'key'*/
                static constexpr auto SELECT_SKIP_LENGTH = 26; //SELECTクエリの飛ばしても良い最大の長さ
                _key = select_query_parse_key(body().data() + SELECT_SKIP_LENGTH, body().size()-30);
            }

            void _setInsertTable() {
                /*INSERT INTO ycsb.usertable (y_id,field0) VALUES ('key','value')*/
                static constexpr auto INSERT_SKIP_LENGTH = 44;
                _key = insert_query_parse_key(body().data() + INSERT_SKIP_LENGTH, body().size() - INSERT_SKIP_LENGTH);
            }

            void _setLwtTable() {
                /*INSERT INTO ycsb.usertable (y_id,field0) VALUES ('key','value') IF NOT EXISTS*/
                static constexpr auto INSERT_SKIP_LENGTH = 44;
                _key = insert_query_parse_key(body().data() + INSERT_SKIP_LENGTH, body().size() - INSERT_SKIP_LENGTH);
            }

            void _setUpdateTable() {
                /*UPDATE ycsb.usertable set field0 = 'value' where y_id = 'key'*/
                static constexpr auto UPDATE_SKIP_LENGTH = 24; //UPDATEクエリの飛ばしても良い最大の長さ
                _key = update_query_parse_key(body().data() + UPDATE_SKIP_LENGTH, body().size() - UPDATE_SKIP_LENGTH);
            }

            void _setBeginParameters() {
                /* transaction query: BEGIN [TRANSACTION] [IN] [keyspace.] table [WHERE key = value] */
                static constexpr auto BEGIN_SKIP_LENGTH = 6;
                //begin_query_parse_key(body().substr(BEGIN_SKIP_LENGTH));
                //_key = nullptr;
            }

            void _setParameter() {
                switch (type()) {
                case Type::Select:
                    _setSelectTable();
                    break;
                case Type::Insert:
                    _setInsertTable();
                    break;
                case Type::Lwt:
                    _setLwtTable();
                    break;
                case Type::Update:
                    _setUpdateTable();
                    break;
                case Type::Begin:
                    //_setBeginParameters();
                    break;
                case Type::Commit:
                    break;
                case Type::Rollback:
                    break;
                default:
                    DebugPrint("");
                    break;
                }
            }

        private:
            static Header _getheader(const std::vector<char> &query) {
                Header header;
                // std::memcpy(&header.version, &query[0], sizeof(header.version));

                // std::memcpy(&header.flags, &query[1], sizeof(header.flags));

                // std::memcpy(&header.stream, &query[2], sizeof(header.stream));
                // header.stream = htons(header.stream);

                // std::memcpy(&header.opcode, &query[4], sizeof(header.opcode));

                // std::memcpy(&header.length, &query[5], sizeof(header.length));
                // header.length = htonl(header.length);
                // std::reverse_iterator<std::string::const_iterator> first(query.begin() +
                // sizeof(Header)), last(
                //        query.begin());
                // std::memcpy(&header, query.data(), sizeof(Header));
                auto first = std::begin(query);
                auto last = first + sizeof(Header);
                std::uninitialized_copy(first, last, reinterpret_cast<char *>(&header));
                header.stream = htons(header.stream);
                header.length = htonl(header.length);
                return header;
            }

            static uint32_t _getcliID(const std::vector<char> &query) {
                uint32_t cliID;
                auto first = std::begin(query) + sizeof(Header);
                auto last = first + sizeof(uint32_t);
                std::uninitialized_copy(first, last, reinterpret_cast<char *>(&cliID));
                // std::memcpy(&cliID, &query[9], sizeof(cliID));
                cliID = htonl(cliID);
                return cliID;
            }

            static uint32_t _getbody_length(const raw_request_type &query) {
                uint32_t body_length;
                //DebugPrint(query.size());
                //auto first = query.begin() + sizeof(Header) + sizeof(uint32_t);
                if(query.size() == sizeof(Header) + sizeof(uint32_t)){
                    return 0;
                }
                auto first = query.begin() + sizeof(Header);
                auto last = first + sizeof(uint32_t);
                // std::uninitialized_copy(first, last, reinterpret_cast<char *>(&body_length));
                std::memcpy(&body_length, first, sizeof(body_length));
                body_length = htonl(body_length);
                return body_length;
            }

            static cpp17::string_view _getbody(const raw_request_type &query,
                                               uint32_t body_length) {
                // std::vector<char> buf(body_length);
                // std::memcpy(buf.data(), &query[17], body_length);
                // std::string body(buf.data(), body_length);
                //auto first = query.begin() + sizeof(Header) + sizeof(uint32_t) + sizeof(uint32_t);
                if(body_length == 0){
                    DebugPrint("");
                    cpp17::string_view body;
                    return body;
                }
                auto first = query.begin() + sizeof(Header) + sizeof(uint32_t);
                // auto last = first + body_length;
                // std::string body(last - first, '\0');
                // std::uninitialized_copy(first, last, std::begin(body));
                cpp17::string_view body(first, body_length);
                //DebugPrint("body = " << body);
                return body;
            }

        public:
            // Header header() const { return _header; }

            // uint32_t cliID() const { return _cliID; }

            uint32_t body_length() const { return _body_length; }

            const cpp17::string_view &body() const { return _body; }

        public:
            RequestQuery() = delete;

            RequestQuery(raw_packet_type packet, raw_request_type query)
                : _rawRequest(std::move(query))
                , _packet(std::move(packet))
                //    , _header(_getheader(_rawRequest))
                //    , _cliID(_getcliID(_rawRequest))
                , _body_length(_getbody_length(_rawRequest))
                , _body(_getbody(_rawRequest, _body_length))
                , _type(_getType(_body)) {
                _setParameter();
            }

            RequestQuery(const RequestQuery &) = delete;

            RequestQuery(RequestQuery &&query) noexcept
                : _rawRequest(query._rawRequest)
                , _packet(query._packet)
                , _body_length(query._body_length)
                , _body(query._body)
                , _type(query._type)
                , _key(std::move(query._key))
                , _keys(std::move(query._keys)){
                query._packet = nullptr;
            }

            RequestQuery &operator=(const RequestQuery &) = delete;

            RequestQuery &operator=(RequestQuery &&query) noexcept {
                _rawRequest = query._rawRequest;

                _packet = query._packet;
                query._packet = nullptr;
                _body_length = query._body_length;
                _body = query._body;
                _type = query._type;
                _key = std::move(query._key);
                _keys = std::move(query._keys);

                return *this;
            }

            ~RequestQuery() {
                if (_packet != nullptr) {
                    rte_pktmbuf_free(_packet);
                }
            }

        public:
            Type type() const noexcept { return _type; }

#define IsQ(t)                                                                                     \
    bool is##t() const noexcept { return type() == Type::t; }

            IsQ(Select);

            IsQ(Insert);

            IsQ(Lwt);

            IsQ(Update);

            IsQ(Begin);

            IsQ(Commit);

            IsQ(Rollback);
#undef IsQ

        public:
            const raw_request_type &raw() const { return _rawRequest; };

        public:
            // const cpp17::optional<std::string> &table() const { return _table; }
            const cpp17::optional<cpp17::string_view> &key() const { return _key; }
            const cpp17::optional<std::vector<cpp17::string_view>> &keys() const { return _keys; }
        };
    } // namespace request
} // namespace proxy
