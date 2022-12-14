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

#include "components/adblock/core/subscription/installed_subscription_impl.h"

#include <memory>

#include "base/memory/scoped_refptr.h"
#include "components/adblock/core/common/flatbuffer_data.h"
#include "components/grit/components_resources.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace adblock {
namespace {

// NOTE! More tests of InstalledSubscriptionImpl are in converter_test.cc. The
// behavior of InstalledSubscriptionImpl is tightly coupled with the flatbuffer
// data generated by the Converter, so they're tested together. Here are tests
// that don't depend on the content of the flatbuffer data.

class MockBuffer : public InMemoryFlatbufferData {
 public:
  MockBuffer() : InMemoryFlatbufferData("[Adblock]") {}
  MOCK_METHOD(void, PermanentlyRemoveSourceOnDestruction, (), (override));
};

}  // namespace

TEST(AdblockInstalledSubscriptionImplTest,
     MarkForPermanentRemovalTriggersSourceRemoval) {
  auto buffer = std::make_unique<MockBuffer>();
  EXPECT_CALL(*buffer, PermanentlyRemoveSourceOnDestruction());
  auto subscription = base::MakeRefCounted<InstalledSubscriptionImpl>(
      std::move(buffer), Subscription::InstallationState::Installed,
      base::Time());
  subscription->MarkForPermanentRemoval();
}

TEST(AdblockInstalledSubscriptionImplTest,
     NormalDestructionDoesNotTriggerSourceRemoval) {
  auto buffer = std::make_unique<MockBuffer>();
  EXPECT_CALL(*buffer, PermanentlyRemoveSourceOnDestruction()).Times(0);
  auto subscription = base::MakeRefCounted<InstalledSubscriptionImpl>(
      std::move(buffer), Subscription::InstallationState::Installed,
      base::Time());
  subscription.reset();
}

TEST(AdblockInstalledSubscriptionImplTest, InstallationStateAndDateReported) {
  const auto installation_time =
      base::Time::FromDeltaSinceWindowsEpoch(base::Seconds(30));
  const auto installation_state = Subscription::InstallationState::Preloaded;
  auto subscription = base::MakeRefCounted<InstalledSubscriptionImpl>(
      std::make_unique<MockBuffer>(), installation_state, installation_time);
  EXPECT_EQ(subscription->GetInstallationState(), installation_state);
  EXPECT_EQ(subscription->GetInstallationTime(), installation_time);
}

}  // namespace adblock
