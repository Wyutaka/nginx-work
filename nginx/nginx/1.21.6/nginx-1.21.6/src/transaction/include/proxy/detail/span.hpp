//
// Created by miwa on 2019/11/05.
//

#ifndef TRANSACTION_SPAN_HPP
#define TRANSACTION_SPAN_HPP

#include <cstddef>

namespace proxy {
    template <class ElementType> class Span {
    private:
        ElementType *_elements;
        size_t _len;

    public:
        Span() = default;
        Span(const Span &) = default;
        Span(Span &&) = default;
        Span &operator=(const Span &) = default;
        Span &operator=(Span &&) = default;

    public:
        constexpr Span(ElementType *elements, size_t len)
            : _elements(elements)
            , _len(len) {}

    public:
        constexpr size_t size() const noexcept { return _len; }

        constexpr size_t size_bytes() const noexcept { return size() * sizeof(ElementType); }

        constexpr ElementType *data() const noexcept { return _elements; }

        constexpr ElementType *begin() const noexcept { return data(); }

        constexpr ElementType *end() const noexcept { return begin() + size(); }
    };
} // namespace proxy
#endif // TRANSACTION_SPAN_HPP
