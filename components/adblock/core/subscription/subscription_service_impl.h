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

#ifndef COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_SUBSCRIPTION_SERVICE_IMPL_H_
#define COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_SUBSCRIPTION_SERVICE_IMPL_H_

#include <memory>
#include <set>
#include <vector>

#include "base/callback.h"

#include "base/functional/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/sequence_checker.h"
#include "components/adblock/core/configuration/filtering_configuration.h"
#include "components/adblock/core/subscription/installed_subscription.h"
#include "components/adblock/core/subscription/preloaded_subscription_provider.h"
#include "components/adblock/core/subscription/subscription_downloader.h"
#include "components/adblock/core/subscription/subscription_persistent_metadata.h"
#include "components/adblock/core/subscription/subscription_persistent_storage.h"
#include "components/adblock/core/subscription/subscription_service.h"
#include "components/adblock/core/subscription/subscription_updater.h"

namespace adblock {

class SubscriptionServiceImpl final : public SubscriptionService,
                                      public FilteringConfiguration::Observer {
 public:
  using SubscriptionCreator =
      base::RepeatingCallback<scoped_refptr<InstalledSubscription>(
          const std::vector<std::string>&)>;
  SubscriptionServiceImpl(
      std::unique_ptr<SubscriptionPersistentStorage> storage,
      std::unique_ptr<SubscriptionDownloader> downloader,
      std::unique_ptr<PreloadedSubscriptionProvider>
          preloaded_subscription_provider,
      std::unique_ptr<SubscriptionUpdater> updater,
      const SubscriptionCreator& custom_subscription_creator,
      SubscriptionPersistentMetadata* persistent_metadata);
  ~SubscriptionServiceImpl() final;

  // SubscriptionService:
  FilteringStatus GetStatus() const final;
  void RunWhenInitialized(base::OnceClosure task) final;
  std::vector<scoped_refptr<Subscription>> GetCurrentSubscriptions(
      FilteringConfiguration* configuration) const final;
  void InstallFilteringConfiguration(
      std::unique_ptr<FilteringConfiguration> configuration) final;
  Snapshot GetCurrentSnapshot() const final;
  void AddObserver(SubscriptionObserver*) final;
  void RemoveObserver(SubscriptionObserver*) final;

  // FilteringConfiguration::Observer:
  void OnEnabledStateChanged(FilteringConfiguration* config) final;
  void OnFilterListsChanged(FilteringConfiguration* config) final;
  void OnAllowedDomainsChanged(FilteringConfiguration* config) final;
  void OnCustomFiltersChanged(FilteringConfiguration* config) final;

 private:
  enum class StorageStatus {
    Initialized,
    LoadingSubscriptions,
    Uninitialized,
  };
  class OngoingInstallation;
  bool IsInitialized() const;
  void RunUpdateCheck();
  void DownloadAndInstallSubscription(const GURL& subscription_url);
  void PingAcceptableAds();
  void UninstallSubscription(const GURL& subscription_url);
  void SetCustomFilters();

  void OnSubscriptionDataAvailable(
      scoped_refptr<OngoingInstallation> ongoing_installation,
      std::unique_ptr<FlatbufferData> raw_data);
  void StorageInitialized(
      std::vector<scoped_refptr<InstalledSubscription>> loaded_subscriptions);
  void InitializeStorage();
  void RemoveDuplicateSubscriptions();
  void SubscriptionAddedToStorage(
      scoped_refptr<OngoingInstallation> ongoing_installation,
      scoped_refptr<InstalledSubscription> subscription);
  void OnHeadRequestDone(const std::string version);
  bool UninstallSubscriptionInternal(const GURL& subscription_url);
  void UpdatePreloadedSubscriptionProvider();
  std::vector<GURL> GetReadySubscriptions() const;
  std::vector<GURL> GetPendingSubscriptions() const;
  void InstallMissingSubscriptions();
  void RemoveUnneededSubscriptions();

  SEQUENCE_CHECKER(sequence_checker_);
  StorageStatus status_ = StorageStatus::Uninitialized;
  // Only one FilteringConfiguration supported currently. Support multiple
  // FilteringConfigurations with DPD-1568.
  std::unique_ptr<FilteringConfiguration> configuration_;
  std::vector<base::OnceClosure> queued_tasks_;
  std::unique_ptr<SubscriptionPersistentStorage> storage_;
  std::unique_ptr<SubscriptionDownloader> downloader_;
  std::unique_ptr<PreloadedSubscriptionProvider>
      preloaded_subscription_provider_;
  std::unique_ptr<SubscriptionUpdater> updater_;
  std::set<scoped_refptr<OngoingInstallation>> ongoing_installations_;
  std::vector<scoped_refptr<InstalledSubscription>> current_state_;
  scoped_refptr<InstalledSubscription> custom_filters_;
  SubscriptionCreator custom_subscription_creator_;
  // TODO(mpawlowski): Should not need to update metadata after DPD-1154, when
  // HEAD requests are removed. Move all use of SubscriptionPersistentMetadata
  // into SubscriptionPersistentStorage.
  SubscriptionPersistentMetadata* persistent_metadata_;
  base::ObserverList<SubscriptionObserver> observers_;
  base::WeakPtrFactory<SubscriptionServiceImpl> weak_ptr_factory_{this};
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_SUBSCRIPTION_SERVICE_IMPL_H_
