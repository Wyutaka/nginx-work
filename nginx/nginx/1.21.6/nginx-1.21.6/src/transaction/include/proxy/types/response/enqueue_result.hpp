//
// Created by miwa on 2019/10/25.
//

#ifndef TRANSACTION_ENQ_RESULT_HPP
#define TRANSACTION_ENQ_RESULT_HPP

#include "response_data.hpp"

extern "C" int print_cql(char *, int, proxy::Aggregator*);

namespace proxy {
    inline void EnqueueResult(const response::ResponseData &response, Aggr aggr) {
        //for (const auto &response : responses) {
            //print_cql(response.get().data(), response.get().size_bytes());
        //}
        auto data = response.get().data();
        // DEBUG::debug(data);
        // std::cout << "-------------\n\n\n\n result data is " <<  data << "\n\n";
        auto size= static_cast<int>(response.get().size_bytes());
        fprintf(stderr, "response size is: %d\n", size);
        fprintf(stderr, "response data is: %d\n", size); 
        for (int i = 0; i  < size; i++) {
            fprintf(stderr, "0x%x", data[i]); 
        }
        fprintf(stderr, "\n"); 
        auto aggre = aggr.get_aggr();
        print_cql(data, size, aggre);
    }
} // namespace proxy

#endif // TRANSACTION_ENQ_RESULT_HPP
