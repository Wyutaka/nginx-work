//
// Created by cerussite on 2019/10/13.
//

#pragma once

#include <random>
#include <string>
#include <vector>

#include <sqlite3.h>
#include <zconf.h>

#include <proxy/detail/unique_id.hpp>
#include <proxy/types/peer.hpp>
#include <proxy/types/request/request_query.hpp>
#include <proxy/types/response/result.hpp>

#include <boost/variant/variant.hpp>

// 抽出結果が返るコールバック関数
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++)
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   return SQLITE_OK;
}


namespace proxy {
    namespace transaction {
        class State {
        private:
            std::string _sqliteFile;
            sqlite3 *_s3;
            std::vector<request::RequestQuery> _queries; // クエリ保存領域

        public:
            State()
                : _sqliteFile(":memory:"
                              ) //:memory:にするとインメモリ
                , _s3()
                , _queries() {
                sqlite3_open_v2(_sqliteFile.c_str(), &_s3, SQLITE_OPEN_READWRITE, nullptr);
                std::string create_table = "create table bench(pk TEXT primary key, field1 INTEGER, field2 INTEGER, field3 INTEGER);"; 
                char *errMsg = nullptr;
                sqlite3_exec(_s3, create_table.c_str(), nullptr, nullptr, &errMsg);
                createTPCCDatabase();
            }

            State(const State &) = delete;
            State(State &&) = default;
            State &operator=(const State &) = delete;
            State &operator=(State && rhs) noexcept {
                // reset();

                _s3 = rhs._s3;
                rhs._s3 = nullptr;
                _sqliteFile = std::move(rhs._sqliteFile);
                // _queriesWAL = std::move(rhs._queriesWAL);
                _queries = std::move(rhs._queries);
                return *this;
            }

            ~State() {
                reset();
            }

        private:
            void reset() noexcept {
                sqlite3_close_v2(_s3);
                ::unlink(_sqliteFile.c_str());
            }
        public:

            bool executeOnly(const request::RequestQuery &query) {
                std::cout << "executeOnly" << "\n";
                char *errMsg = nullptr;
                const char *body = query.body().str().c_str();
                auto res = sqlite3_exec(_s3, query.body().str().c_str(), nullptr, nullptr, &errMsg);
                std::cout << "execute result is " << res << "\n"; // 0

                return res == SQLITE_OK;
            }

            void addQueryOnly(request::RequestQuery query) { _queries.emplace_back(std::move(query)); }

            boost::variant<response::Result, response::GetDist, response::GetWhse, response::GetCust, response::GetItem, response::GetStock> select(request::RequestQuery query) { 
            loadTPCCData(); // TODO 別の場所に移動させる
            sqlite3_stmt *stmt;
            int prepare = sqlite3_prepare_v2(_s3, query.body().str().c_str(), -1, &stmt, nullptr);
            // fprintf(stderr, "sqlite3_prepare is %d\n", prepare );

            int result = sqlite3_step(stmt);
            // fprintf(stderr, "sqlite3_step result is %d\n", result);
            if(result == SQLITE_ROW || result == SQLITE_DONE) { // DONEになる？
                int column_count = sqlite3_column_count(stmt);
                std::string column_name = std::string(sqlite3_column_name(stmt, 0));
                if (column_name.compare("d_next_o_id") == 0 || column_name.compare("d_tax") == 0 ) {
                    response::GetDist res("tiny_tpc_c", "district", {"d_next_o_id", "d_tax"}, column_count) ;
                    res.addRow(sqlite3_column_int(stmt, 0), float(sqlite3_column_double(stmt, 1)));
                    addQueryOnly(std::move(query));
                    return res;
                } else if(column_name.compare("w_tax") == 0) {
                    response::GetWhse res("tiny_tpc_c", "warehouse", {"w_tax"}, column_count);
                    res.addRow(float(sqlite3_column_int(stmt, 0)));
                    addQueryOnly(std::move(query));
                    return res;
                } else if(column_name.compare("c_discount") == 0) {
                    response::GetCust res("tiny_tpc_c", "customer", {"c_discount", "c_last", "c_credit"}, column_count);
                    res.addRow(float(sqlite3_column_int(stmt, 0)), std::string(sqlite3_column_name(stmt, 1)), std::string(sqlite3_column_name(stmt, 2)));
                    addQueryOnly(std::move(query));
                    return res;
                } else if(column_name.compare("i_name") == 0) {
                    response::GetItem res("tiny_tpc_c", "item", {"i_name", "i_price", "i_data"}, column_count);
                    res.addRow(std::string(sqlite3_column_name(stmt, 0)), float(sqlite3_column_int(stmt, 1)), std::string(sqlite3_column_name(stmt, 2)));
                    addQueryOnly(std::move(query));
                    return res;
                } else if(column_name.compare("s_quantity") == 0) {
                    response::GetStock res("tiny_tpc_c", "stock", {"s_quantity", "s_data", "s_dist_01", "s_dist_02", "s_dist_03", "s_dist_04", "s_dist_05", "s_dist_06", "s_dist_07", "s_dist_08", "s_dist_09", "s_dist_10"}, column_count);
                    res.addRow(sqlite3_column_int(stmt, 0), std::string(sqlite3_column_name(stmt, 1)), std::string(sqlite3_column_name(stmt, 2)), std::string(sqlite3_column_name(stmt, 3)), std::string(sqlite3_column_name(stmt, 4))
                    , std::string(sqlite3_column_name(stmt, 5)), std::string(sqlite3_column_name(stmt, 6)), std::string(sqlite3_column_name(stmt, 7)), std::string(sqlite3_column_name(stmt, 8)), std::string(sqlite3_column_name(stmt, 9)), std::string(sqlite3_column_name(stmt, 10))
                    ,std::string(sqlite3_column_name(stmt, 11)));
                    addQueryOnly(std::move(query));
                    return res;
                } else if(column_name.compare("field1") == 0) {
                    response::Result res("txbench", "bench", {"field1"}, column_count);
                    res.addRow(float(sqlite3_column_int(stmt, 0)));
                    addQueryOnly(std::move(query));
                    return res;
                }
            }
            fprintf(stderr, "----------------invalid select statement: %s\n", query.body().str().c_str());
            addQueryOnly(std::move(query));
            std::abort();
            }

