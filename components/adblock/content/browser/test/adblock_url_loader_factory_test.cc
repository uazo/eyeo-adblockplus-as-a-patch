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

#include "components/adblock/content/browser/adblock_url_loader_factory.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/run_loop.h"
#include "base/test/mock_callback.h"
#include "components/adblock/content/browser/test/mock_adblock_content_security_policy_injector.h"
#include "components/adblock/content/browser/test/mock_element_hider.h"
#include "components/adblock/content/browser/test/mock_resource_classification_runner.h"
#include "components/adblock/core/subscription/subscription_service.h"
#include "components/adblock/core/subscription/test/mock_subscription_collection.h"
#include "components/adblock/core/subscription/test/mock_subscription_service.h"
#include "components/adblock/core/test/mock_sitekey_storage.h"
#include "content/public/test/browser_task_environment.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/test/test_url_loader_factory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

class TestURLLoaderFactory : public adblock::AdblockURLLoaderFactory {
 public:
  TestURLLoaderFactory(
      adblock::AdblockURLLoaderFactoryConfig config,
      int32_t render_process_id,
      int frame_tree_node_id,
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver,
      mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory,
      std::string user_agent_string,
      DisconnectCallback on_disconnect)
      : adblock::AdblockURLLoaderFactory(std::move(config),
                                         render_process_id,
                                         frame_tree_node_id,
                                         std::move(receiver),
                                         std::move(target_factory),
                                         user_agent_string,
                                         std::move(on_disconnect)) {}

  bool CheckRenderProcessValid() const override { return true; }
  bool CheckRenderProcessAndFrameValid(
      int32_t /*render_frame_id*/) const override {
    return true;
  }
};

const std::string kTestUserAgent = "test-user-agent";

struct RequestFlow {
  GURL url{"https://test.com"};
  adblock::FilterMatchResult request_match =
      adblock::FilterMatchResult::kAllowRule;
  adblock::FilterMatchResult response_match =
      adblock::FilterMatchResult::kAllowRule;
  absl::optional<GURL> rewrite_url{};
  bool subscription_service_ready = true;
  bool element_hidable = true;

  bool IsRequestAllowed() const {
    return request_match != adblock::FilterMatchResult::kBlockRule;
  }

  bool IsResponseAllowed() const {
    return IsRequestAllowed() &&
           response_match != adblock::FilterMatchResult::kBlockRule;
  }

  const GURL& ActualUrl() const {
    return rewrite_url.has_value() ? rewrite_url.value() : url;
  }

  void ConfigureClassifier(adblock::MockResourceClassificationRunner& service) {
    EXPECT_CALL(service, CheckRewriteFilterMatch(testing::_, url, testing::_,
                                                 testing::_))
        .WillOnce(
            [&](auto, const GURL&, content::GlobalRenderFrameHostId,
                base::OnceCallback<void(const absl::optional<GURL>&)> cb) {
              std::move(cb).Run(rewrite_url);
            });
    EXPECT_CALL(service,
                CheckRequestFilterMatch(testing::_, ActualUrl(), testing::_,
                                        testing::_, testing::_))
        .WillOnce([&](auto, const GURL&, int32_t,
                      content::GlobalRenderFrameHostId,
                      adblock::CheckFilterMatchCallback cb) {
          std::move(cb).Run(request_match);
        });
    if (IsRequestAllowed()) {
      EXPECT_CALL(service,
                  CheckResponseFilterMatch(testing::_, ActualUrl(), testing::_,
                                           testing::_, testing::_))
          .WillOnce([&](auto, const GURL&, content::GlobalRenderFrameHostId,
                        const auto&, adblock::CheckFilterMatchCallback cb) {
            std::move(cb).Run(response_match);
          });
    } else {
      EXPECT_CALL(service,
                  CheckResponseFilterMatch(testing::_, ActualUrl(), testing::_,
                                           testing::_, testing::_))
          .Times(0);
    }
  }

