// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_ATOM_BROWSER_MAIN_PARTS_H_
#define ATOM_BROWSER_ATOM_BROWSER_MAIN_PARTS_H_

#include "base/timer/timer.h"
#include "brightray/browser/browser_main_parts.h"

#if defined(ENABLE_EXTENSIONS)
namespace extensions {
class ExtensionsBrowserClient;
class ExtensionsClient;
class ShellExtensionSystem;
}
#endif

namespace atom {

class AtomBindings;
class Browser;
class JavascriptEnvironment;
class NodeBindings;
class NodeDebugger;

class AtomBrowserMainParts : public brightray::BrowserMainParts {
 public:
  AtomBrowserMainParts();
  virtual ~AtomBrowserMainParts();

  static AtomBrowserMainParts* Get();

  Browser* browser() { return browser_.get(); }

 protected:
  // Implementations of brightray::BrowserMainParts.
  brightray::BrowserContext* CreateBrowserContext() override;

  // Implementations of content::BrowserMainParts.
  void PostEarlyInitialization() override;
  void PreMainMessageLoopRun() override;
#if defined(OS_MACOSX)
  void PreMainMessageLoopStart() override;
  void PostDestroyThreads() override;
#endif

 private:
#if defined(USE_X11)
  void SetDPIFromGSettings();
#endif

  scoped_ptr<Browser> browser_;
  scoped_ptr<JavascriptEnvironment> js_env_;
  scoped_ptr<NodeBindings> node_bindings_;
  scoped_ptr<AtomBindings> atom_bindings_;
  scoped_ptr<NodeDebugger> node_debugger_;

#if defined(ENABLE_EXTENSIONS)
  // Owned by the KeyedService system.
  extensions::ShellExtensionSystem* extension_system_;

  scoped_ptr<extensions::ExtensionsClient> extensions_client_;
  scoped_ptr<extensions::ExtensionsBrowserClient> extensions_browser_client_;
#endif

  base::Timer gc_timer_;

  static AtomBrowserMainParts* self_;

  DISALLOW_COPY_AND_ASSIGN(AtomBrowserMainParts);
};

}  // namespace atom

#endif  // ATOM_BROWSER_ATOM_BROWSER_MAIN_PARTS_H_
