//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <cstdint>

namespace txpx2 {

class Peer {
 private:
  int _1, _2;

 public:
  Peer() = default;
  Peer(int d1, int d2) noexcept : _1(d1), _2(d2) {}

 public:
  int first() const noexcept { return _1; }
  int second() const noexcept { return _2; }

 public:
  bool operator==(const Peer& peer) const noexcept {
    return _1 == peer._1 && _2 == peer._2;
  }

  bool operator!=(const Peer& peer) const noexcept {
    return !((*this) == peer);
  }
};

struct PeerHash {
  std::hash<int> hf;

  std::size_t operator()(const Peer& p) const {
    return hf(p.first()) ^ hf(p.second());
  }
};

}  // namespace txpx2
