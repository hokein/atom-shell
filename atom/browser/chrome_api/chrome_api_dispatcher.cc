// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/chrome_api/chrome_api_dispatcher.h"

#include "atom/browser/atom_browser_context.h"
#include "atom/common/api/api_messages.h"

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "extensions/browser/extension_function_registry.h"

#include "extensions/browser/process_manager.h"
#include "extensions/browser/process_map.h"
#include "extensions/common/extension_messages.h"
#include "extensions/common/extension_api.h"


using content::BrowserThread;
using content::RenderViewHost;
using extensions::Extension;
using extensions::ExtensionAPI;

namespace atom {

class ChromeAPIDispatcher::UIThreadResponseCallbackWrapper
    : public content::WebContentsObserver {
 public:
  UIThreadResponseCallbackWrapper(
      const base::WeakPtr<ChromeAPIDispatcher>& dispatcher,
      RenderViewHost* render_view_host)
      : content::WebContentsObserver(
            content::WebContents::FromRenderViewHost(render_view_host)),
        dispatcher_(dispatcher),
        render_view_host_(render_view_host),
        weak_ptr_factory_(this) {
  }

  ~UIThreadResponseCallbackWrapper() override {}

  // content::WebContentsObserver overrides.
  void RenderViewDeleted(RenderViewHost* render_view_host) override {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    if (render_view_host != render_view_host_)
      return;

    if (dispatcher_.get()) {
      dispatcher_->ui_thread_response_callback_wrappers_
          .erase(render_view_host);
    }

    LOG(ERROR) << "Delete ExtensionFunctionWrapper";
    delete this;
  }

  ExtensionFunction::ResponseCallback CreateCallback(int request_id) {
    return base::Bind(
        &UIThreadResponseCallbackWrapper::OnExtensionFunctionCompleted,
        weak_ptr_factory_.GetWeakPtr(),
        request_id);
  }

 private:
  void OnExtensionFunctionCompleted(int request_id,
                                    ExtensionFunction::ResponseType type,
                                    const base::ListValue& results,
                                    const std::string& error) {
    render_view_host_->Send(new ChromeAPIMsg_Response(
        render_view_host_->GetRoutingID(), error, results, request_id));
  }

  base::WeakPtr<ChromeAPIDispatcher> dispatcher_;
  content::RenderViewHost* render_view_host_;
  base::WeakPtrFactory<UIThreadResponseCallbackWrapper> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(UIThreadResponseCallbackWrapper);
};

ChromeAPIDispatcher::ChromeAPIDispatcher():
  browser_context_(AtomBrowserContext::Get()) {
}

void ChromeAPIDispatcher::Dispatch(
    const ExtensionHostMsg_Request_Params& params,
    RenderViewHost* render_view_host) {
  UIThreadResponseCallbackWrapperMap::const_iterator
      iter = ui_thread_response_callback_wrappers_.find(render_view_host);
  UIThreadResponseCallbackWrapper* callback_wrapper = NULL;
  if (iter == ui_thread_response_callback_wrappers_.end()) {
    callback_wrapper = new UIThreadResponseCallbackWrapper(AsWeakPtr(),
                                                           render_view_host);
    ui_thread_response_callback_wrappers_[render_view_host] = callback_wrapper;
  } else {
    callback_wrapper = iter->second;
  }

  DispatchWithCallbackInternal(
      params, render_view_host, NULL,
      callback_wrapper->CreateCallback(params.request_id));
}

void ChromeAPIDispatcher::DispatchWithCallbackInternal(
    const ExtensionHostMsg_Request_Params& params,
    RenderViewHost* render_view_host,
    content::RenderFrameHost* render_frame_host,
    const ExtensionFunction::ResponseCallback& callback) {
  DCHECK(render_view_host || render_frame_host);
  // TODO(yzshen): There is some shared logic between this method and
  // DispatchOnIOThread(). It is nice to deduplicate.
  extensions::ProcessMap* process_map = extensions::ProcessMap::Get(
      browser_context_);
  if (!process_map)
    return;

  const Extension* extension = NULL;
  int process_id = render_view_host ? render_view_host->GetProcess()->GetID() :
                                      render_frame_host->GetProcess()->GetID();
  scoped_refptr<ExtensionFunction> function(
      CreateExtensionFunction(params,
                              extension,
                              process_id,
                              *process_map,
                              ExtensionAPI::GetSharedInstance(),
                              browser_context_,
                              callback));
  if (!function.get()) {
    LOG(ERROR) << "ExtensionFunction" << params.name <<  " creat failed: ";
    return;
  }

  UIThreadExtensionFunction* function_ui =
      function->AsUIThreadExtensionFunction();
  if (!function_ui) {
    NOTREACHED();
    return;
  }
  if (render_view_host) {
    function_ui->SetRenderViewHost(render_view_host);
  } else {
    function_ui->SetRenderFrameHost(render_frame_host);
  }
  function_ui->set_browser_context(browser_context_);
  function->Run()->Execute();
}

ExtensionFunction* ChromeAPIDispatcher::CreateExtensionFunction(
    //const base::ListValue& arguments,
    const ExtensionHostMsg_Request_Params& params,
    const Extension* extension,
    int requesting_process_id,
    const extensions::ProcessMap& process_map,
    ExtensionAPI* api,
    void* profile_id,
    const ExtensionFunction::ResponseCallback& callback) {
  ExtensionFunction* function =
      ExtensionFunctionRegistry::GetInstance()->NewFunction(
          params.name);
  if (!function) {
    LOG(ERROR) << "Unknown Extension API - " << params.name;
    return NULL;
  }

  function->SetArgs(&params.arguments);
  function->set_source_url(params.source_url);
  function->set_request_id(params.request_id);
  function->set_has_callback(params.has_callback);
  function->set_user_gesture(params.user_gesture);
  function->set_extension(extension);
  function->set_profile_id(profile_id);
  function->set_response_callback(callback);
  function->set_source_tab_id(params.source_tab_id);
  function->set_source_context_type(
      process_map.GetMostLikelyContextType(extension, requesting_process_id));

  return function;
}

}  // namespace atom