        public:
            bool add(request::RequestQuery query) {
                std::cout << "body :" << query.body().str().c_str() << "\n";
                std::cout << "body length:" << query.body_length() << "\n";
                executeOnly(query);
                addQueryOnly(std::move(query));
            }

            void createTPCCDatabase() { // 決め打ちようにsqlite3にtpccのテーブルとデータを作成 // timestampはtext
                char *errMsg = nullptr;
                std::string create_district_table = "create table if not exists district(d_w_id INTEGER primary key, d_next_o_id INTEGER, d_tax REAL);";
                int rc = sqlite3_exec(_s3, create_district_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create district table result is " << rc << "\n";

                std::string create_item_table = "create table if not exists item(i_id INTEGER primary key, i_name TEXT, i_price REAL, i_data TEXT, i_im_id INTEGER);";
                rc = sqlite3_exec(_s3, create_item_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create item table result is " << rc << "\n";

                std::string create_customer_table = "create table if not exists customer(c_w_id INTEGER primary key, c_d_id INTEGER,c_id INGETER, c_discount REAL, c_credit TEXT, c_last TEXT,"
                "c_first TEXT, c_credit_lim REAL, c_balance REAL, c_ytd_payment REAL, c_payment_cnt INTEGER, c_delivery_cnt INTEGER, c_street_1 TEXT, c_street_2 TEXT,"
                "c_city TEXT, c_state TEXT, c_zip TEXT, c_phone TEXT, c_since TEXT, c_middle TEXT, c_data TEXT);";
                rc = sqlite3_exec(_s3, create_customer_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create customer table result is " << rc << "\n";

                std::string create_warehouse_table = "create table if not exists warehouse(w_id INTEGER primary key, w_ytd REAL, w_tax REAL, w_name TEXT, w_street_1 TEXT, w_street_2 TEXT, "
                "w_city TEXT, _state TEXT, w_zip TEXT);";
                rc = sqlite3_exec(_s3, create_warehouse_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create warehouse table result is " << rc << "\n";

                std::string create_stock_table = "create table if not exists stock(s_w_id INTEGER primary key, s_i_id INTEGER, s_quantity INTEGER, s_ytd REAL, s_order_cnt INTEGER, s_remote_cnt int,"
                "s_data TEXT, s_dist_01 TEXT, s_dist_02 TEXT, s_dist_03 TEXT, s_dist_04 TEXT, s_dist_05 TEXT, s_dist_06 TEXT, s_dist_07 TEXT, s_dist_08 TEXT, s_dist_09 TEXT, s_dist_10 TEXT);"; 
                rc = sqlite3_exec(_s3, create_stock_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create stock table result is " << rc << "\n";
                
                std::string create_oorder_table = "create table if not exists oorder(d_w_id INTEGER primary key, o_d_id INTEGER, o_id INTEGER, o_carrier_id INTEGER, o_ol_cnt REAL, o_all_local REAL, o_entry_d TEXT);"; 
                rc = sqlite3_exec(_s3, create_oorder_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create oorder table result is " << rc << "\n";
                
                std::string create_neworder_table = "create table if not exists new_order(no_w_id INTEGER primary key, no_d_id INTEGER, no_o_id INTEGER);"; // create table を自前で書いてる...!
                rc = sqlite3_exec(_s3, create_neworder_table.c_str(), nullptr, nullptr, &errMsg);
                // std::cout << "create new_order table result is " << rc << "\n";
                
                std::string create_table = "create table if not exists bench(pk TEXT primary key, field1 INTEGER, field2 INTEGER, field3 INTEGER);"; // create table を自前で書いてる...!
            }

            void loadTPCCData() {
                int  rc = sqlite3_exec(_s3, "INSERT INTO district(d_w_id , d_next_o_id, d_tax) VALUES (1 ,1, 1.1)", nullptr, nullptr, nullptr);
                // std::cout << "insert district result is " << rc << "\n";

                rc = sqlite3_exec(_s3, "INSERT INTO item(i_id ,i_name, i_price, i_data, i_im_id) VALUES (1,'item', 1.1, 'i_data', 1)", nullptr, nullptr, nullptr);
                // std::cout << "insert item result is " << rc << "\n";
                rc = sqlite3_exec(_s3, "INSERT INTO customer(c_w_id, c_d_id, c_id, c_discount, c_credit, c_last," 
                "c_first, c_credit_lim, c_balance, c_ytd_payment, c_payment_cnt, c_delivery_cnt, c_street_1, c_street_2," 
                "c_city, c_state, c_zip, c_phone, c_since, c_middle, c_data) VALUES (1, 1, 1, 1.1, 'cr', 'c_last', 'c_first', 1.1, 1.1)" 
                "1.1, 1, 1, 'street1', 'street2', 'c_city', 'aa', 'zip', 'phone', '1111-11-11 11:11:11.111', 'mid', 'c_data'"
                , nullptr, nullptr, nullptr);
                // std::cout << "insert customer is " << rc << "\n";

                rc = sqlite3_exec(_s3, "INSERT INTO warehouse(w_id, w_ytd, w_tax, w_name, w_street_1, w_street_2, "
                "w_city, _state, w_zip) VALUES (1, 1.1, 1.1, 'name', 'street1','street2', 'city','aa', 'zip')", nullptr, nullptr, nullptr);
                // std::cout << "insert warehouse is " << rc << "\n";
                rc = sqlite3_exec(_s3, "INSERT INTO stock(s_w_id, s_i_id, s_quantity, s_ytd, s_order_cnt, s_remote_cnt,"
                "s_dist_01, s_dist_02, s_dist_03, s_dist_04, s_dist_05, s_dist_06, s_dist_07, s_dist_08, s_dist_09, s_dist_10) VALUES (1, 1, 1, 1.1,"
                "1, 1, 'test','dist01','dist02','dist03','dist04','dist05','dist06','dist07','dist08','dist09','dist10',)", nullptr, nullptr, nullptr);
                // std::cout << "insert stock is " << rc << "\n";

                rc = sqlite3_exec(_s3, "INSERT INTO oorder(d_w_id, o_d_id, o_id, o_carrier_id, o_ol_cnt, o_all_local, o_entry_d) VALUES (1, 1 ,1, 1,"
                "1, 1.1, 1, '1111-11-11 11:11:11.111')", nullptr, nullptr, nullptr);
                // std::cout << "insert oorder is " << rc << "\n";
                rc = sqlite3_exec(_s3, "INSERT INTO new_order(no_w_id, no_d_id, no_o_id) VALUES (1 ,1, 1)", nullptr, nullptr, nullptr);
                // std::cout << "insert new_order result is " << rc << "\n";
            }
        
        public:
            std::vector<request::RequestQuery> moveAllQueries() { return std::move(_queries); }
        };
    } // namespace transaction
} // namespace proxy