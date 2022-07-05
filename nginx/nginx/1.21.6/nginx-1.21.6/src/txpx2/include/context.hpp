//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <txpx2/parser.hpp>
#include <txpx2/peer.hpp>
#include <txpx2/query.hpp>

// #include <boost/utility/string_view.hpp>
#include <cpp17/string_view.hpp>
#include <cpp17/span.hpp>

#include "error.hpp"
#include "stream_context.hpp"

namespace txpx2 {

namespace detail {

template <class Integral>
Integral ToIntegral(const std::vector<txpx2::Byte>& bytes) {
  return net::ByteSwap(*reinterpret_cast<const Integral*>(bytes.data()));
}

}  // namespace detail

namespace customize {

void SendResponse(Header header, cpp17::span<Byte> body);

}  // namespace customize

enum class ResultType : Int {
  kVoid = 0x0001,
  kRows = 0x0002,
  kSetKeyspace = 0x0003,
  kPrepared = 0x0004,
  kSchemaChange = 0x0005,
};

enum class ErrorCode : Int {
  kServerError = 0x0000,
  kProtocolError = 0x000a,
  kAuthenticationError = 0x0100,
  kUnavailableException = 0x1000,
  kOverloaded = 0x1001,
  kIsBootstrapping = 0x1002,
  kTruncateError = 0x1003,
  kWriteTimeout = 0x1100,
  kReadTimeout = 0x1200,
  kReadFailure = 0x1300,
  kFunctionFailure = 0x1400,
  kWriteFailure = 0x1500,
  kSyntaxError = 0x2000,
  kUnauthorized = 0x2100,
  kInvalid = 0x2200,
  kConfigError = 0x2300,
  kAlreadyExists = 0x2400,
  kUnprepared = 0x2500,
};

/// request information
class Context {
 private:
  StreamContext& stream_context_;
  cpp17::string_view original_query_;
  Query query_;

 private:
  explicit Context(StreamContext& stream_context,
                   cpp17::string_view original_query)
      : stream_context_(stream_context),
        original_query_(original_query),
        query_(ParseCql(original_query)) {}

 public:
  static Context Make(StreamContext& stream_context) {
    auto length = detail::ToIntegral<Int>(stream_context.body());
    return Context{stream_context,
                   cpp17::string_view(reinterpret_cast<const char*>(
                                          stream_context.body().data()) +
                                          sizeof(length),
                                      length)};
  }

  Context(const Context&) = default;
  Context(Context&&) = default;

  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  ~Context() = default;

 public:
  /// get query
  /// \return query
  const Query& query() const noexcept { return query_; }

  /// get client info
  /// \return client info
  Peer peer() const noexcept { return stream_context_.peer(); }

  /// get original query string
  /// \return query string
  const cpp17::string_view& original_query() const noexcept {
    return original_query_;
  }

  /// get stream ID
  /// \return stream ID
  Short stream() const noexcept { return stream_context_.header().stream(); }

  void finish(OpCode op_code, cpp17::span<Byte> response_body) const {
    auto header = stream_context_.header().createResponse(
        op_code, response_body.size_bytes());
    customize::SendResponse(header, response_body);
  }

  void finish(ResultType res) const {
    auto r = static_cast<Int>(res);
    r = htonl(r);
    finish(OpCode::kResult, {reinterpret_cast<const Byte*>(&r), sizeof(r)});
  }

  void abort(ErrorCode error_code) const {
    switch (error_code) {
      case ErrorCode::kProtocolError:
        finish(OpCode::kError, Error::kProtocolError);
        break;
      case ErrorCode::kSyntaxError:
        finish(OpCode::kError, Error::kSyntaxError);
        break;
      case ErrorCode::kAuthenticationError:
        finish(OpCode::kError, Error::kAuthenticationError);
        break;
      case ErrorCode::kOverloaded:
        finish(OpCode::kError, Error::kOverloaded);
        break;
      case ErrorCode::kIsBootstrapping:
        finish(OpCode::kError, Error::kIsBootstrapping);
        break;
      case ErrorCode::kTruncateError:
        finish(OpCode::kError, Error::kTruncateError);
        break;
      case ErrorCode::kUnauthorized:
        finish(OpCode::kError, Error::kUnauthorized);
        break;
      case ErrorCode::kInvalid:
        finish(OpCode::kError, Error::kInvalid);
        break;
      case ErrorCode::kConfigError:
        finish(OpCode::kError, Error::kConfigError);
        break;
      case ErrorCode::kUnprepared:
        finish(OpCode::kError, Error::kUnprepared);
        break;

      case ErrorCode::kUnavailableException:
      case ErrorCode::kWriteTimeout:
      case ErrorCode::kReadTimeout:
      case ErrorCode::kReadFailure:
      case ErrorCode::kFunctionFailure:
      case ErrorCode::kWriteFailure:
      case ErrorCode::kAlreadyExists:
        std::cerr << "not supported error code" << std::endl;
      case ErrorCode::kServerError:
        finish(OpCode::kError, Error::kServerError);
        break;
    }
  }
  StreamContext& internalContext() const noexcept { return stream_context_; }
};

}  // namespace txpx2
