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

#ifndef COMPONENTS_ADBLOCK_CORE_ADBLOCK_CONTROLLER_H_
#define COMPONENTS_ADBLOCK_CORE_ADBLOCK_CONTROLLER_H_

#include <string>
#include <vector>

#include "base/observer_list.h"
#include "components/adblock/core/subscription/installed_subscription.h"
#include "components/adblock/core/subscription/subscription_config.h"
#include "components/keyed_service/core/keyed_service.h"

class GURL;

namespace adblock {
/**
 * @brief Provides the way for the UI to interact with the filter engine.
 * It allows to set the states of the ad-block and acceptable ads and
 * adding, removing and listing subscriptions and allowed domains.
 */
class AdblockController : public KeyedService {
 public:
  // Deprecated, to be removed in version 115
  // [deprecated="Use FilteringConfiguration::Observer instead"]
  class Observer : public base::CheckedObserver {
   public:
    virtual void OnSubscriptionUpdated(const GURL& url) {}
    virtual void OnEnabledStateChanged() {}
  };
  ~AdblockController() override = default;

  // Deprecated, to be removed in version 115
  // [deprecated="Use FilteringConfiguration::AddObserver instead"]
  virtual void AddObserver(Observer* observer) = 0;
  // Deprecated, to be removed in version 115
  // [deprecated="Use FilteringConfiguration::RemoveObserver instead"]
  virtual void RemoveObserver(Observer* observer) = 0;

  virtual void SetAdblockEnabled(bool enabled) = 0;
  virtual bool IsAdblockEnabled() const = 0;
  virtual void SetAcceptableAdsEnabled(bool enabled) = 0;
  virtual bool IsAcceptableAdsEnabled() const = 0;

  virtual void InstallSubscription(const GURL& url) = 0;
  virtual void UninstallSubscription(const GURL& url) = 0;
  virtual std::vector<scoped_refptr<Subscription>> GetInstalledSubscriptions()
      const = 0;

  virtual void AddAllowedDomain(const std::string& domain) = 0;
  virtual void RemoveAllowedDomain(const std::string& domain) = 0;
  virtual std::vector<std::string> GetAllowedDomains() const = 0;

  virtual void AddCustomFilter(const std::string& filter) = 0;
  virtual void RemoveCustomFilter(const std::string& filter) = 0;
  virtual std::vector<std::string> GetCustomFilters() const = 0;

  virtual std::vector<KnownSubscriptionInfo> GetKnownSubscriptions() const = 0;

  // Deprecated, SelectBuiltInSubscription will be removed in version 115
  // Use InstallSubscription(const GURL& url) instead.
  virtual void SelectBuiltInSubscription(const GURL& url) = 0;
  // Deprecated, UnselectBuiltInSubscription will be removed in version 115
  // Use UninstallSubscription(const GURL& url) instead.
  virtual void UnselectBuiltInSubscription(const GURL& url) = 0;
  // Deprecated, GetSelectedBuiltInSubscriptions will be removed in version 115
  // Use GetInstalledSubscriptions() instead.
  virtual std::vector<scoped_refptr<Subscription>>
  GetSelectedBuiltInSubscriptions() const = 0;

  // Deprecated, AddCustomSubscription will be removed in version 115
  // Use InstallSubscription(const GURL& url) instead.
  virtual void AddCustomSubscription(const GURL& url) = 0;
  // Deprecated, RemoveCustomSubscription will be removed in version 115
  // Use UninstallSubscription(const GURL& url) instead.
  virtual void RemoveCustomSubscription(const GURL& url) = 0;
  // Deprecated, GetCustomSubscriptions will be removed in version 115
  // Use GetInstalledSubscriptions() instead.
  virtual std::vector<scoped_refptr<Subscription>> GetCustomSubscriptions()
      const = 0;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_ADBLOCK_CONTROLLER_H_
