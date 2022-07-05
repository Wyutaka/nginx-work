//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <mutex>

#include <txpx2/context.hpp>
#include <txpx2/dpdk_hash_map.hpp>
#include <txpx2/layer.hpp>
#include <txpx2/peer.hpp>

namespace txpx2 {

class Lock : public Layer {
 private:
  struct Key {
    char key[64];
  };

 private:
  DpdkHashMap<Key, Peer> locked_table_;
  DpdkHashSet<Peer> began_peers_table_;

 public:
  explicit Lock(std::unique_ptr<Layer> next)
      : Layer(std::move(next)),
        locked_table_("LockedTable"),
        began_peers_table_("BeganPeersTable") {}

  static std::unique_ptr<Layer> Make(std::unique_ptr<Layer> next) {
    return std::make_unique<Lock>(std::move(next));
  }

 private:
  /// create internal hash table key
  /// \param key output key value
  /// \param table table name
  /// \param value value
  static void createTableKey(Key& key, const Id& table, const Value& value);

 private:
  /// get lock for specified row to peer
  /// \param peer client
  /// \param table table name
  /// \param value primary key value
  /// \return true => locked, false => cannot lock
  bool doLock(const Peer& peer, const Id& table, const Value& value);

  /// unlock the locks for specified peer
  /// \param peer client
  bool doUnLock(const Peer& peer);

  /// check getting lock by other client
  /// \param peer client
  /// \param table table name
  /// \param value primary key value
  /// \return true => locked, false => no one locked
  bool isLockedByOther(const Peer& peer, const Id& table,
                       const Value& value) const;

  /// check getting lock by other client (all rows)
  /// \param peer client
  /// \param table table name
  /// \return true => locked, false => no one locked
  bool isLockedByOther(const Peer& peer, const Id& table) const;

  /// get lock if no one locked
  /// \param peer client
  /// \param table table name
  /// \param value primary key value
  /// \return true => locked, false => cannot lock
  bool doLockIfNotLockedByOther(const Peer& peer, Short stream, const Id& table,
                                const Value& value);

 private:
  /// check transaction started by peer
  /// \param peer client
  /// \return true => started (BEGIN processed), false => not started
  bool isTransactionStarted(const Peer& peer) const;

  /// start transaction phase if peer is not transaction started
  /// \param peer client
  /// \return true => successfully, false => already started
  bool startTransactionIfNotStarted(const Peer& peer);

 private:
  /// do operation for select query
  /// \param context context
  /// \param res response
  /// \return operation finished successfully
  bool doSelect(const Context& context);

  /// do operation for insert query
  /// \param context context
  /// \param res response
  /// \return operation finished successfully
  bool doInsert(const Context& context);

  /// do operation for update query
  /// \param context context
  /// \param res response
  /// \return operation finished successfully
  bool doUpdate(const Context& context);

  /// do operation for begin query
  /// \param context context
  /// \param res response
  /// \return operation finished successfully
  bool doBegin(const Context& context);

  /// do operation for commit and rollback query
  /// \param context context
  /// \param res response
  /// \return operation finished successfully
  bool doCommitOrRollback(const Context& context);

 public:
  /// do lock operation
  /// \param req request
  /// \param res response
  void operator()(const Context& context) override;
};

}  // namespace txpx2
