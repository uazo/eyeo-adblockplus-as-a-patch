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

#ifndef COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_TEST_MOCK_SUBSCRIPTION_SERVICE_H_
#define COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_TEST_MOCK_SUBSCRIPTION_SERVICE_H_

#include "components/adblock/core/configuration/filtering_configuration.h"
#include "components/adblock/core/subscription/subscription_service.h"

#include "testing/gmock/include/gmock/gmock.h"

using testing::NiceMock;

namespace adblock {

class MockSubscriptionService : public NiceMock<SubscriptionService> {
 public:
  MockSubscriptionService();
  ~MockSubscriptionService() override;
  MOCK_METHOD(FilteringStatus, GetStatus, (), (override, const));
  MOCK_METHOD(void, RunWhenInitialized, (base::OnceClosure task), (override));
  MOCK_METHOD(std::vector<scoped_refptr<Subscription>>,
              GetCurrentSubscriptions,
              (FilteringConfiguration*),
              (override, const));
  MOCK_METHOD(Snapshot, GetCurrentSnapshot, (), (override, const));
  MOCK_METHOD(void,
              InstallFilteringConfiguration,
              (std::unique_ptr<FilteringConfiguration> configuration),
              (override));

  void AddObserver(SubscriptionObserver* observer) final;
  void RemoveObserver(SubscriptionObserver* observer) final;

  SubscriptionObserver* observer_ = nullptr;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_TEST_MOCK_SUBSCRIPTION_SERVICE_H_