  void ConfigureElementHider(adblock::MockElementHider& service) {
    if (!IsRequestAllowed() || !IsResponseAllowed()) {
      EXPECT_CALL(service, IsElementTypeHideable(testing::_))
          .WillOnce(testing::Return(element_hidable));
      EXPECT_CALL(service, HideBlockedElement(url, testing::_))
          .Times(element_hidable ? 1 : 0);
    } else {
      EXPECT_CALL(service, IsElementTypeHideable(testing::_)).Times(0);
      EXPECT_CALL(service, HideBlockedElement(url, testing::_)).Times(0);
    }
  }

  void ConfigureSitekeyStorage(adblock::MockSitekeyStorage& service) {
    EXPECT_CALL(service,
                ProcessResponseHeaders(ActualUrl(), testing::_, kTestUserAgent))
        .Times(IsResponseAllowed() ? 1 : 0);
  }

  void ConfigureCspInjector(
      adblock::MockAdblockContentSecurityPolicyInjector& service) {
    if (IsResponseAllowed()) {
      EXPECT_CALL(service, InsertContentSecurityPolicyHeadersIfApplicable(
                               ActualUrl(), testing::_, testing::_, testing::_))
          .WillOnce(
              [&](const GURL&, auto, auto,
                  adblock::InsertContentSecurityPolicyHeadersCallback cb) {
                std::move(cb).Run(nullptr);
              });
    } else {
      EXPECT_CALL(service, InsertContentSecurityPolicyHeadersIfApplicable(
                               ActualUrl(), testing::_, testing::_, testing::_))
          .Times(0);
    }
  }
};

class AdblockURLLoaderFactoryTest : public testing::Test {
 public:
  AdblockURLLoaderFactoryTest() : test_factory_receiver_(&test_factory_) {}

  AdblockURLLoaderFactoryTest(const AdblockURLLoaderFactoryTest&) = delete;
  AdblockURLLoaderFactoryTest& operator=(const AdblockURLLoaderFactoryTest&) =
      delete;

  void StartRequest(RequestFlow flow) {
    auto request = std::make_unique<network::ResourceRequest>();
    request->url = flow.url;

    EXPECT_CALL(subscription_service_, GetStatus())
        .WillRepeatedly(
            testing::Return(flow.subscription_service_ready
                                ? adblock::FilteringStatus::Active
                                : adblock::FilteringStatus::Initializing));
    EXPECT_CALL(subscription_service_, RunWhenInitialized(testing::_))
        .WillRepeatedly([&](base::OnceClosure task) {
          deferred_tasks_.push_back(std::move(task));
        });

    EXPECT_CALL(subscription_service_, GetCurrentSnapshot())
        .WillRepeatedly([]() {
          adblock::SubscriptionService::Snapshot snapshot;
          auto collection =
              std::make_unique<adblock::MockSubscriptionCollection>();
          // TODO(mpawlowski) will the collection be queried for classification?
          // If yes, add EXPECT_CALL(collection, ...) here.
          snapshot.push_back(std::move(collection));
          return snapshot;
        });

    flow.ConfigureClassifier(resource_classifier_);
    flow.ConfigureElementHider(element_hider_);
    flow.ConfigureSitekeyStorage(sitekey_storage_);
    flow.ConfigureCspInjector(csp_injector_);
    loader_ = network::SimpleURLLoader::Create(std::move(request),
                                               TRAFFIC_ANNOTATION_FOR_TESTS);
    mojo::Remote<network::mojom::URLLoaderFactory> factory_remote;
    auto factory_request = factory_remote.BindNewPipeAndPassReceiver();
    loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
        factory_remote.get(), base::DoNothing());

    adblock_factory_ = std::make_unique<TestURLLoaderFactory>(
        adblock::AdblockURLLoaderFactoryConfig{
            &subscription_service_, &resource_classifier_, &element_hider_,
            &sitekey_storage_, &csp_injector_},
        0, 0, std::move(factory_request),
        test_factory_receiver_.BindNewPipeAndPassRemote(), kTestUserAgent,
        base::BindOnce(&AdblockURLLoaderFactoryTest::OnDisconnect,
                       base::Unretained(this)));

