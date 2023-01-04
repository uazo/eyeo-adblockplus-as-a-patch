
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

#ifndef COMPONENTS_ADBLOCK_CORE_CONFIGURATION_PERSISTENT_FILTERING_CONFIGURATION_H_
#define COMPONENTS_ADBLOCK_CORE_CONFIGURATION_PERSISTENT_FILTERING_CONFIGURATION_H_

#include "base/observer_list.h"
#include "base/values.h"
#include "components/adblock/core/configuration/filtering_configuration.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"

namespace adblock {

// An implementation of FilteringConfiguration that persists itself to a dict
// inside PrefService.
//
// All instances live in the same root node in prefs but in serialize themselves
// to individual sub-keys based on their name.
class PersistentFilteringConfiguration final : public FilteringConfiguration {
 public:
  // Each |name| must be unique, otherwise multiple
  // PersistentFilteringConfigurations will try to serialize to the same path in
  // prefs and conflict with one another.
  PersistentFilteringConfiguration(PrefService* pref_service, std::string name);
  ~PersistentFilteringConfiguration() final;

  void AddObserver(Observer* observer) final;
  void RemoveObserver(Observer* observer) final;

  const std::string& GetName() const final;

  void SetEnabled(bool enabled) final;
  bool IsEnabled() const final;

  void AddFilterList(GURL url) final;
  void RemoveFilterList(GURL url) final;
  std::vector<GURL> GetFilterLists() const final;

  void AddAllowedDomain(std::string domain) final;
  void RemoveAllowedDomain(std::string domain) final;
  std::vector<std::string> GetAllowedDomains() const final;

  void AddCustomFilter(std::string filter) final;
  void RemoveCustomFilter(std::string filter) final;
  std::vector<std::string> GetCustomFilters() const final;

  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

 private:
  void PersistToPrefs();
  void NotifyEnabledStateChanged();
  void NotifyFilterListsChanged();
  void NotifyAllowedDomainsChanged();
  void NotifyCustomFiltersChanged();

  PrefService* pref_service_;
  std::string name_;
  base::ObserverList<Observer> observers_;
  base::Value dictionary_;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_CONFIGURATION_PERSISTENT_FILTERING_CONFIGURATION_H_
