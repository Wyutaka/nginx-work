//
// Created by miwa on 2020/02/25.
//
#include <iostream>
#include <parse.h>
#include "./regex_spirit_xpressive_benchmark/types.hpp"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <parse_table.h>
#include <string>

namespace qi = boost::spirit::qi;

namespace rb {

namespace spirit_internal {

template <class Iterator>
struct Cql : qi::grammar<Iterator, Query(), qi::space_type> {
  template <class Data>
  using RuleNoSpace = qi::rule<Iterator, Data>;

  template <class Data>
  using Rule = qi::rule<Iterator, Data, qi::space_type>;

  RuleNoSpace<Id()> id;
  Rule<Table()> table;
  Rule<IdList()> id_list;

  Rule<Number()> number;
  Rule<String()> string_content;
  Rule<String()> string;
  Rule<Value()> value;
  Rule<ValueList()> value_list;
  Rule<KeyValue()> key_value;
  Rule<KeyValueList()> key_value_list;

  Rule<Insert()> insert;
  Rule<Update()> update;
  Rule<Select()> select;

  Rule<Query()> query;

  struct Appender {
    void operator()(std::string& lhs, char ch) const {
      lhs += ch;
    }
    void operator()(std::string& lhs, const boost::optional<char>& ch) const {
      if (ch) {
        (*this)(lhs, *ch);
      }
    }

    void operator()(std::string& lhs, const std::string& rhs) const {
      lhs += rhs;
    }
    void operator()(std::string& lhs, const boost::optional<std::string>& ch) const {
      if (ch) {
        (*this)(lhs, *ch);
      }
    }
  };

  Cql() : Cql::base_type(query) {
    using boost::phoenix::bind;
    using boost::phoenix::push_back;
    using qi::_1;
    using qi::_val;

    id = +((qi::char_('a', 'z') | qi::char_('A', 'Z') | qi::char_('0', '9') | qi::char_('_'))[_val += _1]);
    id_list = id[push_back(_val, _1)] >> *(',' >> id[push_back(_val, _1)]);

    table = ((id[bind(&Table::keyspace, _val) = _1] >> '.' >> id[bind(&Table::table, _val) = _1]) | id[bind(&Table::table, _val) = _1]);

    number = qi::double_[_val = _1];

    string_content = *(qi::alnum | qi::char_('_') | qi::lit("''"))[bind(Appender(), _val, _1)];
    string = qi::char_('\'')[_val += _1] >> string_content[_val += _1] >> qi::char_('\'')[_val += _1];

    value = (number | string)[_val = _1];
    value_list = value[push_back(_val, _1)] >> *(',' >> value[push_back(_val, _1)]);

    key_value = (id[bind(&KeyValue::key, _val) = _1] >> '=' >> value[bind(&KeyValue::value, _val) = _1]);
    key_value_list = (key_value[push_back(_val, _1)] >> *(',' >> key_value[push_back(_val, _1)]));

    insert = qi::no_case["INSERT"] >> qi::no_case["INTO"] >> table[bind(&Insert::table, _val) = _1] >> '(' >> id_list[bind(&Insert::columns, _val) = _1] >> ')' >> qi::no_case["VALUES"] >> '(' >> value_list[bind(&Insert::data, _val) = _1] >> ')';
    update = qi::no_case["UPDATE"] >> table[bind(&Update::table, _val) = _1] >> qi::no_case["SET"] >> key_value_list[bind(&Update::kvs, _val) = _1] >> qi::no_case["WHERE"] >> key_value[bind(&Update::where, _val) = _1];
    select = qi::no_case["SELECT"] >> ('*' | id_list[bind(&Select::columns, _val) = _1]) >> qi::no_case["FROM"] >> table[bind(&Select::table, _val) = _1] >> -(qi::no_case["WHERE"] >> key_value[bind(&Select::where, _val) = _1]);

    query = (update | select | insert)[_val = _1];
  }
};

}  // namespace spirit_internal

}





namespace {

class TableTypeVisitor {
public:
  TableType _getType(const rb::Table& tbl) const {
    if(tbl.table == "peers") {
      return kPeersTable;
    }
    if(tbl.table == "local") {
      return kLocalTable;
    }
    //if(tbl.table == "keyspaces") {
    //  return VksTable;
    //}
    //if(tbl.table == "tables") {
    //  return VtbTable;
    //}
    //if(tbl.table == "columns") {
    //  return VclTable;
    //}
    return  kOtherTable;
  }

  TableType operator()(const rb::Insert& ins)const {
    return _getType(ins.table);
  }

  TableType operator()(const rb::Select& sel) const{
    return _getType(sel.table);
  }
  TableType operator()(const rb::Update& upd) const{
    return _getType(upd.table);
  }
};

} // namespace

extern "C"
TableType parse_table(unsigned char* query_string, size_t length) {
  using CqlParser = rb::spirit_internal::Cql<unsigned char*>;

  //std::cout << str << std::endl;
  CqlParser parser;
  auto itr = query_string;

  rb::Query query;
  qi::phrase_parse(itr, query_string + length, parser, qi::space, query);

  return boost::apply_visitor(TableTypeVisitor(), query);
}

