//
// Created by cerussite on 3/4/20.
//

#pragma once

#include <txpx2/header.hpp>
#include <txpx2/types.hpp>

#include <cpp17/span.hpp>
#include <netinet/in.h>

namespace txpx2 {

class Result {
 private:
  cpp17::span<Byte> raw_response_;

 public:
  explicit Result(cpp17::span<Byte> raw_response)
      : raw_response_(raw_response) {}

 private:
  template <class IntegralT,
            class = std::enable_if_t<std::is_integral<IntegralT>::value>>
  static IntegralT GetNumber(cpp17::span<Byte> Bytes) {
    return net::ByteSwap(*reinterpret_cast<const IntegralT*>(Bytes.data()));
  }

  template <class LengthT,
            class = std::enable_if_t<std::is_integral<LengthT>::value>>
  static std::size_t GetStringOrBytesLength(cpp17::span<Byte> Bytes) {
    return sizeof(LengthT) + GetNumber<LengthT>(Bytes);
  }

  static std::size_t GetValueLength(cpp17::span<Byte> Bytes) {
    return GetStringOrBytesLength<Int>(Bytes);
  }
  static std::size_t GetOptionLength(cpp17::span<Byte> Bytes) {
    Bytes = Bytes.subspan<2>();
    return 2 + GetValueLength(Bytes);
  }

 public:
  std::size_t row_count() const noexcept {
    auto response = raw_response_.subspan<4>();

    auto flags = GetNumber<Int>(response);
    bool has_global_tables_spec = (flags & 0x0001) != 0;
    bool has_paging_state = (flags & 0x0002) != 0;
    bool has_metadata = (flags & 0x0004) == 0;
    response = response.subspan<4>();  // skip flags

    auto columns_count = GetNumber<Int>(response);
    response = response.subspan<4>();  // skip columns_count

    if (has_paging_state) {
      response = response.subspan(GetStringOrBytesLength<Short>(response));
    }

    if (has_metadata) {
      if (has_global_tables_spec) {
        // keyspace
        response = response.subspan(GetStringOrBytesLength<Short>(response));
        // table
        response = response.subspan(GetStringOrBytesLength<Short>(response));
      }

      for (std::size_t i = 0; i < columns_count; ++i) {
        if (!has_global_tables_spec) {
          // keyspace
          response = response.subspan(GetStringOrBytesLength<Short>(response));
          // table
          response = response.subspan(GetStringOrBytesLength<Short>(response));
        }

        // column name
        response = response.subspan(GetStringOrBytesLength<Short>(response));

        response = response.subspan(GetOptionLength(response));
      }
    }

    return GetNumber<Int>(response);
  }

  bool empty() const noexcept {
    switch (static_cast<std::uint_fast8_t>(raw_response_[3])) {
      case 0x01:  // Void
        return true;
      case 0x02:  // Rows
        return row_count() == 0;
      default:
        return false;
    }
  }
};

class ResponseContext {
 private:
  bool continue_response_ = true;
  cpp17::span<Byte> raw_response_;

 public:
  explicit ResponseContext(cpp17::span<Byte> raw_response)
      : raw_response_(raw_response) {}

 public:
  bool continueResponse() const noexcept { return continue_response_; }

  void abort() noexcept { continue_response_ = false; }

  const cpp17::span<Byte>& data() const noexcept { return raw_response_; }

  Result asResult() const noexcept { return Result{raw_response_}; }
};

}  // namespace txpx2
