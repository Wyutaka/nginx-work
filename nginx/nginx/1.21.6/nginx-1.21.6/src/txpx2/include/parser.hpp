//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <string>

#include <txpx2/query.hpp>

#include <cpp17/string_view.hpp>
//#include <boost/utility/string_view.hpp>

namespace txpx2 {

// Query ParseCql(boost::string_view str);
Query ParseCql(cpp17::string_view str);

}  // namespace txpx2
