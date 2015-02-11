// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_RENDERER_API_CHROME_API_REQUEST_SENDER_H_
#define ATOM_RENDERER_API_CHROME_API_REQUEST_SENDER_H_

#include "base/callback.h"
#include "base/values.h"
#include "native_mate/handle.h"
#include "native_mate/wrappable.h"

namespace atom {

namespace api {

class ChromeApiRequestSender : public mate::Wrappable {
 public:
  static mate::Handle<ChromeApiRequestSender> Create(v8::Isolate* isolate);

 private:
  ChromeApiRequestSender();
  virtual ~ChromeApiRequestSender();

  // mate::Wrappable:
  virtual mate::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate);

  void SendRequest(const std::string& name, const base::ListValue& arguements,
      const int& request_id);
  int GetNextRequestID();

  DISALLOW_COPY_AND_ASSIGN(ChromeApiRequestSender);
};

}  // namespace api

}  // namespace atom

#endif  // ATOM_RENDERER_API_CHROME_API_REQUEST_SENDER_H_
