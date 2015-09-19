// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_API_DESKTOP_CAPTURE_HELPER_
#define ATOM_BROWSER_API_DESKTOP_CAPTURE_HELPER_

#include <map>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/media/desktop_media_picker.h"
#include "chrome/browser/media/native_desktop_media_list.h"
#include "content/public/browser/desktop_media_id.h"
#include "content/public/browser/web_contents_observer.h"
#include "v8.h"

namespace content {
class WebContents;
}

namespace atom {

namespace api {

class DesktopCaptureHelper : public content::WebContentsObserver {
 public:
  // function(error, steam_id)
  using ChooseDesktopMediaCallback =
      base::Callback<void(v8::Local<v8::Value>, std::string)>;

  DesktopCaptureHelper(content::WebContents* web_contents);

  void ChooseDesktopMedia(const std::vector<std::string>& sources,
                          const ChooseDesktopMediaCallback& callback);
  void CancelCapture();

 private:
  void RunCallback(const std::string& error_message,
                   const std::string& source_id,
                   const ChooseDesktopMediaCallback& callback);
  void OnPickerDialogResults(int request_id, content::DesktopMediaID source);

  // content::WebContentsObserver overrides.
  void WebContentsDestroyed() override;

  using ChooseDesktopMediaCallbackMap = std::map<int, ChooseDesktopMediaCallback>;

  ChooseDesktopMediaCallbackMap callbacks_;

  content::WebContents* web_contents_; // weak

  scoped_ptr<DesktopMediaPicker> picker_;
};

}  // namespace api

}  // namespace atom

#endif  // ATOM_BROWSER_API_DESKTOP_CAPTURE_HELPER_
