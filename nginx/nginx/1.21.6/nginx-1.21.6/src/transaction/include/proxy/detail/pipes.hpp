//
// Created by cerussite on 2019/10/12.
//

#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace proxy {
    template <class Container, class Function, template <class...> class RetContainer = std::vector>
    auto operator|(const Container &c, Function f) -> RetContainer<decltype(f(*std::begin(c)))> {
        RetContainer<decltype(f(*std::begin(c)))> ret(c.size());
        std::transform(std::begin(c), std::end(c), std::begin(ret), f);
        return ret;
    }

    template <class Function> std::string operator|(const std::string &c, Function f) {
        std::string ret(c.size(), 0);
        std::transform(std::begin(c), std::end(c), std::begin(ret), f);
        return ret;
    }

    template <class Function>
    cpp17::string_view operator|(const cpp17::string_view &c, Function f) {
        cpp17::string_view ret(c);
        std::transform(std::begin(c), std::end(c), std::begin(ret), f);
        return ret;
    }
} // namespace proxy
