//
// Created by cerussite on 2/20/20.
//

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <functional>

namespace txpx2 {

/// identifier
using Id = std::string;

/// keyspace name and table name
/// $keyspace.$table
struct Table {
 private:
  Id keyspace_;
  Id table_;

 public:
  /// get keyspace name
  /// \return keyspace name
  const Id& keyspace() const noexcept { return keyspace_; }

  /// get table name
  /// \return table name
  const Id& table() const noexcept { return table_; }

  /// set keyspace name
  /// \param ks keyspace name
  void set_keyspace(const Id& ks) { keyspace_ = ks; }

  /// set table name
  /// \param tbl table name
  void set_table(const Id& tbl) { table_ = tbl; }

  std::string to_query() const {
    if (keyspace().empty()) {
      return table();
    }
    return keyspace() + '.' + table();
  }
};

/// ID list (usually comma separated)
using IdList = std::vector<Id>;

/// number value
using Number = double;
/// string value with single quotation
using String = std::string;
/// number or string value
using Value = boost::variant<Number, String>;
/// value list (usually comma separated)
using ValueList = std::vector<Value>;

/// key and value (usually $key = $value format)
struct KeyValue {
 private:
  Id key_;
  Value value_;

 public:
  /// get key name
  /// \return key name
  const Id& key() const noexcept { return key_; }

  /// get value
  /// \return value
  const Value& value() const noexcept { return value_; }

  /// set key name
  /// \param key key name
  void set_key(const Id& key) { key_ = key; }

  /// set value
  /// \param value value
  void set_value(const Value& value) { value_ = value; }
};
/// key and value list (usually comma separated)
using KeyValueList = std::vector<KeyValue>;

/// insert query elements
/// INSERT INTO $table ($columns) VALUES ($values)
struct Insert {
 private:
  Table table_;
  IdList columns_;
  ValueList data_;
  bool if_not_exists_ = false;

 public:
  /// get table name
  /// \return table name
  const Table& table() const noexcept { return table_; }

  /// get column names
  /// \return column names
  const IdList& columns() const noexcept { return columns_; }

  /// get column name
  /// \param index column index
  /// \return column name
  const Id& column(std::size_t index) const noexcept {
    return columns()[index];
  }

  /// get values
  /// \return values
  const ValueList& values() const noexcept { return data_; }

  /// get value
  /// \param index value index
  /// \return value
  const Value& value(std::size_t index) const noexcept {
    return values()[index];
  }

  /// is light weight transaction
  /// \return status (query is LWT)
  bool is_lwt() const noexcept { return if_not_exists_; }

  /// set table name
  /// \param table table name
  void set_table(const Table& table) { table_ = table; }

  /// set column names
  /// \param columns column names
  void set_columns(const IdList& columns) { columns_ = columns; }

  /// set values
  /// \param values values
  void set_values(const ValueList& values) { data_ = values; }

  void set_as_if_not_exists(bool status) noexcept { if_not_exists_ = status; }
};

/// select query elements
/// SELECT $columns FROM $table WHERE $where
struct Select {
 private:
  Table table_;
  IdList columns_;
  boost::optional<KeyValue> where_;

 public:
  /// get table name
  /// \return table name
  const Table& table() const noexcept { return table_; }

  /// get columns name
  /// \return columns name (return empty list if '*' specified)
  const IdList& columns() const noexcept { return columns_; }

  /// get column name
  /// \param index column index
  /// \return column name
  const Id& column(std::size_t index) const noexcept {
    return columns()[index];
  }

  /// get where query
  /// \return where query (if no where query, return boost::none)
  const boost::optional<KeyValue>& where() const noexcept { return where_; }

  /// set table name
  /// \param table table name
  void set_table(const Table& table) { table_ = table; }

  /// set columns name
  /// \param columns columns name
  void set_columns(const IdList& columns) { columns_ = columns; }

