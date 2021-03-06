/*
 *      Copyright (C) 2013 DataStax Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include <boost/format.hpp>

#include "cql/internal/cql_util.hpp"
#include "cql/exceptions/cql_invalid_type_exception.hpp"

using namespace boost;

std::string
cql::cql_invalid_type_exception::create_message(
        const std::string& param_name,
        const std::string& expected_type,
        const std::string& received_type) const 
{
    return str(format(
        "Received object of type: %1%, expected type: %2%. " 
        "(Parameter name that caused exception: %s)")
			% received_type
			% expected_type
			% param_name);
}
