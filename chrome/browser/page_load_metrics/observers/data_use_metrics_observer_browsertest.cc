// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/page_load_metrics/observers/data_use_metrics_observer.h"

#include "base/run_loop.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/data_use_measurement/core/data_use_user_data.h"
#include "content/public/test/browser_test_base.h"
#include "content/public/test/browser_test_utils.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "services/network/public/cpp/features.h"
#include "testing/gtest/include/gtest/gtest.h"

class DataUseMetricsObserverBrowserTest : public InProcessBrowserTest {
 protected:
  void SetUp() override {
    scoped_feature_list_.InitAndEnableFeature(
        network::features::kNetworkService);
    InProcessBrowserTest::SetUp();
  }
  base::test::ScopedFeatureList scoped_feature_list_;
};

IN_PROC_BROWSER_TEST_F(DataUseMetricsObserverBrowserTest,
                       NavigateToSimplePage) {
  const struct {
    std::string url;
    size_t expected_min_page_size;
    size_t expected_max_page_size;
  } tests[] = {
      // The range of the pages is calculated approximately from the html size
      // and the size of the subresources it includes.
      {"/google/google.html", 5000, 20000},
      {"/simple.html", 100, 1000},
      {"/media/youtube.html", 5000, 20000},
  };
  ASSERT_TRUE(embedded_test_server()->Start());

  for (const auto& test : tests) {
    base::HistogramTester histogram_tester;
    ui_test_utils::NavigateToURL(browser(),
                                 embedded_test_server()->GetURL(test.url));

    base::RunLoop().RunUntilIdle();
    // Navigate away to finish the histogram recording.
    ui_test_utils::NavigateToURL(browser(), GURL(url::kAboutBlankURL));

    uint64_t total_usage = 0, total_apptabstate_usage = 0;
    for (const auto& sample : histogram_tester.GetAllSamples(
             "DataUse.TrafficSize.User.Downstream.Foreground.NotCellular")) {
      total_usage += sample.min * sample.count;
    }
    for (const auto& sample : histogram_tester.GetAllSamples(
             "DataUse.AppTabState.Downstream.AppForeground.TabForeground")) {
      total_apptabstate_usage += sample.min * sample.count;
    }

    EXPECT_LE(test.expected_min_page_size, total_usage);
    EXPECT_GE(test.expected_max_page_size, total_usage);
    EXPECT_LE(test.expected_min_page_size, total_apptabstate_usage);
    EXPECT_GE(test.expected_max_page_size, total_apptabstate_usage);
  }
}

IN_PROC_BROWSER_TEST_F(DataUseMetricsObserverBrowserTest, TestContentType) {
  ASSERT_TRUE(embedded_test_server()->Start());
  base::HistogramTester histogram_tester;
  ui_test_utils::NavigateToURL(
      browser(), embedded_test_server()->GetURL("/google/google.html"));

  base::RunLoop().RunUntilIdle();
  // Navigate away to finish the histogram recording.
  ui_test_utils::NavigateToURL(browser(), GURL(url::kAboutBlankURL));

  base::HistogramBase::Count main_frame_html_data_use =
      histogram_tester.GetBucketCount(
          "DataUse.ContentType.UserTrafficKB",
          data_use_measurement::DataUseUserData::MAIN_FRAME_HTML);
  base::HistogramBase::Count image_data_use = histogram_tester.GetBucketCount(
      "DataUse.ContentType.UserTrafficKB",
      data_use_measurement::DataUseUserData::IMAGE);

  // Verify that some bytes are recorded for the main frame html and image.
  EXPECT_LE(1, main_frame_html_data_use);
  EXPECT_LE(1, image_data_use);
}
