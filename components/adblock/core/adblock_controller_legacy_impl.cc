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

#include "base/command_line.h"
#include "base/logging.h"
#include "base/ranges/algorithm.h"
#include "components/adblock/core/adblock_controller_impl.h"
#include "components/adblock/core/adblock_switches.h"
#include "components/adblock/core/common/adblock_prefs.h"
#include "components/adblock/core/subscription/subscription_config.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "url/gurl.h"

namespace adblock {
namespace {

GURL GurlFromString(const std::string& string) {
  return GURL(string);
}

}  // namespace

AdblockControllerLegacyImpl::AdblockControllerLegacyImpl(
    PrefService* pref_service,
    FilteringConfiguration* adblock_filtering_configuration,
    SubscriptionService* subscription_service,
    const std::string& locale,
    std::vector<KnownSubscriptionInfo> known_subscriptions)
    : AdblockControllerImpl(adblock_filtering_configuration,
                            subscription_service,
                            locale,
                            std::move(known_subscriptions)) {
  adblock_enabled_.Init(
      prefs::kEnableAdblockLegacy, pref_service,
      base::BindRepeating(
          &AdblockControllerLegacyImpl::OnPrefChangedAdblockEnabled,
          base::Unretained(this)));
  aa_enabled_.Init(prefs::kEnableAcceptableAdsLegacy, pref_service,
                   base::BindRepeating(
                       &AdblockControllerLegacyImpl::OnPrefChangedSubscriptions,
                       base::Unretained(this)));
  custom_subscriptions_.Init(
      prefs::kAdblockCustomSubscriptionsLegacy, pref_service,
      base::BindRepeating(
          &AdblockControllerLegacyImpl::OnPrefChangedSubscriptions,
          base::Unretained(this)));
  subscriptions_.Init(
      prefs::kAdblockSubscriptionsLegacy, pref_service,
      base::BindRepeating(
          &AdblockControllerLegacyImpl::OnPrefChangedSubscriptions,
          base::Unretained(this)));
  allowed_domains_.Init(
      prefs::kAdblockAllowedDomainsLegacy, pref_service,
      base::BindRepeating(
          &AdblockControllerLegacyImpl::OnPrefChangedAllowedDomains,
          base::Unretained(this)));
  custom_filters_.Init(
      prefs::kAdblockCustomFiltersLegacy, pref_service,
      base::BindRepeating(
          &AdblockControllerLegacyImpl::OnPrefChangedCustomFilters,
          base::Unretained(this)));
}

AdblockControllerLegacyImpl::~AdblockControllerLegacyImpl() = default;

void AdblockControllerLegacyImpl::SetAdblockEnabled(bool enabled) {
  adblock_enabled_.SetValue(enabled);
  AdblockControllerImpl::SetAdblockEnabled(enabled);
}

void AdblockControllerLegacyImpl::SetAcceptableAdsEnabled(bool enabled) {
  aa_enabled_.SetValue(enabled);
  AdblockControllerImpl::SetAcceptableAdsEnabled(enabled);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::SelectBuiltInSubscription(const GURL& url) {
  AdblockControllerImpl::SelectBuiltInSubscription(url);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::UnselectBuiltInSubscription(const GURL& url) {
  AdblockControllerImpl::UnselectBuiltInSubscription(url);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::AddCustomSubscription(const GURL& url) {
  AdblockControllerImpl::AddCustomSubscription(url);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::RemoveCustomSubscription(const GURL& url) {
  AdblockControllerImpl::RemoveCustomSubscription(url);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::InstallSubscription(const GURL& url) {
  AdblockControllerImpl::InstallSubscription(url);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::UninstallSubscription(const GURL& url) {
  AdblockControllerImpl::UninstallSubscription(url);
  UpdateSubscriptionPrefs();
}

void AdblockControllerLegacyImpl::AddAllowedDomain(const std::string& domain) {
  AdblockControllerImpl::AddAllowedDomain(domain);
  allowed_domains_.SetValue(GetAllowedDomains());
}

void AdblockControllerLegacyImpl::RemoveAllowedDomain(
    const std::string& domain) {
  AdblockControllerImpl::RemoveAllowedDomain(domain);
  allowed_domains_.SetValue(GetAllowedDomains());
}

void AdblockControllerLegacyImpl::AddCustomFilter(const std::string& filter) {
  AdblockControllerImpl::AddCustomFilter(filter);
  custom_filters_.SetValue(GetCustomFilters());
}

void AdblockControllerLegacyImpl::RemoveCustomFilter(
    const std::string& filter) {
  AdblockControllerImpl::RemoveCustomFilter(filter);
  custom_filters_.SetValue(GetCustomFilters());
}

void AdblockControllerLegacyImpl::ReadStateFromPrefs() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableAdblock)) {
    adblock_enabled_.SetValue(false);
  }
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableAcceptableAds)) {
    aa_enabled_.SetValue(false);
  }
  OnPrefChangedAdblockEnabled();
  OnPrefChangedSubscriptions();
  OnPrefChangedAllowedDomains();
  OnPrefChangedCustomFilters();
}

void AdblockControllerLegacyImpl::OnPrefChangedAdblockEnabled() {
  AdblockControllerImpl::SetAdblockEnabled(adblock_enabled_.GetValue());
}

void AdblockControllerLegacyImpl::OnPrefChangedSubscriptions() {
  std::vector<GURL> subscriptions_in_prefs;
  base::ranges::transform(custom_subscriptions_.GetValue(),
                          std::back_inserter(subscriptions_in_prefs),
                          &GurlFromString);
  base::ranges::transform(subscriptions_.GetValue(),
                          std::back_inserter(subscriptions_in_prefs),
                          &GurlFromString);
  if (aa_enabled_.GetValue() &&
      !base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableAcceptableAds)) {
    subscriptions_in_prefs.push_back(AcceptableAdsUrl());
  } else {
    subscriptions_in_prefs.erase(
        base::ranges::remove(subscriptions_in_prefs, AcceptableAdsUrl()),
        subscriptions_in_prefs.end());
  }
  base::ranges::sort(subscriptions_in_prefs);
  subscriptions_in_prefs.erase(base::ranges::unique(subscriptions_in_prefs),
                               subscriptions_in_prefs.end());

  auto subscriptions_in_configuration =
      adblock_filtering_configuration_->GetFilterLists();
  base::ranges::sort(subscriptions_in_configuration);

  std::vector<GURL> subs_to_add;
  base::ranges::set_difference(subscriptions_in_prefs,
                               subscriptions_in_configuration,
                               std::back_inserter(subs_to_add));
  for (const auto& sub : subs_to_add)
    adblock_filtering_configuration_->AddFilterList(sub);

  std::vector<GURL> subs_to_remove;
  base::ranges::set_difference(subscriptions_in_configuration,
                               subscriptions_in_prefs,
                               std::back_inserter(subs_to_remove));
  for (const auto& sub : subs_to_remove)
    adblock_filtering_configuration_->RemoveFilterList(sub);
}

void AdblockControllerLegacyImpl::OnPrefChangedAllowedDomains() {
  auto domains_in_prefs = allowed_domains_.GetValue();
  base::ranges::sort(domains_in_prefs);
  domains_in_prefs.erase(base::ranges::unique(domains_in_prefs),
                         domains_in_prefs.end());
  auto domains_in_configuration =
      adblock_filtering_configuration_->GetAllowedDomains();
  base::ranges::sort(domains_in_configuration);

  std::vector<std::string> domains_to_add;
  base::ranges::set_difference(domains_in_prefs, domains_in_configuration,
                               std::back_inserter(domains_to_add));
  for (const auto& domain : domains_to_add)
    adblock_filtering_configuration_->AddAllowedDomain(domain);

  std::vector<std::string> domains_to_remove;
  base::ranges::set_difference(domains_in_configuration, domains_in_prefs,
                               std::back_inserter(domains_to_remove));
  for (const auto& domain : domains_to_remove)
    adblock_filtering_configuration_->RemoveAllowedDomain(domain);
}

void AdblockControllerLegacyImpl::OnPrefChangedCustomFilters() {
  auto filters_in_prefs = custom_filters_.GetValue();
  base::ranges::sort(filters_in_prefs);
  filters_in_prefs.erase(base::ranges::unique(filters_in_prefs),
                         filters_in_prefs.end());
  auto filters_in_configuration =
      adblock_filtering_configuration_->GetCustomFilters();
  base::ranges::sort(filters_in_configuration);

  std::vector<std::string> filters_to_add;
  base::ranges::set_difference(filters_in_prefs, filters_in_configuration,
                               std::back_inserter(filters_to_add));
  for (const auto& filter : filters_to_add)
    adblock_filtering_configuration_->AddCustomFilter(filter);

  std::vector<std::string> filters_to_remove;
  base::ranges::set_difference(filters_in_configuration, filters_in_prefs,
                               std::back_inserter(filters_to_remove));
  for (const auto& filter : filters_to_remove)
    adblock_filtering_configuration_->RemoveCustomFilter(filter);
}

bool AdblockControllerLegacyImpl::IsKnownSubscription(
    const GURL& subscription_url) const {
  return base::ranges::any_of(
      known_subscriptions_, [&](const auto& known_subscription) {
        return known_subscription.url == subscription_url;
      });
}

void AdblockControllerLegacyImpl::UpdateSubscriptionPrefs() {
  std::vector<std::string> subscriptions;
  std::vector<std::string> custom_subscriptions;
  for (const auto& url : adblock_filtering_configuration_->GetFilterLists()) {
    if (url == AcceptableAdsUrl())
      continue;
    if (IsKnownSubscription(url))
      subscriptions.push_back(url.spec());
    else
      custom_subscriptions.push_back(url.spec());
  }
  subscriptions_.SetValue(subscriptions);
  custom_subscriptions_.SetValue(custom_subscriptions);
}

}  // namespace adblock
