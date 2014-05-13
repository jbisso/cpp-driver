/*
  Copyright 2014 DataStax

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef __CASS_BATCH_STATEMENT_HPP_INCLUDED__
#define __CASS_BATCH_STATEMENT_HPP_INCLUDED__

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "common.hpp"
#include "message_body.hpp"
#include "serialization.hpp"
#include "statement.hpp"

#define CASS_QUERY_FLAG_VALUES             0x01
#define CASS_QUERY_FLAG_SKIP_METADATA      0x02
#define CASS_QUERY_FLAG_PAGE_SIZE          0x04
#define CASS_QUERY_FLAG_PAGING_STATE       0x08
#define CASS_QUERY_FLAG_SERIAL_CONSISTENCY 0x10

namespace cass {

struct BatchStatement
    : public MessageBody {
  typedef std::list<Statement*> StatementCollection;

  uint8_t             type;
  StatementCollection statements;
  int16_t             consistency;

  explicit BatchStatement(size_t consistency)
    : MessageBody(CQL_OPCODE_BATCH)
    ,  consistency(consistency) {}

  ~BatchStatement() {
    for (Statement* statement : statements) {
      delete statement;
    }
  }

  void
  add_statement(
      Statement* statement) {
    statements.push_back(statement);
  }

  bool
  consume(
      char*  buffer,
      size_t size) {
    (void) buffer;
    (void) size;
    return true;
  }

  bool
  prepare(
      size_t reserved,
      char** output,
      size_t& size) {
    // reserved + type + length
    size = reserved + sizeof(uint8_t) + sizeof(uint16_t);

    for (const Statement* statement : statements) {
      size += sizeof(uint8_t);
      if (statement->kind() == 0) {
        size += sizeof(int32_t);
      } else {
        size += sizeof(int16_t);
      }
      size += statement->statement_size();

      for (const auto& value : *statement) {
        size += sizeof(int32_t);
        size += value.get_size();
      }
    }
    size    += sizeof(int16_t);
    *output  = new char[size];

    char* buffer = encode_byte(*output + reserved, type);
    buffer = encode_short(buffer, statements.size());

    for (const Statement* statement : statements) {
      buffer = encode_byte(buffer, statement->kind());

      if (statement->kind() == 0) {
        buffer = encode_long_string(
            buffer,
            statement->statement(),
            statement->statement_size());
      } else {
        buffer = encode_string(
            buffer,
            statement->statement(),
            statement->statement_size());
      }

      buffer = encode_short(buffer, statement->size());
      for (const auto& value : *statement) {
        buffer = value.encode(buffer);
      }
    }
    encode_short(buffer, consistency);
    return true;
  }
};

} // namespace cass

#endif