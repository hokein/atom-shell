// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/chrome_api/chrome_api_host.h"

#include "atom/browser/chrome_api/chrome_api_dispatcher.h"
#include "atom/common/api/api_messages.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "extensions/common/extension_messages.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "ipc/ipc_message_macros.h"

namespace atom {

ChromeAPIHost::ChromeAPIHost(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      chrome_api_dispatcher_(new ChromeAPIDispatcher()) {
}

ChromeAPIHost::~ChromeAPIHost() {
}

void ChromeAPIHost::RenderViewCreated(
    content::RenderViewHost* render_view_host) {
  render_view_host_ = render_view_host;
}

bool ChromeAPIHost::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ChromeAPIHost, message)
    IPC_MESSAGE_HANDLER(ChromeAPIHostMsg_Request, OnRequest)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ChromeAPIHost::OnRequest(const std::string& name,
    const base::ListValue& args, const int& request_id) {
  ExtensionHostMsg_Request_Params params;
  params.name = name;
  params.arguments.Swap(args.DeepCopy());
  params.request_id = request_id;
  chrome_api_dispatcher_->Dispatch(params, render_view_host_);
}

}  // namespace atom
