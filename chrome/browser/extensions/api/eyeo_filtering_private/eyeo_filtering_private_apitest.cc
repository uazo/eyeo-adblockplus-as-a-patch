// This file is part of eyeo Chromium SDK,
// Copyright (C) 2006-present eyeo GmbH
//
// eyeo Chromium SDK is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation.
//
// eyeo Chromium SDK is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eyeo Chromium SDK.  If not, see <http://www.gnu.org/licenses/>.

#include "chrome/browser/adblock/adblock_content_browser_client.h"
#include "chrome/browser/adblock/adblock_controller_factory.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/extensions/api/adblock_private.h"
#include "components/adblock/core/adblock_controller.h"
#include "components/adblock/core/common/adblock_constants.h"
#include "content/public/test/browser_test.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace extensions {

namespace {
enum class Mode { Normal, Incognito };
}  // namespace

class EyeoFilteringPrivateApiTest : public ExtensionApiTest,
                                    public testing::WithParamInterface<Mode> {
 public:
  EyeoFilteringPrivateApiTest() {}
  ~EyeoFilteringPrivateApiTest() override = default;
  EyeoFilteringPrivateApiTest(const EyeoFilteringPrivateApiTest&) = delete;
  EyeoFilteringPrivateApiTest& operator=(const EyeoFilteringPrivateApiTest&) =
      delete;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    extensions::ExtensionApiTest::SetUpCommandLine(command_line);
    if (IsIncognito()) {
      command_line->AppendSwitch(switches::kIncognito);
    }
  }

 protected:
  void SetUpOnMainThread() override {
    ExtensionApiTest::SetUpOnMainThread();

    adblock::AdblockControllerFactory::GetForBrowserContext(
        browser()->profile())
        ->RemoveCustomFilter(adblock::kAllowlistEverythingFilter);

    AdblockContentBrowserClient::ForceAdblockProxyForTesting();
  }

  bool IsIncognito() { return GetParam() == Mode::Incognito; }

  bool RunTest(const std::string& subtest) {
    const std::string page_url = "main.html?subtest=" + subtest;
    return RunExtensionTest("eyeo_filtering_private",
                            {.extension_url = page_url.c_str()},
                            {.allow_in_incognito = IsIncognito(),
                             .load_as_component = !IsIncognito()});
  }
};

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       CreateAndGetConfigurations) {
  EXPECT_TRUE(RunTest("createAndGetConfigurations")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       CreateAndGetConfigurationsWithPromises) {
  EXPECT_TRUE(RunTest("createAndGetConfigurationsWithPromises")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       EnableAndDisableConfiguration) {
  EXPECT_TRUE(RunTest("enableAndDisableConfiguration")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       EnableAndDisableConfigurationWithPromises) {
  EXPECT_TRUE(RunTest("enableAndDisableConfigurationWithPromises")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       AddAllowedDomainToCustomConfiguration) {
  EXPECT_TRUE(RunTest("addAllowedDomainToCustomConfiguration")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       AddAllowedDomainToCustomConfigurationWithPromises) {
  EXPECT_TRUE(RunTest("addAllowedDomainToCustomConfigurationWithPromises"))
      << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       AddCustomFilterToCustomConfiguration) {
  EXPECT_TRUE(RunTest("addCustomFilterToCustomConfiguration")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       AddCustomFilterToCustomConfigurationWithPromises) {
  EXPECT_TRUE(RunTest("addCustomFilterToCustomConfigurationWithPromises"))
      << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       SubscribeToFilterListInCustomConfiguration) {
  EXPECT_TRUE(RunTest("subscribeToFilterListInCustomConfiguration"))
      << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       SubscribeToFilterListInCustomConfigurationWithPromises) {
  EXPECT_TRUE(RunTest("subscribeToFilterListInCustomConfigurationWithPromises"))
      << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest, MissingConfiguration) {
  EXPECT_TRUE(RunTest("missingConfiguration")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest,
                       MissingConfigurationWithPromises) {
  EXPECT_TRUE(RunTest("missingConfigurationWithPromises")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest, AllowedDomainsEvent) {
  EXPECT_TRUE(RunTest("allowedDomainsEvent")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest, EnabledStateEvent) {
  EXPECT_TRUE(RunTest("enabledStateEvent")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest, FilterListsEvent) {
  EXPECT_TRUE(RunTest("filterListsEvent")) << message_;
}

IN_PROC_BROWSER_TEST_P(EyeoFilteringPrivateApiTest, CustomFiltersEvent) {
  EXPECT_TRUE(RunTest("customFiltersEvent")) << message_;
}

INSTANTIATE_TEST_SUITE_P(All,
                         EyeoFilteringPrivateApiTest,
                         testing::Values(Mode::Normal, Mode::Incognito));

}  // namespace extensions