  /// set where query
  /// \param where where query
  void set_where(const boost::optional<KeyValue>& where) { where_ = where; }
};

/// update query elements
/// UPDATE $table SET $key_values WHERE $where
struct Update {
 private:
  Table table_;
  KeyValueList kvs_;
  KeyValue where_;

 public:
  /// get table name
  /// \return table name
  const Table& table() const noexcept { return table_; }

  /// get key-values list
  /// \return key-values list
  const KeyValueList& key_values() const noexcept { return kvs_; }

  /// get where query
  /// \return where query
  const KeyValue& where() const noexcept { return where_; }

  /// set table name
  /// \param table table name
  void set_table(const Table& table) { table_ = table; }

  /// set key-values list
  /// \param kvs key-values list
  void set_key_values(const KeyValueList& kvs) { kvs_ = kvs; }

  /// set where query
  /// \param where where query
  void set_where(const KeyValue& where) { where_ = where; }
};

struct Begin {};
struct Commit {};
struct Rollback {};

/// query
/// SELECT, UPDATE, or INSERT
using Query = boost::variant<Select, Update, Insert, Begin, Commit, Rollback>;

constexpr int kSelectIndex = 0, kUpdateIndex = 1, kInsertIndex = 2,
              kBeginIndex = 3, kCommitIndex = 4, kRollbackIndex = 5;

struct ValueToStringVisitor {
  static ValueToStringVisitor& GetInstance() {
    static ValueToStringVisitor visitor;
    return visitor;
  }

  std::string operator()(Number number) const { return std::to_string(number); }
  std::string operator()(const String& s) const { return s; }
};

struct QueryPrint {
  void operator()(const Insert& insert) const {
    assert(insert.columns().size() == insert.values().size());

    std::cout << "insert" << std::endl;
    std::cout << "  table: " << insert.table().keyspace() << "."
              << insert.table().table() << std::endl;
    std::cout << "  data:" << std::endl;
    for (std::size_t i = 0; i < insert.columns().size(); ++i) {
      std::cout << "    " << insert.column(i) << "=" << insert.value(i)
                << std::endl;
    }
    std::cout << "  LWT: " << std::boolalpha << insert.is_lwt() << std::endl;
  }

  void operator()(const Select& select) const {
    std::cout << "select" << std::endl;
    std::cout << "  table: " << select.table().keyspace() << "."
              << select.table().table() << std::endl;
    std::cout << "  columns: ";
    if (select.columns().empty()) {
      std::cout << "*" << std::endl;
    } else {
      std::cout << "\n";
      for (const auto& column : select.columns()) {
        std::cout << "    " << column << std::endl;
      }
    }
    std::cout << "  where: ";
    if (select.where()) {
      std::cout << select.where()->key() << "=" << select.where()->value()
                << std::endl;
    } else {
      std::cout << "null" << std::endl;
    }
  }

  void operator()(const Update& update) const {
    std::cout << "update" << std::endl;
    std::cout << "  table: " << update.table().keyspace() << "."
              << update.table().table() << std::endl;
    std::cout << "  data: " << std::endl;
    for (const auto& kv : update.key_values()) {
      std::cout << "    " << kv.key() << "=" << kv.value() << std::endl;
    }

    std::cout << "  where: " << update.where().key() << "="
              << update.where().value() << std::endl;
  }

  void operator()(Begin) const { std::cout << "begin" << std::endl; }
  void operator()(Commit) const { std::cout << "commit" << std::endl; }
  void operator()(Rollback) const { std::cout << "rollback" << std::endl; }
};

struct ValueHash {
  struct {
    using result_type = std::size_t;
    ::std::hash<double> dh;
    ::std::hash<std::string> sh;

    std::size_t operator()(double d) const { return dh(d); }
    std::size_t operator()(const std::string& s) const { return sh(s); }
  } hi;

  std::size_t operator()(const Value& value) const {
    return boost::apply_visitor(hi, value);
  }
};

}  // namespace txpx2