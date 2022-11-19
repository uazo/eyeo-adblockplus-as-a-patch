/*
 * This file is part of eyeo Chromium SDK,
 * Copyright (C) 2006-present eyeo GmbH
 *
 * eyeo Chromium SDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * eyeo Chromium SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eyeo Chromium SDK.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/adblock/core/adblock_switches.h"
#include "components/adblock/core/subscription/subscription_config.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"

namespace adblock {

class AdblockFilterListDownloadTestBase : public InProcessBrowserTest {
 public:
  AdblockFilterListDownloadTestBase()
      : https_server_(net::EmbeddedTestServer::TYPE_HTTPS) {}
  // We need to set server and request handler asap
  void SetUpInProcessBrowserTestFixture() override {
    InProcessBrowserTest::SetUpInProcessBrowserTestFixture();
    host_resolver()->AddRule("easylist-downloads.adblockplus.org", "127.0.0.1");
    https_server_.RegisterRequestHandler(
        base::BindRepeating(&AdblockFilterListDownloadTestBase::RequestHandler,
                            base::Unretained(this)));
    net::EmbeddedTestServer::ServerCertificateConfig cert_config;
    cert_config.dns_names = {"easylist-downloads.adblockplus.org"};
    https_server_.SetSSLConfig(cert_config);
    ASSERT_TRUE(https_server_.Start());
    SetFilterListServerPortForTesting(https_server_.port());
  }

  virtual std::unique_ptr<net::test_server::HttpResponse> RequestHandler(
      const net::test_server::HttpRequest& request) {
    if (base::StartsWith(request.relative_url, "/abp-filters-anti-cv.txt") ||
        base::StartsWith(request.relative_url, "/easylist.txt") ||
        base::StartsWith(request.relative_url, "/exceptionrules.txt")) {
      std::string os;
      base::ReplaceChars(version_info::GetOSType(), base::kWhitespaceASCII, "",
                         &os);
      EXPECT_TRUE(request.relative_url.find("addonName=eyeo-chromium-sdk") !=
                  std::string::npos);
      EXPECT_TRUE(request.relative_url.find("addonVersion=1.0") !=
                  std::string::npos);
      EXPECT_TRUE(request.relative_url.find("platformVersion=1.0") !=
                  std::string::npos);
      EXPECT_TRUE(request.relative_url.find("platform=" + os) !=
                  std::string::npos);
      default_lists_.insert(request.relative_url.substr(
          1, request.relative_url.find_first_of("?") - 1));
    }

    // Unhandled requests result in the Embedded test server sending a 404.
    return nullptr;
  }

  void CloseBrowserFromAnyThread() {
    content::GetUIThreadTaskRunner({base::TaskPriority::USER_BLOCKING})
        ->PostTask(
            FROM_HERE,
            base::BindOnce(
                &AdblockFilterListDownloadTestBase::CloseBrowserAsynchronously,
                base::Unretained(this), browser()));
  }

 protected:
  net::EmbeddedTestServer https_server_;
  std::set<std::string> default_lists_;
};

class AdblockEnabledFilterListDownloadTest
    : public AdblockFilterListDownloadTestBase {
 public:
  std::unique_ptr<net::test_server::HttpResponse> RequestHandler(
      const net::test_server::HttpRequest& request) override {
    auto return_value =
        AdblockFilterListDownloadTestBase::RequestHandler(request);
    // If we get all expected requests we simply finish the test by closing
    // the browser, otherwise test will fail with a timeout.
    if (default_lists_.size() == 3) {
      CloseBrowserFromAnyThread();
    }

    return return_value;
  }
};

IN_PROC_BROWSER_TEST_F(AdblockEnabledFilterListDownloadTest,
                       TestInitialDownloads) {
  RunUntilBrowserProcessQuits();
}

class AdblockDisabledFilterListDownloadTest
    : public AdblockFilterListDownloadTestBase {
 public:
  void SetUpCommandLine(base::CommandLine* command_line) override {
    command_line->AppendSwitch(adblock::switches::kDisableAdblock);
  }

  void VerifyNoDownloads() {
    ASSERT_EQ(0u, default_lists_.size());
    CloseBrowserFromAnyThread();
  }
};

IN_PROC_BROWSER_TEST_F(AdblockDisabledFilterListDownloadTest,
                       TestInitialDownloads) {
  // This test assumes that inital downloads (for adblock enabled) will happen
  // within 10 seconds. When tested locally it always happens within 3 seconds.
  base::OneShotTimer timer;
  timer.Start(
      FROM_HERE, base::Seconds(10),
      base::BindOnce(&AdblockDisabledFilterListDownloadTest::VerifyNoDownloads,
                     base::Unretained(this)));
  RunUntilBrowserProcessQuits();
}

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
class AdblockPtLocaleFilterListDownloadTest
    : public AdblockFilterListDownloadTestBase {
 public:
  AdblockPtLocaleFilterListDownloadTest()
      : AdblockFilterListDownloadTestBase() {
    setenv("LC_ALL", "pt_PT.UTF-8", 1);
  }

  std::unique_ptr<net::test_server::HttpResponse> RequestHandler(
      const net::test_server::HttpRequest& request) override {
    EXPECT_FALSE(base::StartsWith(request.relative_url, "/easylist.txt"));
    if (base::StartsWith(request.relative_url, "/abp-filters-anti-cv.txt") ||
        base::StartsWith(request.relative_url,
                         "/easylistportuguese+easylist.txt") ||
        base::StartsWith(request.relative_url, "/exceptionrules.txt")) {
      default_lists_.insert(request.relative_url.substr(
          1, request.relative_url.find_first_of("?") - 1));
    }

    // If we get all expected requests we simply finish the test by closing
    // the browser, otherwise test will fail with a timeout.
    if (default_lists_.size() == 3) {
      CloseBrowserFromAnyThread();
    }

    // Unhandled requests result in the Embedded test server sending a 404.
    return nullptr;
  }
};

IN_PROC_BROWSER_TEST_F(AdblockPtLocaleFilterListDownloadTest,
                       TestInitialDownloads) {
  RunUntilBrowserProcessQuits();
}
#endif

}  // namespace adblock
