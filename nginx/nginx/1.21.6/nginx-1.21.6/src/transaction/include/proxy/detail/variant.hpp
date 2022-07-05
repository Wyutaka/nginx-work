//
// Created by miwa on 2019/12/26.
//

#ifndef TRANSACTION_VARIANT_HPP
#define TRANSACTION_VARIANT_HPP

namespace proxy {
    namespace detail {
        template <class T1, class T2> class Variant {
            union Union {
                T1 sigle;
                T2 multi;
            };
        private:
            union Union _query;

        public:
            Variant() = default;
            Variant(const Variant&) = default;
            Variant(Variant &&) = default;
            Variant &operator=(const Variant&) = default;
            Variant &operator=(Variant &&) = default;

        public:
            Variant(){}
        };
    } // namespace detail
} // namespace proxy
#endif // TRANSACTION_VARIANT_HPP
