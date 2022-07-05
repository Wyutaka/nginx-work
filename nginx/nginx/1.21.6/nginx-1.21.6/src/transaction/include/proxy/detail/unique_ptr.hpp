//
// Created by miwa on 2019/12/09.
//

#ifndef TRANSACTION_UNIQUE_PTR_HPP
#define TRANSACTION_UNIQUE_PTR_HPP
#include <cstddef>
#include <type_traits>

namespace proxy {
    namespace detail {
        template <class T, class D> class unique_ptr {
        public:
            using pointer = T *;
            using element_type = T;
            using deleter_type = D;

        private:
            pointer _ptr;

        public:
            unique_ptr(pointer p) noexcept
                : _ptr(p) {}

            unique_ptr(std::nullptr_t) noexcept
                : _ptr(nullptr) {}

            unique_ptr() noexcept
                : unique_ptr(nullptr) {}

            unique_ptr(const unique_ptr &) = delete;
            unique_ptr(unique_ptr &&ptr) noexcept
                : _ptr(ptr._ptr) {
                ptr._ptr = nullptr;
            }
            unique_ptr &operator=(const unique_ptr &) = delete;
            unique_ptr &operator=(unique_ptr &&ptr) noexcept {
                reset(ptr._ptr);
                ptr._ptr = nullptr;
            }
            ~unique_ptr(){
                reset();
            }

        public:
            void reset(pointer p = nullptr) noexcept {
                if (_ptr) {
                    D{}(_ptr);
                }
                _ptr = p;
            }

            pointer get() const noexcept { return _ptr; }

            explicit operator bool() const noexcept { return _ptr != nullptr; }

            typename std::add_lvalue_reference<T>::type operator*() const { return *get(); }

            pointer operator->() const noexcept {
                return get();
            }

            element_type& operator[](std::size_t index) noexcept{
                return get()[index];
            }
            const element_type& operator[](std::size_t index)const noexcept {
                return get()[index];
            }
        };
    } // namespace detail
} // namespace proxy

#endif // TRANSACTION_UNIQUE_PTR_HPP
