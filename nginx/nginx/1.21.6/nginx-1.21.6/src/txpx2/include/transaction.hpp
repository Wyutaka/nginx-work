//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <unordered_map>

#include <txpx2/context.hpp>
#include <txpx2/journal.hpp>
#include <txpx2/layer.hpp>
#include <txpx2/peer.hpp>
#include <txpx2/query.hpp>
#include <txpx2/wal_state.hpp>

#include <sqlite3.h>

namespace txpx2 {

class Transaction : public Layer {
 private:
  std::shared_ptr<journal::MmapPool> _pool;
  std::unordered_map<Peer, WalState, PeerHash> _states;

 public:
  explicit Transaction(std::unique_ptr<Layer> next)
      : Layer(std::move(next)),
        _pool(journal::MmapPool::New("./data", 16_KiB, 10)) {}

  static std::unique_ptr<Layer> Make(std::unique_ptr<Layer> next) {
    return std::make_unique<Transaction>(std::move(next));
  }

 public:
  /// operate begin query
  /// \param context request
  /// \param res response
  void begin(const Context &context);

  /// operate commit query
  /// \param context request
  /// \param res response
  void commit(const Context &context);

  /// operate rollback query
  /// \param context request
  /// \param res response
  void rollback(const Context &context);

  /// operate insert with LWT query
  /// \param insert insert query info
  /// \param context request
  /// \param res resposne
  void insertIfNotExists(const Insert &insert, const Context &context);

  /// operate other queries (e.g. insert, update, or select)
  /// \param context request
  /// \param res response
  void query(const Context &context);

  /// entry point
  /// \param context request
  /// \param res response
  void operator()(const Context &context) override;
};

}  // namespace txpx2
