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

#include "components/adblock/core/adblock_controller_legacy_impl.h"

#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/test/task_environment.h"
#include "components/adblock/core/adblock_switches.h"
#include "components/adblock/core/common/adblock_constants.h"
#include "components/adblock/core/common/adblock_prefs.h"
#include "components/adblock/core/configuration/fake_filtering_configuration.h"
#include "components/adblock/core/subscription/subscription_config.h"
#include "components/adblock/core/subscription/test/mock_subscription.h"
#include "components/adblock/core/subscription/test/mock_subscription_service.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/testing_pref_service.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace adblock {

using InstallationState = Subscription::InstallationState;
using testing::_;
using testing::Return;

namespace {}  // namespace

class AdblockControllerLegacyImplTest : public testing::Test {
 public:
  AdblockControllerLegacyImplTest() {
    prefs::RegisterProfilePrefs(pref_service_.registry());
    custom_subscriptions_pref_.Init(prefs::kAdblockCustomSubscriptionsLegacy,
                                    &pref_service_);
    subscriptions_pref_.Init(prefs::kAdblockSubscriptionsLegacy,
                             &pref_service_);
    allowed_domains_pref_.Init(prefs::kAdblockAllowedDomainsLegacy,
                               &pref_service_);
    custom_filters_pref_.Init(prefs::kAdblockCustomFiltersLegacy,
                              &pref_service_);
  }

  void RecreateController(
      std::vector<KnownSubscriptionInfo> known_subscriptions =
          config::GetKnownSubscriptions()) {
    testee_ = std::make_unique<AdblockControllerLegacyImpl>(
        &pref_service_, &filtering_configuration_, &subscription_service_,
        "pl-PL", std::move(known_subscriptions));
    testee_->ReadStateFromPrefs();
  }

  base::test::TaskEnvironment task_environment_;
  TestingPrefServiceSimple pref_service_;
  FakeFilteringConfiguration filtering_configuration_;
  MockSubscriptionService subscription_service_;
  StringListPrefMember custom_subscriptions_pref_;
  StringListPrefMember subscriptions_pref_;
  StringListPrefMember allowed_domains_pref_;
  StringListPrefMember custom_filters_pref_;
  std::unique_ptr<AdblockControllerLegacyImpl> testee_;
};

TEST_F(AdblockControllerLegacyImplTest,
       SubscriptionsInConfigurationsInitializedFromLegacyPrefs) {
  // Initially, the FilteringConfiguration contains some custom filter lists,
  // for whatever reason.
  const std::vector<GURL> kInitialConfiguration{
      GURL{"https://1.com/filters.txt"}, GURL{"https://2.com/filters.txt"}};
  filtering_configuration_.filter_lists = kInitialConfiguration;

  // The legacy prefs contain a different set of subscriptions that partially
  // overlap with the content of FilteringConfiguration.
  subscriptions_pref_.SetValue({DefaultSubscriptionUrl().spec()});
  custom_subscriptions_pref_.SetValue({kInitialConfiguration[0].spec()});
  // Acceptable Ads are also enabled via the legacy pref.
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, true);

  RecreateController();

  // Filter lists defined in FilteringConfiguration have been replaced with the
  // content of legacy prefs. The Acceptable Ads list is among them.
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(DefaultSubscriptionUrl(),
                                            AcceptableAdsUrl(),
                                            kInitialConfiguration[0]));
}

TEST_F(AdblockControllerLegacyImplTest,
       SubscriptionsInConfigurationsFollowLegacyPrefs) {
  const GURL kCustomList1{"https://1.com/filters.txt"};
  const GURL kCustomList2{"https://2.com/filters.txt"};
  const GURL kBuiltInList1(config::GetKnownSubscriptions()[0].url);
  const GURL kBuiltInList2(config::GetKnownSubscriptions()[1].url);
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, false);
  RecreateController();

  // Changes to legacy prefs are reflected in the FilteringConfiguration.
  // New custom list added through prefs:
  custom_subscriptions_pref_.SetValue(
      {kCustomList1.spec(), kCustomList2.spec()});
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kCustomList1, kCustomList2));
  // New built-in list added through prefs:
  subscriptions_pref_.SetValue({kBuiltInList1.spec(), kBuiltInList2.spec()});
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kCustomList1, kCustomList2,
                                            kBuiltInList1, kBuiltInList2));

  // Custom list deleted from prefs:
  custom_subscriptions_pref_.SetValue({kCustomList1.spec()});
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kCustomList1, kBuiltInList1,
                                            kBuiltInList2));
  // Built-in list deleted from prefs:
  subscriptions_pref_.SetValue({kBuiltInList1.spec()});
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kCustomList1, kBuiltInList1));

  // Acceptable Ads enabled:
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, true);
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kCustomList1, kBuiltInList1,
                                            AcceptableAdsUrl()));
  // Acceptable Ads disabled:
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, false);
  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kCustomList1, kBuiltInList1));
}

TEST_F(AdblockControllerLegacyImplTest, AllowedDomainsFollowLegacyPrefs) {
  allowed_domains_pref_.SetValue({"www.google.com", "www.abc.com"});
  RecreateController();
  EXPECT_THAT(filtering_configuration_.GetAllowedDomains(),
              testing::UnorderedElementsAre("www.google.com", "www.abc.com"));

  allowed_domains_pref_.SetValue({"www.google.com"});
  EXPECT_THAT(filtering_configuration_.GetAllowedDomains(),
              testing::UnorderedElementsAre("www.google.com"));
}

