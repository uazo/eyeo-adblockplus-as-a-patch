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

#ifndef COMPONENTS_ADBLOCK_CORE_ADBLOCK_CONTROLLER_LEGACY_IMPL_H_
#define COMPONENTS_ADBLOCK_CORE_ADBLOCK_CONTROLLER_LEGACY_IMPL_H_

#include <string>
#include <vector>

#include "components/adblock/core/adblock_controller_impl.h"
#include "components/prefs/pref_member.h"

class PrefService;

namespace adblock {

/**
 * @brief A wrapper over AdblockControllerImpl that OnPrefChangeds the legacy
 * prefs (kEnableAdblockLegacy, kEnableAcceptableAdsLegacy,
 * kAdblockAllowedDomainsLegacy, kAdblockCustomFiltersLegacy,
 * kAdblockSubscriptionsLegacy, kAdblockCustomSubscriptionsLegacy) with the
 * state tracked by FilteringConfiguration. Scheduled for removal in version
 * 111.
 */
class AdblockControllerLegacyImpl : public AdblockControllerImpl {
 public:
  AdblockControllerLegacyImpl(
      PrefService* pref_service,
      FilteringConfiguration* adblock_filtering_configuration,
      SubscriptionService* subscription_service,
      const std::string& locale,
      std::vector<KnownSubscriptionInfo> known_subscriptions);
  ~AdblockControllerLegacyImpl() override;

  AdblockControllerLegacyImpl(const AdblockControllerLegacyImpl&) = delete;
  AdblockControllerLegacyImpl& operator=(const AdblockControllerLegacyImpl&) =
      delete;
  AdblockControllerLegacyImpl(AdblockControllerLegacyImpl&&) = delete;
  AdblockControllerLegacyImpl& operator=(AdblockControllerLegacyImpl&&) =
      delete;

  void SetAdblockEnabled(bool enabled) override;
  void SetAcceptableAdsEnabled(bool enabled) override;

  void SelectBuiltInSubscription(const GURL& url) override;
  void UnselectBuiltInSubscription(const GURL& url) override;

  void AddCustomSubscription(const GURL& url) override;
  void RemoveCustomSubscription(const GURL& url) override;

  void InstallSubscription(const GURL& url) override;
  void UninstallSubscription(const GURL& url) override;

  void AddAllowedDomain(const std::string& domain) override;
  void RemoveAllowedDomain(const std::string& domain) override;

  void AddCustomFilter(const std::string& filter) override;
  void RemoveCustomFilter(const std::string& filter) override;

  void ReadStateFromPrefs();

 protected:
  void OnPrefChangedAdblockEnabled();
  void OnPrefChangedSubscriptions();
  void OnPrefChangedAllowedDomains();
  void OnPrefChangedCustomFilters();
  bool IsKnownSubscription(const GURL& subscription_url) const;
  void UpdateSubscriptionPrefs();
  BooleanPrefMember adblock_enabled_;
  BooleanPrefMember aa_enabled_;
  StringListPrefMember custom_subscriptions_;
  StringListPrefMember subscriptions_;
  StringListPrefMember allowed_domains_;
  StringListPrefMember custom_filters_;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_ADBLOCK_CONTROLLER_LEGACY_IMPL_H_
