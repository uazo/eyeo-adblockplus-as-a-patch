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
#include "base/json/json_reader.h"
#include "chrome/browser/adblock/adblock_telemetry_service_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/adblock/core/activeping_telemetry_topic_provider.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"

namespace adblock {

class AdblockTelemetryServiceBrowserTestBase : public InProcessBrowserTest {
 public:
  // We need to set server and request handler asap
  void SetUpInProcessBrowserTestFixture() override {
    InProcessBrowserTest::SetUpInProcessBrowserTestFixture();
    host_resolver()->AddRule(
        ActivepingTelemetryTopicProvider::DefaultBaseUrl().host(), "127.0.0.1");
    http_server_.RegisterRequestHandler(base::BindRepeating(
        &AdblockTelemetryServiceBrowserTestBase::RequestHandler,
        base::Unretained(this)));
    ASSERT_TRUE(http_server_.Start());
    ActivepingTelemetryTopicProvider::SetHttpPortForTesting(
        http_server_.port());
    auto testing_interval = base::Seconds(2);
    ActivepingTelemetryTopicProvider::SetIntervalsForTesting(testing_interval);
    AdblockTelemetryServiceFactory::GetInstance()
        ->SetCheckAndDelayIntervalsForTesting(testing_interval,
                                              testing_interval);
  }

  base::Value* GetFirstPing(absl::optional<base::Value>& parsed) {
    return GetPayload(parsed)->Find("first_ping");
  }

  base::Value* GetLastPing(absl::optional<base::Value>& parsed) {
    return GetPayload(parsed)->Find("last_ping");
  }

  base::Value* GetLastPingTag(absl::optional<base::Value>& parsed) {
    return GetPayload(parsed)->Find("last_ping_tag");
  }

  base::Value* GetPreviousLastPing(absl::optional<base::Value>& parsed) {
    return GetPayload(parsed)->Find("previous_last_ping");
  }

  void CloseBrowserFromAnyThread() {
    content::GetUIThreadTaskRunner({base::TaskPriority::USER_BLOCKING})
        ->PostTask(FROM_HERE,
                   base::BindOnce(&AdblockTelemetryServiceBrowserTestBase::
                                      CloseBrowserAsynchronously,
                                  base::Unretained(this), browser()));
  }

  std::unique_ptr<net::test_server::HttpResponse> CreateResponse(
      const std::string& token) {
    auto http_response =
        std::make_unique<net::test_server::BasicHttpResponse>();
    http_response->set_code(net::HTTP_OK);
    http_response->set_content("{\"token\": \"" + token + "\"}");
    http_response->set_content_type("text/plain");
    return std::move(http_response);
  }

  virtual std::unique_ptr<net::test_server::HttpResponse> RequestHandler(
      const net::test_server::HttpRequest& request) = 0;

 private:
  base::Value::Dict* GetPayload(absl::optional<base::Value>& parsed) {
    EXPECT_TRUE(parsed && parsed->is_dict());
    base::Value::Dict* parsed_dict = parsed->GetIfDict();
    EXPECT_TRUE(parsed_dict);
    base::Value::Dict* payload = parsed_dict->FindDict("payload");
    EXPECT_TRUE(payload);
    return payload;
  }

