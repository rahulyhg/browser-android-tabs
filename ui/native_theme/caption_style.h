// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_NATIVE_THEME_CAPTION_STYLE_H_
#define UI_NATIVE_THEME_CAPTION_STYLE_H_

#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkScalar.h"
#include "ui/native_theme/native_theme_export.h"

#include <string>

namespace ui {

struct NATIVE_THEME_EXPORT CaptionStyle {
  CaptionStyle();
  CaptionStyle(const CaptionStyle& other);
  ~CaptionStyle();

  // Returns a CaptionStyle parsed from a specification string, which is a
  // serialized JSON object whose keys are strings and whose values are of
  // variable types. See the body of this method for details. This is used to
  // parse the value of the "--force-caption-style" command-line argument and
  // for testing.
  static CaptionStyle FromSpec(const std::string& spec);

  std::string text_color;
  std::string background_color;
};

}  // namespace ui

#endif
