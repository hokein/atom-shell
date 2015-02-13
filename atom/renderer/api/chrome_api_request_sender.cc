// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/renderer/api/chrome_api_request_sender.h"

#include "atom/common/api/api_messages.h"
#include "atom/common/native_mate_converters/value_converter.h"
#include "content/public/renderer/render_view.h"
#include "native_mate/callback.h"
#include "native_mate/dictionary.h"
#include "native_mate/object_template_builder.h"
#include "native_mate/dictionary.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "atom/common/node_includes.h"

namespace {

content::RenderView* GetRenderView() {
  blink::WebLocalFrame* frame = blink::WebLocalFrame::frameForCurrentContext();
  if (!frame)
    return NULL;

  blink::WebView* view = frame->view();
  if (!view)
    return NULL;  // can happen during closing.

  return content::RenderView::FromWebView(view);
}

}

namespace atom {

namespace api {

ChromeApiRequestSender::ChromeApiRequestSender() {}

ChromeApiRequestSender::~ChromeApiRequestSender() {}

void ChromeApiRequestSender::SendRequest(const std::string& name,
    const base::ListValue& arguments, const int& request_id) {
  content::RenderView* render_view = GetRenderView();
  render_view->Send(new ChromeAPIHostMsg_Request(
        render_view->GetRoutingID(), name, arguments, request_id));
}

int ChromeApiRequestSender::GetNextRequestID() {
  static int request_id = 0;
  return request_id++;
}

void ChromeApiRequestSender::AttachEvent(const std::string& event_name) {
  content::RenderView* render_view = GetRenderView();
  render_view->Send(new ChromeAPIHostMsg_AddListener(
        render_view->GetRoutingID(), event_name));
}

mate::ObjectTemplateBuilder ChromeApiRequestSender::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return mate::ObjectTemplateBuilder(isolate)
      .SetMethod("_sentRequest", &ChromeApiRequestSender::SendRequest)
      .SetMethod("_getNextRequestID",
          &ChromeApiRequestSender::GetNextRequestID)
      .SetMethod("_attachEvent", &ChromeApiRequestSender::AttachEvent);
}

mate::Handle<ChromeApiRequestSender> ChromeApiRequestSender::Create(
    v8::Isolate* isolate) {
  return CreateHandle(isolate, new ChromeApiRequestSender);
}

}  // namespace api

}  // namespace atom

namespace {

void Initialize(v8::Handle<v8::Object> exports, v8::Handle<v8::Value> unused,
                v8::Handle<v8::Context> context, void* priv) {
  v8::Isolate* isolate = context->GetIsolate();
  mate::Dictionary dict(isolate, exports);
  dict.Set("chromeApiRequestSender", atom::api::ChromeApiRequestSender::Create(
        isolate));
}

}  // namespace

NODE_MODULE_CONTEXT_AWARE_BUILTIN(atom_renderer_chrome_api_request_sender,
    Initialize)
