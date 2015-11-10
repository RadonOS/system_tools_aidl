/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <vector>

#include <binder/IServiceManager.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/StrongPointer.h>

#include "android/aidl/tests/ITestService.h"
#include "android/aidl/tests/INamedCallback.h"

// libutils:
using android::OK;
using android::sp;
using android::status_t;
using android::String16;
using android::String8;

// libbinder:
using android::getService;
using android::IBinder;

// generated
using android::aidl::tests::ITestService;
using android::aidl::tests::INamedCallback;

using std::cerr;
using std::cout;
using std::endl;
using std::vector;

namespace {

const char kServiceName[] = "android.aidl.tests.ITestService";

bool GetService(sp<ITestService>* service) {
  cout << "Retrieving test service binder" << endl;
  status_t status = getService(String16(kServiceName), service);
  if (status != OK) {
    cerr << "Failed to get service binder: '" << kServiceName
         << "' status=" << status << endl;
    return false;
  }
  return true;
}

template <typename T>
bool RepeatPrimitive(const sp<ITestService>& service,
                     status_t(ITestService::*func)(T, T*),
                     const T input) {
  T reply;
  status_t status = (*service.*func)(input, &reply);
  if (status != OK || input != reply) {
    cerr << "Failed to repeat primitive. status=" << status << "." << endl;
    return false;
  }
  return true;
}

bool ConfirmPrimitiveRepeat(const sp<ITestService>& s) {
  cout << "Confirming passing and returning primitives works." << endl;

  if (!RepeatPrimitive(s, &ITestService::RepeatBoolean, true) ||
      !RepeatPrimitive(s, &ITestService::RepeatByte, int8_t{-128}) ||
      !RepeatPrimitive(s, &ITestService::RepeatChar, char16_t{'A'}) ||
      !RepeatPrimitive(s, &ITestService::RepeatInt, int32_t{1 << 30}) ||
      !RepeatPrimitive(s, &ITestService::RepeatLong, int64_t{1ll << 60}) ||
      !RepeatPrimitive(s, &ITestService::RepeatFloat, float{1.0f/3.0f}) ||
      !RepeatPrimitive(s, &ITestService::RepeatDouble, double{1.0/3.0})) {
    return false;
  }

  vector<String16> inputs = {
      String16("Deliver us from evil."),
      String16(),
      String16("\0\0", 2),
  };
  for (const auto& input : inputs) {
    String16 reply;
    status_t status = s->RepeatString(input, &reply);
    if (status != OK || input != reply) {
      cerr << "Failed while requesting service to repeat String16=\""
           << String8(input).string()
           << "\". Got status=" << status << endl;
      return false;
    }
  }
  return true;
}

template <typename T>
bool ReverseArray(const sp<ITestService>& service,
                  status_t(ITestService::*func)(const vector<T>&,
                                                vector<T>*,
                                                vector<T>*),
                  vector<T> input) {
  vector<T> actual_reversed;
  vector<T> actual_repeated;
  status_t status = (*service.*func)(input, &actual_repeated, &actual_reversed);
  if (status != OK) {
    cerr << "Failed to repeat array. status=" << status << "." << endl;
    return false;
  }
  if (input != actual_repeated) {
    cerr << "Repeated version of array did not match" << endl;
    cerr << "input.size()=" << input.size()
         << " repeated.size()=" << actual_repeated.size() << endl;
    return false;
  }
  std::reverse(input.begin(), input.end());
  if (input != actual_reversed) {
    cerr << "Reversed version of array did not match" << endl;
    return false;
  }
  return true;
}

bool ConfirmReverseArrays(const sp<ITestService>& s) {
  cout << "Confirming passing and returning arrays works." << endl;

  if (!ReverseArray(s, &ITestService::ReverseBoolean,
                    {true, false, false}) ||
      !ReverseArray(s, &ITestService::ReverseByte,
                    {int8_t{-128}, int8_t{0}, int8_t{127}}) ||
      !ReverseArray(s, &ITestService::ReverseChar,
                    {char16_t{'A'}, char16_t{'B'}, char16_t{'C'}}) ||
      !ReverseArray(s, &ITestService::ReverseInt,
                    {1, 2, 3}) ||
      !ReverseArray(s, &ITestService::ReverseLong,
                    {-1ll, 0ll, int64_t{1ll << 60}}) ||
      !ReverseArray(s, &ITestService::ReverseFloat,
                    {-0.3f, -0.7f, 8.0f}) ||
      !ReverseArray(s, &ITestService::ReverseDouble,
                    {1.0/3.0, 1.0/7.0, 42.0}) ||
      !ReverseArray(s, &ITestService::ReverseString,
                    {String16{"f"}, String16{"a"}, String16{"b"}})) {
    return false;
  }

  return true;
}

bool ConfirmReverseLists(const sp<ITestService>& s) {
  cout << "Confirming passing and returning List<T> works." << endl;

  if (!ReverseArray(s, &ITestService::ReverseStringList,
                    {String16{"f"}, String16{"a"}, String16{"b"}})) {
    return false;
  }

  return true;
}

bool ConfirmReverseBinderLists(const sp<ITestService>& s) {
  status_t status;
  cout << "Confirming passing and returning List<T> works with binders." << endl;

  vector<String16> names = {
    String16{"Larry"},
    String16{"Curly"},
    String16{"Moe"}
  };

  vector<sp<IBinder>> input;

  for (int i = 0; i < 3; i++) {
    sp<INamedCallback> got;

    status = s->GetOtherTestService(names[i], &got);
    if (status != OK) {
      cerr << "Could not retrieve service for test." << endl;
      return false;
    }

    input.push_back(INamedCallback::asBinder(got));
  }

  vector<sp<IBinder>> output;
  vector<sp<IBinder>> reversed;

  status = s->ReverseNamedCallbackList(input, &output, &reversed);

  if (output.size() != 3) {
    cerr << "ReverseNamedCallbackList gave repetition with wrong length." << endl;
    return false;
  }

  if (reversed.size() != 3) {
    cerr << "ReverseNamedCallbackList gave reversal with wrong length." << endl;
    return false;
  }

  for (int i = 0; i < 3; i++) {
    String16 ret;
    sp<INamedCallback> named_callback =
        android::interface_cast<INamedCallback>(output[i]);
    status = named_callback->GetName(&ret);

    if (status != OK) {
      cerr << "Could not query INamedCallback from output" << endl;
      return false;
    }

    if (ret != names[i]) {
      cerr << "Output had wrong INamedCallback" << endl;
      return false;
    }
  }

  for (int i = 0; i < 3; i++) {
    String16 ret;
    sp<INamedCallback> named_callback =
        android::interface_cast<INamedCallback>(reversed[i]);
    status = named_callback->GetName(&ret);

    if (status != OK) {
      cerr << "Could not query INamedCallback from reversed output" << endl;
      return false;
    }

    if (ret != names[2 - i]) {
      cerr << "Reversed output had wrong INamedCallback" << endl;
      return false;
    }
  }

  return true;
}
}  // namespace

int main(int /* argc */, char * /* argv */ []) {
  sp<ITestService> service;

  if (!GetService(&service)) return 1;

  if (!ConfirmPrimitiveRepeat(service)) return 1;

  if (!ConfirmReverseArrays(service)) return 1;

  if (!ConfirmReverseLists(service)) return 1;

  if (!ConfirmReverseBinderLists(service)) return 1;

  return 0;
}