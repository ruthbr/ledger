#pragma once
//------------------------------------------------------------------------------
//
//   Copyright 2018-2019 Fetch.AI Limited
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

#include "vm/module.hpp"

#include <cmath>

namespace fetch {
namespace vm_modules {

inline double Exp(fetch::vm::VM * /*vm*/, double x)
{
  return std::exp(x);
}

inline void BindExp(vm::Module &module)
{
  module.CreateFreeFunction("exp", &Exp);
}

}  // namespace vm_modules
}  // namespace fetch