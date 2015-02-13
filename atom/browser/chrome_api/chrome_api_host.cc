// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/chrome_api/chrome_api_host.h"

#include "atom/browser/atom_browser_context.h"
#include "atom/browser/chrome_api/chrome_api_dispatcher.h"
#include "atom/common/api/api_messages.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "extensions/common/extension_messages.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "ipc/ipc_message_macros.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/browser/shell_extension_system.h"
#include "extensions/browser/event_router.h"

using extensions::Extension;

namespace atom {

ChromeAPIHost::ChromeAPIHost(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      render_process_host_(web_contents->GetRenderProcessHost()),
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
    IPC_MESSAGE_HANDLER(ChromeAPIHostMsg_AddListener, OnAddListener)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ChromeAPIHost::OnAddListener(const std::string& event_name) {
  LOG(ERROR) << "Event: " << " OnAddListener" << event_name;
  if (render_process_host_)
    LOG(ERROR) << "RenderProcessHost not null";
  else
    LOG(ERROR) << "RenderProcessHost null";
  extensions::ShellExtensionSystem* extension_system =
      static_cast<extensions::ShellExtensionSystem*>(
          extensions::ExtensionSystem::Get(AtomBrowserContext::Get()));
  extensions::EventRouter* event_router = extension_system->event_router();
  if (event_router)
    LOG(ERROR) << "EventRouter not null";
  event_router->AddEventListener(event_name, render_process_host_,
      std::string());
}

void ChromeAPIHost::OnRequest(const std::string& name,
    const base::ListValue& args, const int& request_id) {
  ExtensionHostMsg_Request_Params params;
  params.name = name;
  params.arguments.Swap(args.DeepCopy());
  params.request_id = request_id;
  //params.extension_id = extension->id();
  chrome_api_dispatcher_->Dispatch(params, render_view_host_);
}

}  // namespace atom
