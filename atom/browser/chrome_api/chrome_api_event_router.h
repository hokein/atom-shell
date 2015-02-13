// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_CHROME_API_CHROME_API_EVENT_ROUTER_H_
#define ATOM_BROWSER_CHROME_API_CHROME_API_EVENT_ROUTER_H_

#include "extensions/browser/event_router.h"

namespace content {
class BrowserContext;
}

namespace extensions {
struct Event;
class ExtensionPrefs;
class EventListener;
}

namespace atom {

class ChromeAPIEventRouter : public extensions::EventRouter {
 public:
  ChromeAPIEventRouter(content::BrowserContext* browser_context,
                       extensions::ExtensionPrefs* extension_pref);

  // Override EventRouter methods
  // Broadcasts an event to every listener registered for that event.
  void BroadcastEvent(scoped_ptr<extensions::Event> event) override;
  // Dispatches an event to the given extension.
  void DispatchEventToExtension(const std::string& extension_id,
                                scoped_ptr<extensions::Event> event) override;
  void OnListenerAdded(const extensions::EventListener* listener) override;

  //DISALLOW_COPY_AND_ASSIGN(ChromeAPIEventRouter);
};

}  // namespace atom

#endif  // ATOM_BROWSER_CHROME_API_CHROME_API_EVENT_ROUTER_H_