TEST_F(AdblockControllerLegacyImplTest, CustomFiltersFollowLegacyPrefs) {
  custom_filters_pref_.SetValue({"google", "abc"});
  RecreateController();
  EXPECT_THAT(filtering_configuration_.GetCustomFilters(),
              testing::UnorderedElementsAre("google", "abc"));

  custom_filters_pref_.SetValue({"abc"});
  EXPECT_THAT(filtering_configuration_.GetCustomFilters(),
              testing::UnorderedElementsAre("abc"));
}

TEST_F(AdblockControllerLegacyImplTest, LegacyPrefsFollowApiCalls) {
  const GURL kCustomList1{"https://1.com/filters.txt"};
  const GURL kCustomList2{"https://2.com/filters.txt"};
  const GURL kBuiltInList1(config::GetKnownSubscriptions()[0].url);
  const GURL kBuiltInList2(config::GetKnownSubscriptions()[1].url);
  RecreateController();

  testee_->AddAllowedDomain("www.google.com");
  testee_->AddAllowedDomain("www.abc.com");
  testee_->AddAllowedDomain("www.ddd.com");
  testee_->RemoveAllowedDomain("www.ddd.com");
  EXPECT_THAT(allowed_domains_pref_.GetValue(),
              testing::UnorderedElementsAre("www.google.com", "www.abc.com"));
  EXPECT_THAT(filtering_configuration_.GetAllowedDomains(),
              testing::UnorderedElementsAre("www.google.com", "www.abc.com"));

  testee_->AddCustomFilter("a");
  testee_->AddCustomFilter("b");
  testee_->AddCustomFilter("c");
  testee_->RemoveCustomFilter("a");
  EXPECT_THAT(custom_filters_pref_.GetValue(),
              testing::UnorderedElementsAre("b", "c"));
  EXPECT_THAT(filtering_configuration_.GetCustomFilters(),
              testing::UnorderedElementsAre("b", "c"));

  testee_->SetAcceptableAdsEnabled(false);
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kEnableAcceptableAdsLegacy));

  testee_->AddCustomSubscription(kCustomList1);
  testee_->AddCustomSubscription(kCustomList2);
  testee_->RemoveCustomSubscription(kCustomList1);
  EXPECT_THAT(custom_subscriptions_pref_.GetValue(),
              testing::UnorderedElementsAre(kCustomList2.spec()));

  testee_->SelectBuiltInSubscription(kBuiltInList1);
  testee_->SelectBuiltInSubscription(kBuiltInList2);
  testee_->UnselectBuiltInSubscription(kBuiltInList1);
  EXPECT_THAT(subscriptions_pref_.GetValue(),
              testing::UnorderedElementsAre(kBuiltInList2.spec()));

  EXPECT_THAT(filtering_configuration_.GetFilterLists(),
              testing::UnorderedElementsAre(kBuiltInList2, kCustomList2));

  testee_->SetAcceptableAdsEnabled(false);
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kEnableAcceptableAdsLegacy));
}

class AdblockControllerLegacyImplBooleanArgumentTest
    : public AdblockControllerLegacyImplTest,
      public testing::WithParamInterface<bool> {
 public:
  bool InitialValue() { return GetParam(); }
};

TEST_P(AdblockControllerLegacyImplBooleanArgumentTest,
       EnabledStateReadFromPrefs) {
  pref_service_.SetBoolean(prefs::kEnableAdblockLegacy, InitialValue());
  RecreateController();
  EXPECT_EQ(filtering_configuration_.IsEnabled(), InitialValue());
  EXPECT_EQ(testee_->IsAdblockEnabled(), InitialValue());

  // State is then tracked during external pref changes.
  pref_service_.SetBoolean(prefs::kEnableAdblockLegacy, !InitialValue());
  EXPECT_EQ(filtering_configuration_.IsEnabled(), !InitialValue());
  EXPECT_EQ(testee_->IsAdblockEnabled(), !InitialValue());

  // API calls are reflected into prefs.
  testee_->SetAdblockEnabled(InitialValue());
  EXPECT_EQ(pref_service_.GetBoolean(prefs::kEnableAdblockLegacy),
            InitialValue());
}

TEST_P(AdblockControllerLegacyImplBooleanArgumentTest,
       AcceptableAdsStateReadFromPrefs) {
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, InitialValue());
  RecreateController();
  EXPECT_EQ(testee_->IsAcceptableAdsEnabled(), InitialValue());

  // State is then tracked during external pref changes.
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, !InitialValue());
  EXPECT_EQ(testee_->IsAcceptableAdsEnabled(), !InitialValue());

  // API calls are reflected into prefs.
  testee_->SetAcceptableAdsEnabled(InitialValue());
  EXPECT_EQ(pref_service_.GetBoolean(prefs::kEnableAcceptableAdsLegacy),
            InitialValue());
}

TEST_P(AdblockControllerLegacyImplBooleanArgumentTest,
       AcceptableAdsDisabledByCommandLineSwitch) {
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      switches::kDisableAcceptableAds);
  pref_service_.SetBoolean(prefs::kEnableAcceptableAdsLegacy, InitialValue());
  RecreateController();

  EXPECT_FALSE(testee_->IsAcceptableAdsEnabled());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kEnableAcceptableAdsLegacy));
}

TEST_P(AdblockControllerLegacyImplBooleanArgumentTest,
       AdBlockingDisabledByCommandLineSwitch) {
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      switches::kDisableAdblock);
  pref_service_.SetBoolean(prefs::kEnableAdblockLegacy, InitialValue());
  RecreateController();

  EXPECT_FALSE(testee_->IsAdblockEnabled());
  EXPECT_FALSE(pref_service_.GetBoolean(prefs::kEnableAdblockLegacy));
}

INSTANTIATE_TEST_SUITE_P(All,
                         AdblockControllerLegacyImplBooleanArgumentTest,
                         testing::Values(false, true));

}  // namespace adblock
