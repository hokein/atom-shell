// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/api/desktop_capture_helper.h"

#include "atom/browser/browser.h"
#include "atom/browser/native_window.h"
#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "third_party/webrtc/modules/desktop_capture/screen_capturer.h"
#include "third_party/webrtc/modules/desktop_capture/window_capturer.h"
#include "v8/include/v8.h"

namespace atom {

namespace api {

DesktopCaptureHelper::DesktopCaptureHelper(content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

void DesktopCaptureHelper::ChooseDesktopMedia(
    const std::vector<std::string>& sources,
    const ChooseDesktopMediaCallback& callback) {
  Observe(web_contents_);

  static int request_id = 0;
  bool show_screens = false;
  bool show_windows = false;
  for (const auto& source_type : sources) {
    if (source_type == "screen") {
      show_screens = true;
    } else if (source_type == "window") {
      show_windows = true;
    }
  }

  if (!show_windows && !show_screens) {
    RunCallback("At least one source type must be specified.", "", callback);
    return;
  }


  webrtc::DesktopCaptureOptions options =
      webrtc::DesktopCaptureOptions::CreateDefault();

#if defined(OS_WIN)
  // On windows, desktop effects (e.g. Aero) will be disabled when the Desktop
  // capture API is active by default.
  // We keep the desktop effects in most times. Howerver, the screen still
  // fickers when the API is capturing the window due to limitation of current
  // implemetation. This is a known and wontFix issue in webrtc (see:
  // http://code.google.com/p/webrtc/issues/detail?id=3373)
  options.set_disable_effects(false);
#endif

  const gfx::NativeWindow parent_window =
      web_contents_->GetTopLevelNativeWindow();
  scoped_ptr<webrtc::ScreenCapturer> screen_capturer(
      show_screens ? webrtc::ScreenCapturer::Create(options) : nullptr);
  scoped_ptr<webrtc::WindowCapturer> window_capturer(
      show_windows ? webrtc::WindowCapturer::Create(options) : nullptr);
  scoped_ptr<DesktopMediaList> media_list(new NativeDesktopMediaList(
      screen_capturer.Pass(), window_capturer.Pass()));

  // DesktopMediaPicker is implemented only for Windows, OSX and
  // Aura Linux builds.
#if defined(TOOLKIT_VIEWS) || defined(OS_MACOSX)
  picker_ = DesktopMediaPicker::Create();
#else
  RunCallback("The desktop capture API doesn't support this platform.",
              "",
              callback)
#endif

  callbacks_[++request_id] = callback;
  DesktopMediaPicker::DoneCallback picker_callback = base::Bind(
      &DesktopCaptureHelper::OnPickerDialogResults,
      base::Unretained(this),
      request_id);

  std::string app_name = Browser::Get()->GetName();
  picker_->Show(web_contents_,
                parent_window,
                parent_window,
                base::UTF8ToUTF16(app_name),
                base::UTF8ToUTF16(app_name),
                media_list.Pass(),
                picker_callback);
}

void DesktopCaptureHelper::CancelCapture() {
  if (picker_) {
    picker_.reset();
  }
}

void DesktopCaptureHelper::RunCallback(
    const std::string& error_message,
    const std::string& source_id,
    const ChooseDesktopMediaCallback& callback) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Locker locker(isolate);
  v8::HandleScope handle_scope(isolate);
  if (!error_message.empty()) {
    v8::Local<v8::String> error = v8::String::NewFromUtf8(
        isolate, error_message.c_str());
    callback.Run(v8::Exception::Error(error), "");
  } else {
    callback.Run(v8::Null(isolate), source_id);
  }
}

void DesktopCaptureHelper::OnPickerDialogResults(
    int request_id, content::DesktopMediaID source) {
  std::string result;
  if (source.type != content::DesktopMediaID::TYPE_NONE && web_contents_)
    result = source.ToString();

  RunCallback("", result, callbacks_[request_id]);
  callbacks_.erase(request_id);
}

void DesktopCaptureHelper::WebContentsDestroyed() {
  CancelCapture();
}

}  // namespace api

}  // namespace atom
