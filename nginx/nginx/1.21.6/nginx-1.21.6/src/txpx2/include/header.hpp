//
// Created by cerussite on 3/9/20.
//

#pragma once

#include <cstdint>

#include <netinet/in.h>

#include "types.hpp"

namespace txpx2 {

using Int = std::int32_t;
using Long = std::int64_t;
using Short = std::int16_t;

enum class OpCode : std::uint8_t {
  kError = 0x00,
  kStartup = 0x01,
  kReady = 0x02,
  kAuthenticate = 0x03,
  kOptions = 0x05,
  kSupported = 0x06,
  kQuery = 0x07,
  kResult = 0x08,
  kPrepare = 0x09,
  kExecute = 0x0a,
  kRegister = 0x0b,
  kEvent = 0x0c,
  kBatch = 0x0d,
  kAuthChallenge = 0x0e,
  kAuthResponse = 0x0f,
  kAuthSuccess = 0x10,
};

namespace net {

constexpr Short ByteSwap(Short data) noexcept {
  auto u16 = static_cast<Short>(data);

  std::uint16_t ret = u16 << 8;
  ret |= u16 >> 8;
  return ret;
}

constexpr Int ByteSwap(Int data) noexcept {
  auto u32 = static_cast<Int>(data);

  uint32_t ret = u32 << 24;
  ret |= (u32 & 0x0000FF00) << 8;
  ret |= (u32 & 0x00FF0000) >> 8;
  ret |= u32 >> 24;
  return ret;
}

}  // namespace net

class NetworkByteOrder {};

class [[gnu::packed]] Header {
 public:
  enum Flags {
    kCompression = 0x01,
    kTracingFlag = 0x02,
    kCustomPayload = 0x04,
    kWarningFlag = 0x08,
  };
  static constexpr Byte kResponseVersion = static_cast<Byte>(0x84);
  static constexpr Byte kRequestVersion = static_cast<Byte>(0x04);

 private:
  Byte version_;
  Byte flags_;
  Short stream_;
  OpCode opcode_;
  Int length_;

 public:
  constexpr Header(NetworkByteOrder, Byte version, Flags flags, Short stream,
                   OpCode opcode, Int length) noexcept
      : version_(version),
        flags_(static_cast<Byte>(flags)),
        stream_(stream),
        opcode_(opcode),
        length_(length) {}

  constexpr Header(Byte version, Flags flags, Short stream, OpCode opcode,
                   Int length) noexcept
      : Header(NetworkByteOrder{}, version, flags, net::ByteSwap(stream),
               opcode, net::ByteSwap(length)) {}

  Header() = default;
  Header(const Header&) = default;
  Header(Header &&) = default;

  Header& operator=(const Header&) = default;
  Header& operator=(Header&&) = default;

  ~Header() = default;

 public:
  Byte version() const noexcept { return version_; }
  Flags flags() const noexcept { return static_cast<Flags>(flags_); }
  bool is(Flags flag) const noexcept { return (flags() & flag) != 0; }

  OpCode opcode() const noexcept { return opcode_; }

  Short stream() const noexcept { return ntohs(stream_); }
  Int length() const noexcept { return ntohl(length_); }

  void length(Int l) noexcept { length_ = htonl(l); }

 public:
  bool isRequest() const noexcept {
    return (static_cast<std::uint_fast8_t>(version()) & 0x80) == 0;
  }
  bool isResponse() const noexcept { return !isRequest(); }

 public:
  Header createResponse(OpCode opc, Int length) const noexcept {
    if (isRequest()) {
      return Header(kResponseVersion, Flags{}, stream(), opc, length);
    } else {
      return *this;
    }
  }
};

}  // namespace txpx2