    test_factory_.AddResponse(flow.url.spec(), "Hello.");
    base::RunLoop().RunUntilIdle();
  }

  void StartSubscriptionService() {
    EXPECT_CALL(subscription_service_, GetStatus())
        .WillRepeatedly(testing::Return(adblock::FilteringStatus::Active));
    for (auto& task : deferred_tasks_) {
      std::move(task).Run();
    }
    deferred_tasks_.clear();
    base::RunLoop().RunUntilIdle();
  }

  void OnDisconnect(adblock::AdblockURLLoaderFactory* factory) {
    EXPECT_EQ(factory, adblock_factory_.get());
    adblock_factory_.reset();
  }

  content::BrowserTaskEnvironment task_environment_;
  std::unique_ptr<network::SimpleURLLoader> loader_;
  std::unique_ptr<TestURLLoaderFactory> adblock_factory_;
  network::TestURLLoaderFactory test_factory_;
  mojo::Receiver<network::mojom::URLLoaderFactory> test_factory_receiver_;
  adblock::MockSubscriptionService subscription_service_;
  adblock::MockResourceClassificationRunner resource_classifier_;
  adblock::MockElementHider element_hider_;
  adblock::MockSitekeyStorage sitekey_storage_;
  adblock::MockAdblockContentSecurityPolicyInjector csp_injector_;
  std::vector<base::OnceClosure> deferred_tasks_;
};

TEST_F(AdblockURLLoaderFactoryTest, HappyPath) {
  StartRequest({});
  EXPECT_EQ(net::OK, loader_->NetError());
}

TEST_F(AdblockURLLoaderFactoryTest, WaitForInitialize) {
  StartRequest({.subscription_service_ready = false});
  StartSubscriptionService();
  EXPECT_EQ(net::OK, loader_->NetError());
}

TEST_F(AdblockURLLoaderFactoryTest, BlockedWithRequestFilter) {
  StartRequest({.request_match = adblock::FilterMatchResult::kBlockRule});
  EXPECT_EQ(net::ERR_BLOCKED_BY_ADMINISTRATOR, loader_->NetError());
}

TEST_F(AdblockURLLoaderFactoryTest, BlockedWithResponseFilter) {
  StartRequest({.response_match = adblock::FilterMatchResult::kBlockRule});
  EXPECT_EQ(net::ERR_BLOCKED_BY_ADMINISTRATOR, loader_->NetError());
}

TEST_F(AdblockURLLoaderFactoryTest, BlockedWithRequestFilterNonHideable) {
  StartRequest({.request_match = adblock::FilterMatchResult::kBlockRule,
                .element_hidable = false});
  EXPECT_EQ(net::ERR_BLOCKED_BY_ADMINISTRATOR, loader_->NetError());
}

TEST_F(AdblockURLLoaderFactoryTest, BlockedWithResponseFilterNonHideable) {
  StartRequest({.response_match = adblock::FilterMatchResult::kBlockRule,
                .element_hidable = false});
  EXPECT_EQ(net::ERR_BLOCKED_BY_ADMINISTRATOR, loader_->NetError());
}

// TODO: Rewrite will do redirect, it is required more tinkering for
// TestURLLoaderFactory to handle this.
/*
TEST_F(AdblockURLLoaderFactoryTest, RewriteUrl) {
  StartRequest({.rewrite_url = absl::optional<GURL>(GURL("http://other.com"))});
  EXPECT_EQ(net::OK, loader_->NetError());
}
*/

/*
 TODO:
 - Simulate CheckRenderProcessValid and CheckRenderProcessAndFrameValid is false
   on various steps.
 - Simulate SubscriptionService is not initialized in the middle of flow.
*/
