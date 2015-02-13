// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_CHROME_API_CHROME_API_HOST_H_
#define ATOM_BROWSER_CHROME_API_CHROME_API_HOST_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_observer.h"

namespace base {
class ListValue;
}

namespace IPC {
class Message;
}

namespace content {
class WebContents;
class RenderViewHost;
class RenderProcessHost;
}

namespace atom {

class ChromeAPIDispatcher;

class ChromeAPIHost : public content::WebContentsObserver {
 public:
  ChromeAPIHost(content::WebContents* web_contents);
  virtual ~ChromeAPIHost();

 protected:
  void OnRequest(const std::string& name, const base::ListValue& args,
      const int& request_id);
  void OnAddListener(const std::string& event_name);

  // Implementations of content::WebContentsObserver.
  void RenderViewCreated(content::RenderViewHost* render_view_host) override;
  bool OnMessageReceived(const IPC::Message& message) override;

 private:
  content::RenderViewHost* render_view_host_;
  content::RenderProcessHost* render_process_host_;

  scoped_ptr<ChromeAPIDispatcher> chrome_api_dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(ChromeAPIHost);
};

}  // namespace atom

#endif  // ATOM_BROWSER_CHROME_API_CHROME_API_HOST_H_
