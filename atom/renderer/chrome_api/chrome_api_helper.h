// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_RENDERER_CHROME_API_CHROME_API_HELPER_H_
#define ATOM_RENDERER_CHROME_API_CHROME_API_HELPER_H_

#include "content/public/renderer/render_view_observer.h"

namespace base {
class ListValue;
}

namespace atom {

class ChromeAPIHelper: public content::RenderViewObserver {
 public:
  explicit ChromeAPIHelper(content::RenderView* render_view);

 protected:
  virtual ~ChromeAPIHelper();

 private:
  bool OnMessageReceived(const IPC::Message& message) override;

  void OnChromeApiResponse(const std::string& message,
                           const base::ListValue& results,
                           const int& request_id);

  DISALLOW_COPY_AND_ASSIGN(ChromeAPIHelper);
};

}  // namespace atom

#endif  // ATOM_RENDERER_CHROME_API_CHROME_API_HELPER_H_ 
