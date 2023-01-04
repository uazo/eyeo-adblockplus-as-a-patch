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

#include "chrome/browser/adblock/adblock_content_browser_client.h"

#include "base/callback_list.h"
#include "base/run_loop.h"
#include "base/test/mock_callback.h"
#include "chrome/browser/adblock/resource_classification_runner_factory.h"
#include "chrome/browser/adblock/subscription_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "components/adblock/content/browser/test/mock_resource_classification_runner.h"
#include "components/adblock/core/common/adblock_prefs.h"
#include "components/adblock/core/common/content_type.h"
#include "components/adblock/core/subscription/installed_subscription.h"
#include "components/adblock/core/subscription/subscription_service.h"
#include "components/adblock/core/subscription/test/mock_subscription_collection.h"
#include "components/adblock/core/subscription/test/mock_subscription_service.h"
#include "components/adblock/core/test/mock_adblock_controller.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/test/mock_render_process_host.h"
#include "content/public/test/test_renderer_host.h"
#include "gmock/gmock.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/websocket.mojom.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::Return;

namespace adblock {
namespace {

class DelayedInitSubscriptionService : public MockSubscriptionService {
 public:
  FilteringStatus GetStatus() const override { return status_; }
  void RunWhenInitialized(base::OnceClosure task) override {
    deferred_tasks_.AddUnsafe(std::move(task));
  }

  void InitializeNow() {
    status_ = FilteringStatus::Active;
    deferred_tasks_.Notify();
  }

