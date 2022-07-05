//
// Created by cerussite on 2019/10/13.
//
#include <cpp17/string_view.hpp>
#include <proxy/detail/str.hpp>

namespace proxy {
    bool StartsWith(const std::string &target, const std::string &check) {
        return target.find(check) == 0;
    }

    //bool StartsWith(const cpp17::string_view &target, const std::string &check) {
    //    return target.find(check) == 0;
    //}

    bool EndsWith(const std::string &target, const std::string &check) {
        return target.rfind(check) == (target.size() - check.size());
    }

    //bool EndsWith(const cpp17::string_view &target, const std::string &check) {
    //    return target.rfind(check) == (target.size() - check.size());
    //}
} // namespace proxy
