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

namespace fetch {
namespace vm_modules {

/**
 * method for type converting from arithmetic to string
 */
template <typename T>
fetch::math::meta::IfIsArithmetic<T, fetch::vm::Ptr<fetch::vm::String>> ToString(fetch::vm::VM *vm,
                                                                                 T const &      a)
{
  if (std::is_same<T, bool>::value)
  {
    fetch::vm::Ptr<fetch::vm::String> ret(new fetch::vm::String(vm, a ? "True" : "False"));
    return ret;
  }
  else
  {
    fetch::vm::Ptr<fetch::vm::String> ret(new fetch::vm::String(vm, std::to_string(a)));
    return ret;
  }
}

template <typename T>
bool ToBool(fetch::vm::VM * /*vm*/, T const &a)
{
  return static_cast<bool>(a);
}

inline void CreateToString(fetch::vm::Module &module)
{
  module.CreateFreeFunction("toString", &ToString<int32_t>);
  module.CreateFreeFunction("toString", &ToString<uint32_t>);
  module.CreateFreeFunction("toString", &ToString<int64_t>);
  module.CreateFreeFunction("toString", &ToString<uint64_t>);
  module.CreateFreeFunction("toString", &ToString<float_t>);
  module.CreateFreeFunction("toString", &ToString<double_t>);
  module.CreateFreeFunction("toString", &ToString<bool>);
}

inline void CreateToString(std::shared_ptr<vm::Module> module)
{
  CreateToString(*module.get());
}

inline void CreateToBool(fetch::vm::Module &module)
{

  module.CreateFreeFunction("toBool", &ToBool<int32_t>);
  module.CreateFreeFunction("toBool", &ToBool<uint32_t>);
  module.CreateFreeFunction("toBool", &ToBool<int64_t>);
  module.CreateFreeFunction("toBool", &ToBool<uint64_t>);
  module.CreateFreeFunction("toBool", &ToBool<float_t>);
  module.CreateFreeFunction("toBool", &ToBool<double_t>);
}

}  // namespace vm_modules
}  // namespace fetch