  FilteringStatus status_ = FilteringStatus::Initializing;
  base::OnceClosureList deferred_tasks_;
};

}  // namespace

class AdblockContentBrowserClientUnitTest
    : public ChromeRenderViewHostTestHarness {
 public:
  TestingProfile::TestingFactories GetTestingFactories() const override {
    return {std::make_pair(
                SubscriptionServiceFactory::GetInstance(),
                base::BindRepeating([](content::BrowserContext* bc)
                                        -> std::unique_ptr<KeyedService> {
                  return std::make_unique<DelayedInitSubscriptionService>();
                })),
            std::make_pair(
                ResourceClassificationRunnerFactory::GetInstance(),
                base::BindRepeating([](content::BrowserContext* bc)
                                        -> std::unique_ptr<KeyedService> {
                  return std::make_unique<MockResourceClassificationRunner>();
                }))};
  }

  void SetUp() override {
    RenderViewHostTestHarness::SetUp();

    subscription_service_ = static_cast<DelayedInitSubscriptionService*>(
        SubscriptionServiceFactory::GetForBrowserContext(profile()));
    resource_classification_runner_ =
        static_cast<MockResourceClassificationRunner*>(
            ResourceClassificationRunnerFactory::GetForBrowserContext(
                profile()));
  }

  DelayedInitSubscriptionService* subscription_service_;
  MockResourceClassificationRunner* resource_classification_runner_;
};

TEST_F(AdblockContentBrowserClientUnitTest,
       WillInterceptWebSocketWhenFilteringActive) {
  AdblockContentBrowserClient content_client;
  subscription_service_->status_ = FilteringStatus::Inactive;
  EXPECT_FALSE(content_client.WillInterceptWebSocket(main_rfh()));
  subscription_service_->status_ = FilteringStatus::Initializing;
  EXPECT_TRUE(content_client.WillInterceptWebSocket(main_rfh()));
  subscription_service_->status_ = FilteringStatus::Active;
  EXPECT_TRUE(content_client.WillInterceptWebSocket(main_rfh()));
}

TEST_F(AdblockContentBrowserClientUnitTest,
       RenderFrameHostDiesBeforeSubscriptionServiceInitialized) {
  AdblockContentBrowserClient content_client;

  base::MockCallback<content::ContentBrowserClient::WebSocketFactory>
      web_socket_factory;
  // The web_socket_factory callback will never be called because the associated
  // RenderFrameHost will be dead.
  EXPECT_CALL(web_socket_factory, Run(_, _, _, _, _)).Times(0);

  // In case the resource classification proceeds, let it finish to expose
  // potential use-after-free.
  ON_CALL(*subscription_service_, GetCurrentSnapshot()).WillByDefault([]() {
    SubscriptionService::Snapshot snapshot;
    snapshot.push_back(std::make_unique<MockSubscriptionCollection>());
    return snapshot;
  });

  ON_CALL(*resource_classification_runner_,
          CheckRequestFilterMatchForWebSocket(_, _, _, _))
      .WillByDefault([&](auto subscription_collection, const auto& url,
                         auto rfh_id, auto callback) {
        std::move(callback).Run(FilterMatchResult::kAllowRule);
      });

  const net::SiteForCookies site_for_cookies;
  content_client.CreateWebSocket(main_rfh(), web_socket_factory.Get(),
                                 GURL("wss://domain.com/test"),
                                 site_for_cookies, absl::nullopt, {});
  // Tab is closed.
  DeleteContents();

  // SubscriptionService becomes initialized, triggers deferred classification
  // of web socket. This does nothing.
  subscription_service_->InitializeNow();

  task_environment()->RunUntilIdle();
}

TEST_F(AdblockContentBrowserClientUnitTest,
       RenderFrameHostDiesBeforeClassificationFinished) {
  const auto kSocketUrl = GURL("wss://domain.com/test");
  // SubscriptionService is initialized from the start.
  subscription_service_->InitializeNow();
  // It will be queried for a SubscriptionCollection to classify the web socket
  // connection.
  EXPECT_CALL(*subscription_service_, GetCurrentSnapshot()).WillOnce([]() {
    SubscriptionService::Snapshot snapshot;
    snapshot.push_back(std::make_unique<MockSubscriptionCollection>());
    return snapshot;
  });
  // The SubscriptionCollection will be passed to ResourceClassificationRunner
  // to run the classification asynchronously. Save the callback to run it
  // later.
  CheckFilterMatchCallback classification_callback;
  EXPECT_CALL(*resource_classification_runner_,
              CheckRequestFilterMatchForWebSocket(_, kSocketUrl,
                                                  main_rfh()->GetGlobalId(), _))
      .WillOnce([&](auto subscription_collection, const auto& url, auto rfh_id,
                    auto callback) {
        classification_callback = std::move(callback);
      });

  AdblockContentBrowserClient content_client;
  base::MockCallback<content::ContentBrowserClient::WebSocketFactory>
      web_socket_factory;
  // The web_socket_factory callback will never be called because the associated
  // RenderFrameHost will be dead.
  EXPECT_CALL(web_socket_factory, Run(_, _, _, _, _)).Times(0);

  const net::SiteForCookies site_for_cookies;
  content_client.CreateWebSocket(main_rfh(), web_socket_factory.Get(),
                                 kSocketUrl, site_for_cookies, absl::nullopt,
                                 {});
  // Tab is closed.
  DeleteContents();

  // Classification finishes now. It will not trigger a call to
  // |web_socket_factory| because the RFH is dead.
  std::move(classification_callback).Run(FilterMatchResult::kBlockRule);

  task_environment()->RunUntilIdle();
}

TEST_F(AdblockContentBrowserClientUnitTest, WebSocketAllowed) {
  const auto kSocketUrl = GURL("wss://domain.com/test");
  // SubscriptionService is initialized from the start.
  subscription_service_->InitializeNow();
  // It will be queried for a SubscriptionCollection to classify the web socket
  // connection.
  EXPECT_CALL(*subscription_service_, GetCurrentSnapshot()).WillOnce([]() {
    SubscriptionService::Snapshot snapshot;
    snapshot.push_back(std::make_unique<MockSubscriptionCollection>());
    return snapshot;
  });
  // The SubscriptionCollection will be passed to ResourceClassificationRunner
  // to run the classification asynchronously. Save the callback to run it
  // later.
  CheckFilterMatchCallback classification_callback;
  EXPECT_CALL(*resource_classification_runner_,
              CheckRequestFilterMatchForWebSocket(_, kSocketUrl,
                                                  main_rfh()->GetGlobalId(), _))
      .WillOnce([&](auto subscription_collection, const auto& url, auto rfh_id,
                    auto callback) {
        classification_callback = std::move(callback);
      });

  AdblockContentBrowserClient content_client;
  base::MockCallback<content::ContentBrowserClient::WebSocketFactory>
      web_socket_factory;
  // The web_socket_factory callback will be called to let the web socket
  // continue connecting.
  EXPECT_CALL(web_socket_factory, Run(kSocketUrl, _, _, _, _));

  const net::SiteForCookies site_for_cookies;
  content_client.CreateWebSocket(main_rfh(), web_socket_factory.Get(),
                                 kSocketUrl, site_for_cookies, absl::nullopt,
                                 {});

  // Classification finishes now. It will trigger a call to |web_socket_factory|
  std::move(classification_callback).Run(FilterMatchResult::kAllowRule);

  task_environment()->RunUntilIdle();
}

TEST_F(AdblockContentBrowserClientUnitTest, WebSocketBlocked) {
  const auto kSocketUrl = GURL("wss://domain.com/test");
  // SubscriptionService is initialized from the start.
  subscription_service_->InitializeNow();
  // It will be queried for a SubscriptionCollection to classify the web socket
  // connection.
  EXPECT_CALL(*subscription_service_, GetCurrentSnapshot()).WillOnce([]() {
    SubscriptionService::Snapshot snapshot;
    snapshot.push_back(std::make_unique<MockSubscriptionCollection>());
    return snapshot;
  });
  // The SubscriptionCollection will be passed to ResourceClassificationRunner
  // to run the classification asynchronously. Save the callback to run it
  // later.
  CheckFilterMatchCallback classification_callback;
  EXPECT_CALL(*resource_classification_runner_,
              CheckRequestFilterMatchForWebSocket(_, kSocketUrl,
                                                  main_rfh()->GetGlobalId(), _))
      .WillOnce([&](auto subscription_collection, const auto& url, auto rfh_id,
                    auto callback) {
        classification_callback = std::move(callback);
      });

  AdblockContentBrowserClient content_client;
  base::MockCallback<content::ContentBrowserClient::WebSocketFactory>
      web_socket_factory;
  // The web_socket_factory callback will not be called as to disallow
  // connection.
  EXPECT_CALL(web_socket_factory, Run(kSocketUrl, _, _, _, _)).Times(0);

  const net::SiteForCookies site_for_cookies;
  content_client.CreateWebSocket(main_rfh(), web_socket_factory.Get(),
                                 kSocketUrl, site_for_cookies, absl::nullopt,
                                 {});

  // Classification finishes now.
  std::move(classification_callback).Run(FilterMatchResult::kBlockRule);

  task_environment()->RunUntilIdle();
}

}  // namespace adblock
