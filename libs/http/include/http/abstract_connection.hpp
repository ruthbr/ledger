

#pragma once
//------------------------------------------------------------------------------
//
//   Copyright 2018 Fetch.AI Limited
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//------------------------------------------------------------------------------

#include "core/byte_array/byte_array.hpp"
#include "http/response.hpp"
#include "network/fetch_asio.hpp"

#include <memory>
#include <string>

namespace fetch {
namespace http {

class AbstractHTTPConnection
{
public:
  using shared_type = std::shared_ptr<AbstractHTTPConnection>;

  virtual ~AbstractHTTPConnection() {}
  virtual void        Send(HTTPResponse const &) = 0;
  virtual std::string Address()                  = 0;
};
}  // namespace http
}  // namespace fetch
