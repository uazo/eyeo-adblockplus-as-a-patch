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

#ifndef CHROME_BROWSER_ADBLOCK_ADBLOCK_TELEMETRY_SERVICE_FACTORY_H_
#define CHROME_BROWSER_ADBLOCK_ADBLOCK_TELEMETRY_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "base/time/time.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace adblock {
class AdblockTelemetryService;
class AdblockTelemetryServiceFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static AdblockTelemetryService* GetForProfile(Profile* profile);
  static AdblockTelemetryServiceFactory* GetInstance();

  // Sets the initial delay and interval checks required for browser tests.
  // Must be called before BuildServiceInstanceFor().
  void SetCheckAndDelayIntervalsForTesting(base::TimeDelta check_interval,
                                           base::TimeDelta initial_delay);

 private:
  friend class base::NoDestructor<AdblockTelemetryServiceFactory>;
  AdblockTelemetryServiceFactory();
  ~AdblockTelemetryServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
};

}  // namespace adblock

#endif  // CHROME_BROWSER_ADBLOCK_ADBLOCK_TELEMETRY_SERVICE_FACTORY_H_