  net::EmbeddedTestServer http_server_;
};

// Test three initial pings each after startup and each fails for the 1st time
class AdblockTelemetryServiceFirstPingAfterRestartWithRetryBrowserTest
    : public AdblockTelemetryServiceBrowserTestBase {
 public:
  std::unique_ptr<net::test_server::HttpResponse> RequestHandler(
      const net::test_server::HttpRequest& request) override {
    EXPECT_TRUE(base::StartsWith(request.relative_url,
                                 "/topic/eyeochromium_activeping/version/1"));
    EXPECT_TRUE(request.has_content);
    absl::optional<base::Value> parsed =
        base::JSONReader::Read(request.content);
    base::Value* first_ping = GetFirstPing(parsed);
    base::Value* last_ping = GetLastPing(parsed);
    base::Value* previous_last_ping = GetPreviousLastPing(parsed);

    if (expected_first_ping_.empty()) {
      EXPECT_FALSE(first_ping);
    } else {
      EXPECT_EQ(expected_first_ping_, *first_ping);
    }

    if (expected_last_ping_.empty()) {
      EXPECT_FALSE(last_ping);
    } else {
      EXPECT_EQ(expected_last_ping_, *last_ping);
    }

    if (expected_previous_last_ping_.empty()) {
      EXPECT_FALSE(previous_last_ping);
    } else {
      EXPECT_EQ(expected_previous_last_ping_, *previous_last_ping);
    }

    if (!attempt_++) {
      // Force retry by 404 response but 1st save last_ping_tag if any
      base::Value* last_ping_tag = GetLastPingTag(parsed);
      if (last_ping_tag) {
        previous_last_ping_tag_ = last_ping_tag->GetString();
      }
      return nullptr;
    }

    // Verifies that retried ping has the same last_ping_tag
    if (!previous_last_ping_tag_.empty()) {
      base::Value* last_ping_tag = GetLastPingTag(parsed);
      EXPECT_EQ(previous_last_ping_tag_, *last_ping_tag);
    }

    // If we get expected telemetry ping with retry we simply finish the test
    // by closing the browser, otherwise test will fail with a timeout.
    CloseBrowserFromAnyThread();

    // This will be still processed before CloseBrowserFromAnyThread() applies
    return CreateResponse(server_response_);
  }

  void TearDownInProcessBrowserTestFixture() override {
    // Make sure we called RequestHandler exactly twice: 1st ping failed, 2nd
    // was successful
    EXPECT_EQ(2, attempt_);
  }

 protected:
  std::string server_response_;
  std::string expected_first_ping_;
  std::string expected_last_ping_;
  std::string expected_previous_last_ping_;

 private:
  int attempt_ = 0;
  std::string previous_last_ping_tag_ = "";
};

IN_PROC_BROWSER_TEST_F(
    AdblockTelemetryServiceFirstPingAfterRestartWithRetryBrowserTest,
    PRE_PRE_TestPing) {
  // Sets the ping to be returned by the server and checked in the next test
  server_response_ = "11111";
  RunUntilBrowserProcessQuits();
}

IN_PROC_BROWSER_TEST_F(
    AdblockTelemetryServiceFirstPingAfterRestartWithRetryBrowserTest,
    PRE_TestPing) {
  // Sets the ping to be returned by the server and checked in the next test
  server_response_ = "22222";
  expected_first_ping_ = expected_last_ping_ = "11111";
  RunUntilBrowserProcessQuits();
}

IN_PROC_BROWSER_TEST_F(
    AdblockTelemetryServiceFirstPingAfterRestartWithRetryBrowserTest,
    TestPing) {
  expected_first_ping_ = "11111";
  expected_last_ping_ = "22222";
  expected_previous_last_ping_ = "11111";
  RunUntilBrowserProcessQuits();
}

// Test three inital pings
class AdblockTelemetryServiceSubsequentPingsBrowserTest
    : public AdblockTelemetryServiceBrowserTestBase {
 public:
  std::unique_ptr<net::test_server::HttpResponse> RequestHandler(
      const net::test_server::HttpRequest& request) override {
    EXPECT_TRUE(base::StartsWith(request.relative_url,
                                 "/topic/eyeochromium_activeping/version/1"));
    EXPECT_TRUE(request.has_content);
    absl::optional<base::Value> parsed =
        base::JSONReader::Read(request.content);
    base::Value* first_ping = GetFirstPing(parsed);
    base::Value* last_ping = GetLastPing(parsed);
    base::Value* previous_last_ping = GetPreviousLastPing(parsed);

    if (count_ == 1) {
      // No ping payload in the very 1st ping
      EXPECT_FALSE(first_ping);
      EXPECT_FALSE(last_ping);
    } else if (count_ == 2) {
      // For 2nd ping `first_ping` == `last_ping`
      EXPECT_EQ(first_ping_, *first_ping);
      EXPECT_EQ(first_ping_, *last_ping);
    } else if (count_ == 3) {
      // From 3rd ping onward `first_ping` != `last_ping` and we also get
      // `previous_last_ping`
      EXPECT_EQ(first_ping_, *first_ping);
      EXPECT_EQ(second_ping_, *last_ping);
      EXPECT_EQ(first_ping_, *previous_last_ping);
    }

    // If we get three expected telemetry pings we simply finish the test by
    // closing the browser, otherwise test will fail with a timeout.
    if (count_ == 3) {
      CloseBrowserFromAnyThread();
      return nullptr;
    }
    return CreateResponse(count_++ == 1 ? first_ping_ : second_ping_);
  }

  void TearDownInProcessBrowserTestFixture() override {
    // Make sure we called RequestHandler exactly three times
    EXPECT_EQ(3, count_);
  }

 private:
  const std::string first_ping_ = "11111";
  const std::string second_ping_ = "22222";
  int count_ = 1;
};

IN_PROC_BROWSER_TEST_F(AdblockTelemetryServiceSubsequentPingsBrowserTest,
                       TestPing) {
  RunUntilBrowserProcessQuits();
}

}  // namespace adblock
