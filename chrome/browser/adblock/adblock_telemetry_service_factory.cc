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

#include "chrome/browser/adblock/adblock_telemetry_service_factory.h"

#include <memory>

#include "base/no_destructor.h"
#include "chrome/browser/adblock/adblock_controller_factory.h"
#include "chrome/browser/adblock/subscription_service_factory.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/adblock/core/activeping_telemetry_topic_provider.h"
#include "components/adblock/core/adblock_telemetry_service.h"
#include "components/adblock/core/common/adblock_utils.h"
#include "components/adblock/core/configuration/filtering_configuration.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/storage_partition.h"

namespace adblock {
namespace {
std::optional<base::TimeDelta> g_check_interval_for_testing;
std::optional<base::TimeDelta> g_initial_delay_for_testing;
constexpr base::StringPiece kAdblockFilteringConfigurationName = "adblock";

base::TimeDelta GetInitialDelay() {
  static base::TimeDelta kInitialDelay =
      g_initial_delay_for_testing ? g_initial_delay_for_testing.value()
                                  : base::Seconds(30);
  return kInitialDelay;
}

base::TimeDelta GetCheckInterval() {
  static base::TimeDelta kCheckInterval =
      g_check_interval_for_testing ? g_check_interval_for_testing.value()
                                   : base::Minutes(5);
  return kCheckInterval;
}

FilteringConfiguration* GetAdblockFilteringConfiguration(
    content::BrowserContext* context) {
  auto filtering_configurations =
      SubscriptionServiceFactory::GetForBrowserContext(context)
          ->GetInstalledFilteringConfigurations();
  const auto it = base::ranges::find(filtering_configurations,
                                     kAdblockFilteringConfigurationName,
                                     &FilteringConfiguration::GetName);
  DCHECK(it != filtering_configurations.end());
  return *it;
}
}  // namespace

// static
AdblockTelemetryService* AdblockTelemetryServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<AdblockTelemetryService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}
// static
AdblockTelemetryServiceFactory* AdblockTelemetryServiceFactory::GetInstance() {
  static base::NoDestructor<AdblockTelemetryServiceFactory> instance;
  return instance.get();
}

AdblockTelemetryServiceFactory::AdblockTelemetryServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "AdblockTelemetryService",
          BrowserContextDependencyManager::GetInstance()) {
  // AdblockController creates Adblock Filtering Configuration required by
  // AdblockTelemetryService
  DependsOn(AdblockControllerFactory::GetInstance());
}

AdblockTelemetryServiceFactory::~AdblockTelemetryServiceFactory() = default;

KeyedService* AdblockTelemetryServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  // Need to use a URLLoaderFactory specific to the browser context, not from
  // system_network_context_manager(), because the required Accept-Language
  // header depends on user's language settings and is not present in requests
  // made from the System network context.
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory =
      context->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess();
  auto* prefs = Profile::FromBrowserContext(context)->GetPrefs();
  auto service = std::make_unique<AdblockTelemetryService>(
      GetAdblockFilteringConfiguration(context), url_loader_factory,
      GetInitialDelay(), GetCheckInterval());
  service->AddTopicProvider(std::make_unique<ActivepingTelemetryTopicProvider>(
      utils::GetAppInfo(), prefs,
      AdblockControllerFactory::GetForBrowserContext(context),
      ActivepingTelemetryTopicProvider::DefaultBaseUrl(),
      ActivepingTelemetryTopicProvider::DefaultAuthToken()));

  if (url_loader_factory) {
    service->Start();
  }

  return service.release();
}

content::BrowserContext* AdblockTelemetryServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextRedirectedInIncognito(context);
}

bool AdblockTelemetryServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}

bool AdblockTelemetryServiceFactory::ServiceIsCreatedWithBrowserContext()
    const {
  return true;
}

void AdblockTelemetryServiceFactory::SetCheckAndDelayIntervalsForTesting(
    base::TimeDelta check_interval,
    base::TimeDelta initial_delay) {
  g_check_interval_for_testing = check_interval;
  g_initial_delay_for_testing = initial_delay;
}

}  // namespace adblock
