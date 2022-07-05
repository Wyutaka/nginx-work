//
// Created by cerussite on 3/2/20.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <txpx2/peer.hpp>
#include <txpx2/query.hpp>
#include <txpx2/response_context.hpp>
#include <txpx2/types.hpp>

#include <cpp17/span.hpp>

namespace txpx2 {

/// call multiple function at once.
/// \tparam Args function argument types
template <class... Args>
class FunctionSet {
 private:
  std::vector<std::function<void(Args...)>> functions_;

 public:
  /// add function
  /// \tparam Function function type
  /// \param func function to add
  template <class Function>
  void add(Function&& func) {
    functions_.emplace_back(std::forward<Function>(func));
  }

 public:
  /// call functions
  /// \param args arguments
  void operator()(Args... args) const {
    for (const auto& function : functions_) {
      function(args...);
    }
  }
};

namespace detail {

/// Peer and key's value
/// internal use in class Hook only
class PeerKey {
 private:
  Peer peer_;
  Value value_;

 public:
  /// construct from peer and key's value
  /// \param peer client
  /// \param value key's value
  PeerKey(Peer peer, Value value)
      : peer_(std::move(peer)), value_(std::move(value)) {}
  PeerKey(const PeerKey&) = default;
  PeerKey(PeerKey&&) = default;

  PeerKey& operator=(const PeerKey&) = default;
  PeerKey& operator=(PeerKey&&) = default;
  ~PeerKey() = default;

 public:
  /// get client info
  /// \return client info
  const Peer& peer() const noexcept { return peer_; }

  /// get key's value
  /// \return key's value
  const Value& value() const noexcept { return value_; }
};

struct PeerKeyHash {
  PeerHash ph;
  ValueHash vh;

  std::size_t operator()(const PeerKey& pk) const {
    return ph(pk.peer()) ^ vh(pk.value());
  }
};

}  // namespace detail

class Hook {
 private:
  std::unordered_map<std::uint_fast64_t, FunctionSet<ResponseContext&>>
      functions_;

 private:
  static std::uint_fast64_t CreateKey(const Peer& peer,
                                      std::uint_fast64_t stream) noexcept {
    return ((stream << 32) | peer.first()) << 16 ^ peer.second();
  }

  /// add function to function set for peer and key
  /// \tparam Function function type
  /// \param pk peer and key's value
  /// \param func function
  template <class Function>
  void add(const Peer& peer, Short stream, Function&& func) {
    functions_[CreateKey(peer, stream)].add(std::forward<Function>(func));
  }

  /// call and remove functions in peer and key
  /// \param pk peer and key's value
  void callAndRemove(const Peer& peer, Short stream, ResponseContext& rc) {
    auto itr = functions_.find(CreateKey(peer, stream));
    if (itr == std::end(functions_)) {
      return;
    }
    itr->second(rc);
    functions_.erase(itr);
  }

 private:
  /// singleton support
  /// \return instance
  static Hook& GetInstance() {
    static Hook hook;
    return hook;
  }

 public:
  /// add function front end
  /// \tparam Function function type
  /// \param peer client
  /// \param value key's value
  /// \param func function to add
  template <class Function>
  static void Add(const Peer& peer, Short stream, Function&& func) {
    GetInstance().add(peer, stream, std::forward<Function>(func));
  }

  /// call and remove functions in peer and key
  /// \param peer client
  /// \param value key's value
  static void CallAndRemove(const Peer& peer, Short stream,
                            ResponseContext& rc) {
    GetInstance().callAndRemove(peer, stream, rc);
  }
};

}  // namespace txpx2
