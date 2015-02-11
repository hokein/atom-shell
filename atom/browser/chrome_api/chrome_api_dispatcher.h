// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_CHROME_API_CHROME_API_DISPATCHER_H_
#define ATOM_BROWSER_CHROME_API_CHROME_API_DISPATCHER_H_

#include "base/memory/weak_ptr.h"
#include "extensions/browser/extension_function.h"

struct ExtensionHostMsg_Request_Params;

namespace content {
class BrowserContext;
class RenderFrameHost;
class RenderViewHost;
class WebContents;
}

namespace extensions {
class Extension;
class ExtensionAPI;
class ExtensionMessageFilter;
class InfoMap;
class ProcessMap;
}

namespace atom {

class ChromeAPIDispatcher : public base::SupportsWeakPtr<ChromeAPIDispatcher> {
 public:
  ChromeAPIDispatcher();

  // The response is sent to the corresponding render view in an
  // ExtensionMsg_Response message.
  void Dispatch(const ExtensionHostMsg_Request_Params& params,
                content::RenderViewHost* render_view_host);

 private:
  // For a given RenderViewHost instance, UIThreadResponseCallbackWrapper
  // creates ExtensionFunction::ResponseCallback instances which send responses
  // to the corresponding render view in ExtensionMsg_Response messages.
  // This class tracks the lifespan of the RenderViewHost instance, and will be
  // destroyed automatically when it goes away.
  class UIThreadResponseCallbackWrapper;

  static ExtensionFunction* CreateExtensionFunction(
      const ExtensionHostMsg_Request_Params& params,
      const extensions::Extension* extension,
      int requesting_process_id,
      const extensions::ProcessMap& process_map,
      extensions::ExtensionAPI* api,
      void* profile_id,
      const ExtensionFunction::ResponseCallback& callback);

  void DispatchWithCallbackInternal(
      const ExtensionHostMsg_Request_Params& params,
      content::RenderViewHost* render_view_host,
      content::RenderFrameHost* render_frame_host,
      const ExtensionFunction::ResponseCallback& callback);

  // The BrowserContext that this dispatcher is associated with.
  content::BrowserContext* browser_context() { return browser_context_; }

  content::BrowserContext* browser_context_;

  typedef std::map<content::RenderViewHost*, UIThreadResponseCallbackWrapper*>
      UIThreadResponseCallbackWrapperMap;
  UIThreadResponseCallbackWrapperMap ui_thread_response_callback_wrappers_;
};

}  // namespace atom

#endif  // ATOM_BROWSER_CHROME_API_CHROME_API_DISPATCHER_H_
