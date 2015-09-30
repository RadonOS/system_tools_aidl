/*
 * Copyright (C) 2015, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "type_namespace.h"

#include <iostream>
#include <string>

#include <base/stringprintf.h>

#include "aidl_language.h"
#include "parse_helpers.h"

using android::base::StringPrintf;
using std::cerr;
using std::endl;
using std::string;

namespace android {
namespace aidl {

bool TypeNamespace::HasType(const string& type_name) const {
  return GetValidatableType(type_name) != nullptr;
}

bool TypeNamespace::IsValidReturnType(const AidlType& raw_type,
                                      const string& filename) const {
  const string error_prefix = StringPrintf(
      "In file %s line %d return type %s%s:\n    ",
      filename.c_str(), raw_type.type.lineno, raw_type.type.data,
      raw_type.Brackets().c_str());

  const ValidatableType* return_type = GetValidatableType(raw_type.type.data);
  if (return_type == nullptr) {
    cerr << error_prefix << "unknown return type" << endl;
    return false;
  }

  if (!return_type->CanWriteToParcel()) {
    cerr << error_prefix << "return type cannot be marshalled" << endl;
    return false;
  }

  if (raw_type.dimension > 0 && !return_type->CanBeArray()) {
    cerr << error_prefix << "return type cannot be an array" << endl;
    return false;
  }

  if (raw_type.dimension > 1) {
    cerr << error_prefix << "only one dimensional arrays are supported" << endl;
    return false;
  }
  return true;
}

bool TypeNamespace::IsValidArg(const AidlArgument& a,
                               int arg_index,
                               const string& filename) const {
  string error_prefix = StringPrintf(
      "In file %s line %d parameter %s (%d):\n    ",
      filename.c_str(), a.GetLine(), a.GetName().c_str(), arg_index);

  // check the arg type
  const ValidatableType* t = GetValidatableType(a.GetType().type.data);
  if (t == nullptr) {
    cerr << error_prefix << "unknown type " << a.GetType().type.data << endl;
    return false;
  }

  if (!t->CanWriteToParcel()) {
    cerr << error_prefix
         << StringPrintf("'%s %s' can't be marshalled.",
                         a.GetType().type.data, a.GetName().c_str()) << endl;
    return false;
  }

  if (!a.DirectionWasSpecified() &&
      (a.GetType().dimension != 0 || t->CanBeOutParameter())) {
    cerr << error_prefix << StringPrintf(
        "'%s %s' can be an out parameter, so you must declare it as in,"
        " out or inout.", a.GetType().type.data, a.GetName().c_str()) << endl;
    return false;
  }

  if (a.GetDirection() != AidlArgument::IN_DIR &&
      !t->CanBeOutParameter() &&
      a.GetType().dimension == 0) {
    cerr << error_prefix << StringPrintf(
        "'%s' can only be an in parameter.",
        a.ToString().c_str()) << endl;
    return false;
  }

  if (a.GetType().dimension > 0 && !t->CanBeArray()) {
    cerr << error_prefix << StringPrintf(
        "'%s' cannot be an array.",
        a.ToString().c_str()) << endl;
    return false;
  }

  if (a.GetType().dimension > 1) {
    cerr << error_prefix << "Only one dimensional arrays are supported."
         << endl;
    return false;
  }

  // check that the name doesn't match a keyword
  if (is_java_keyword(a.GetName().c_str())) {
    cerr << error_prefix << "Argument name is a C++, Java, or aidl keyword"
         << endl;
    return false;
  }

  // Reserve a namespace for internal use
  if (a.GetName().substr(0, 5)  == "_aidl") {
    cerr << error_prefix << "Argument name cannot begin with '_aidl'"
         << endl;
    return false;
  }

  return true;
}

}  // namespace aidl
}  // namespace android
