#include "atom/browser/chrome_api/chrome_api_event_router.h"

#include "atom/browser/atom_browser_context.h"
#include "content/public/browser/browser_context.h"

using extensions::Event;
using extensions::EventListener;

namespace atom {

ChromeAPIEventRouter::ChromeAPIEventRouter(
    content::BrowserContext* browser_context,
    extensions::ExtensionPrefs* extension_pref)
        :extensions::EventRouter(browser_context, extension_pref) {
}

void ChromeAPIEventRouter::OnListenerAdded(const EventListener* listener) {
  LOG(ERROR) << "OnListenerAdded: " << listener->event_name();
  const extensions::EventListenerInfo details(listener->event_name(),
                                  listener->extension_id(),
                                  listener->listener_url(),
                                  AtomBrowserContext::Get());

  std::string base_event_name = GetBaseEventName(listener->event_name());
  LOG(ERROR) << base_event_name;
  ObserverMap::iterator observer = observers_.find(base_event_name);
  if (observer != observers_.end()) {
    LOG(ERROR) << "Find observer!";
    observer->second->OnListenerAdded(details);
  }
}

void ChromeAPIEventRouter::BroadcastEvent(scoped_ptr<Event> event) {
  LOG(ERROR) << "BroadcastEvent: " << event->event_name;
}

void ChromeAPIEventRouter::DispatchEventToExtension(const std::string& extension_id,
    scoped_ptr<Event> event) {
  LOG(ERROR) << "BroadcastEventToExtension: " << event->event_name;
}

}  // namespace atom
