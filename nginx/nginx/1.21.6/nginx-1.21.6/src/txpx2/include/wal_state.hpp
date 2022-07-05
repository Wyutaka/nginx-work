//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <txpx2/journal.hpp>

#include <sqlite3.h>

namespace txpx2 {
class WalState {
 public:
  static constexpr bool kAsyncCommit = false;

 private:
  sqlite3 *_s3;
  journal::MmapJournal _queries;

 public:
  /// construct from Mmap Pool
  /// \param pool mmap pool
  explicit WalState(std::shared_ptr<journal::MmapPool> pool);

  WalState(const WalState &) = delete;
  WalState(WalState &&rhs) noexcept;

  WalState &operator=(const WalState &) = delete;
  WalState &operator=(WalState &&rhs) noexcept;

  ~WalState();

 private:
  /// free all resources
  void reset() noexcept;

 public:
  /// execute on DB
  /// \param query query string
  /// \return true => execute on DB successfully, false => failed
  bool executeOnly(boost::string_view query) noexcept;

  /// add query string to Log area
  /// \param query query string
  void addQueryOnly(const boost::string_view &query);

 public:
  /// add and execute query
  /// \param query query string
  /// \return true => operation finished successfully, false => faild
  bool add(const boost::string_view &query);

 public:
  /// get all saved queries
  /// \return saved queries
  std::vector<boost::string_view> getAllQueries() const;
};
}  // namespace txpx2
