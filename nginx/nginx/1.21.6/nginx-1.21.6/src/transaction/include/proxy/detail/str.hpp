//
// Created by cerussite on 2019/10/12.
//

#pragma once

#include <string>

namespace proxy {
    bool StartsWith(const std::string &target, const std::string &check);
    bool EndsWith(const std::string &target, const std::string &check);
} // namespace proxy
