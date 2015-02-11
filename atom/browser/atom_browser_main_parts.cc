// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/atom_browser_main_parts.h"

#include "atom/browser/atom_browser_client.h"
#include "atom/browser/atom_browser_context.h"
#include "atom/browser/browser.h"
#include "atom/browser/javascript_environment.h"
#include "atom/browser/node_debugger.h"
#include "atom/common/api/atom_bindings.h"
#include "atom/common/node_bindings.h"
#include "base/command_line.h"
#include "v8/include/v8-debug.h"

#if defined(USE_X11)
#include "chrome/browser/ui/libgtk2ui/gtk2_util.h"
#endif

#include "atom/common/node_includes.h"

#if defined(ENABLE_EXTENSIONS)
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/storage_monitor/storage_monitor.h"
#include "extensions/browser/shell_extension_system.h"
#include "extensions/browser/shell_extension_system_factory.h"
#include "extensions/browser/shell_extensions_browser_client.h"
#include "extensions/common/shell_extensions_client.h"
#include "extensions/browser/browser_context_keyed_service_factories.h"
#endif

namespace atom {

// static
AtomBrowserMainParts* AtomBrowserMainParts::self_ = NULL;

AtomBrowserMainParts::AtomBrowserMainParts()
    : browser_(new Browser),
      node_bindings_(NodeBindings::Create(true)),
      atom_bindings_(new AtomBindings),
      gc_timer_(true, true) {
  DCHECK(!self_) << "Cannot have two AtomBrowserMainParts";
  self_ = this;
}

AtomBrowserMainParts::~AtomBrowserMainParts() {
}

// static
AtomBrowserMainParts* AtomBrowserMainParts::Get() {
  DCHECK(self_);
  return self_;
}

brightray::BrowserContext* AtomBrowserMainParts::CreateBrowserContext() {
  return new AtomBrowserContext();
}

void AtomBrowserMainParts::PostEarlyInitialization() {
  brightray::BrowserMainParts::PostEarlyInitialization();

#if defined(USE_X11)
  SetDPIFromGSettings();
#endif

  // The ProxyResolverV8 has setup a complete V8 environment, in order to avoid
  // conflicts we only initialize our V8 environment after that.
  js_env_.reset(new JavascriptEnvironment);

  node_bindings_->Initialize();

  // Support the "--debug" switch.
  node_debugger_.reset(new NodeDebugger(js_env_->isolate()));

  // Create the global environment.
  global_env = node_bindings_->CreateEnvironment(js_env_->context());

  // Make sure node can get correct environment when debugging.
  if (node_debugger_->IsRunning())
    global_env->AssignToContext(v8::Debug::GetDebugContext());

  // Add atom-shell extended APIs.
  atom_bindings_->BindTo(js_env_->isolate(), global_env->process_object());

  // Load everything.
  node_bindings_->LoadEnvironment(global_env);
}

void AtomBrowserMainParts::PreMainMessageLoopRun() {
  // Run user's main script before most things get initialized, so we can have
  // a chance to setup everything.
  node_bindings_->PrepareMessageLoop();
  node_bindings_->RunMessageLoop();

  // Start idle gc.
  gc_timer_.Start(
      FROM_HERE, base::TimeDelta::FromMinutes(1),
      base::Bind(base::IgnoreResult(&v8::Isolate::IdleNotification),
                 base::Unretained(js_env_->isolate()),
                 1000));

  brightray::BrowserMainParts::PreMainMessageLoopRun();

#if defined(ENABLE_EXTENSIONS)
  storage_monitor::StorageMonitor::Create();
  extensions_client_.reset(new extensions::ShellExtensionsClient());
  extensions::ExtensionsClient::Set(extensions_client_.get());

  extensions_browser_client_.reset(
      new extensions::ShellExtensionsBrowserClient(browser_context()));
  extensions::ExtensionsBrowserClient::Set(extensions_browser_client_.get());

  // Create our custom ExtensionSystem first because other
  // KeyedServices depend on it.
  extension_system_ = static_cast<extensions::ShellExtensionSystem*>(
            extensions::ExtensionSystem::Get(browser_context()));
  extension_system_->InitForRegularProfile(true);

  // Register additional KeyedService factories here. See
  // ChromeBrowserMainExtraPartsProfiles for details.
  extensions::EnsureBrowserContextKeyedServiceFactoriesBuilt();
  extensions::ShellExtensionSystemFactory::GetInstance();


  BrowserContextDependencyManager::GetInstance()->CreateBrowserContextServices(
      browser_context());
  extensions::ShellExtensionSystem* extension_system =
      static_cast<extensions::ShellExtensionSystem*>(
          extensions::ExtensionSystem::Get(browser_context()));
  extension_system->Init();
#endif

#if defined(USE_X11)
  libgtk2ui::GtkInitFromCommandLine(*CommandLine::ForCurrentProcess());
#endif

#if !defined(OS_MACOSX)
  // The corresponding call in OS X is in AtomApplicationDelegate.
  Browser::Get()->WillFinishLaunching();
  Browser::Get()->DidFinishLaunching();
#endif
}

}  // namespace atom
