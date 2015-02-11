// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/renderer/chrome_api/chrome_api_helper.h"

#include "atom/common/api/api_messages.h"
#include "atom/common/native_mate_converters/value_converter.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebKit.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "atom/common/node_includes.h"

namespace atom {

namespace {

bool GetChromeApiRequestSenderObject(v8::Isolate* isolate,
    v8::Handle<v8::Context> context,
    v8::Handle<v8::Object>* chrome_api_request_sender) {
  v8::Handle<v8::String> key = mate::StringToV8(isolate,
                                                "chrome_api_request_sender");
  v8::Handle<v8::Value> value = context->Global()->GetHiddenValue(key);
  if (value.IsEmpty() || !value->IsObject())
    return false;
  *chrome_api_request_sender = value->ToObject();
  return true;
}

std::vector<v8::Handle<v8::Value>> ListValueToVector(
    v8::Isolate* isolate,
    const base::ListValue& list) {
  v8::Handle<v8::Value> array = mate::ConvertToV8(isolate, list);
  std::vector<v8::Handle<v8::Value>> result;
  mate::ConvertFromV8(isolate, array, &result);
  return result;
}

}  // namespace

ChromeAPIHelper::ChromeAPIHelper(content::RenderView* render_view)
    : content::RenderViewObserver(render_view) {
}

ChromeAPIHelper::~ChromeAPIHelper() {
}

bool ChromeAPIHelper::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ChromeAPIHelper, message)
    IPC_MESSAGE_HANDLER(ChromeAPIMsg_Response, OnChromeApiResponse)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ChromeAPIHelper::OnChromeApiResponse(const std::string& message,
    const base::ListValue& values, const int& request_id) {
  if (!render_view()->GetWebView())
    return;

  blink::WebFrame* frame = render_view()->GetWebView()->mainFrame();
  if (!frame || frame->isWebRemoteFrame())
    return;

  v8::Isolate* isolate = blink::mainThreadIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Context> context = frame->mainWorldScriptContext();
  v8::Context::Scope context_scope(context);

  std::vector<v8::Handle<v8::Value>> arguments = ListValueToVector(
      isolate, values);
  arguments.insert(arguments.begin(), mate::ConvertToV8(isolate, message));
  arguments.insert(arguments.begin(), mate::ConvertToV8(isolate, request_id));

  v8::Handle<v8::Object> chrome_api_request_sender;
  if (GetChromeApiRequestSenderObject(isolate, context,
      &chrome_api_request_sender)) {
    node::MakeCallback(isolate, chrome_api_request_sender, "_handleResponse",
        arguments.size(), &arguments[0]);
  }
}

}  // namespace atom
