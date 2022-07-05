//
// Created by cerussite on 3/8/20.
//

#pragma once

#include <vector>

#include "header.hpp"
#include "peer.hpp"
#include "types.hpp"

namespace txpx2 {

class StreamContext {
 public:
  enum class Phase {
    kHeaderReading,
    kHeaderWasRead,
    kBodyReading,
    kBodyWasRead,
    kProcessing,
    kGarbage,
  };

 private:
  int socket_, core_;
  Header header_;
  Phase phase_;
  std::vector<Byte> body_;

 public:
  StreamContext(int socket, int core, Header header, Phase phase,
                std::vector<Byte> body)
      : socket_(socket),
        core_(core),
        header_(header),
        phase_(phase),
        body_(std::move(body)) {}

  StreamContext(int socket, int core)
      : StreamContext(socket, core, {}, Phase::kHeaderReading, {}) {}

  StreamContext(const StreamContext&) = default;
  StreamContext(StreamContext&&) = default;

  StreamContext& operator=(const StreamContext&) = default;
  StreamContext& operator=(StreamContext&&) = default;

 public:
  Header& header() noexcept { return header_; }
  const Header& header() const noexcept { return header_; }

 public:
  Phase phase() const noexcept { return phase_; }
  void phase(Phase phase) noexcept { phase_ = phase; }

 public:
  std::vector<Byte>& body() noexcept { return body_; }
  const std::vector<Byte>& body() const noexcept { return body_; }

 public:
  int socket() const noexcept { return socket_; }
  int core() const noexcept { return core_; }

  Peer peer() const noexcept { return Peer{core(), socket()}; }

 public:
  StreamContext createResponse(OpCode opc, std::vector<Byte> body) const
      noexcept {
    if (header().isResponse()) {
      return *this;
    }
    auto res_header = header().createResponse(opc, body.size());

    return StreamContext(socket(), core(), res_header, phase_, std::move(body));
  }
};

}  // namespace txpx2
