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

#ifndef COMPONENTS_ADBLOCK_CONTENT_BROWSER_TEST_MOCK_RESOURCE_CLASSIFICATION_RUNNER_
#define COMPONENTS_ADBLOCK_CONTENT_BROWSER_TEST_MOCK_RESOURCE_CLASSIFICATION_RUNNER_

#include "components/adblock/content/browser/resource_classification_runner.h"
#include "content/public/browser/render_frame_host.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "url/gurl.h"

namespace adblock {

class MockResourceClassificationRunner : public ResourceClassificationRunner {
 public:
  MockResourceClassificationRunner();
  ~MockResourceClassificationRunner() override;

  void AddObserver(ResourceClassificationRunner::Observer*) override;
  void RemoveObserver(ResourceClassificationRunner::Observer*) override;
  void NotifyAdMatched(const GURL& url,
                       FilterMatchResult result,
                       const std::vector<GURL>& parent_frame_urls,
                       ContentType content_type,
                       content::RenderFrameHost* render_frame_host,
                       const GURL& subscription);

  MOCK_METHOD((FilterMatchResult),
              ShouldBlockPopup,
              (const SubscriptionService::Snapshot&,
               const GURL&,
               content::RenderFrameHost*),
              (override));

  MOCK_METHOD(void,
              CheckRequestFilterMatch,
              (const SubscriptionService::Snapshot,
               const GURL&,
               ContentType,
               content::GlobalRenderFrameHostId,
               CheckFilterMatchCallback),
              (override));

  MOCK_METHOD(void,
              CheckRequestFilterMatchForWebSocket,
              (const SubscriptionService::Snapshot,
               const GURL&,
               content::GlobalRenderFrameHostId,
               CheckFilterMatchCallback),
              (override));

  MOCK_METHOD(void,
              CheckDocumentAllowlisted,
              (SubscriptionService::Snapshot,
               const GURL& request_url,
               content::GlobalRenderFrameHostId render_frame_host_id),
              (override));

  MOCK_METHOD(void,
              CheckResponseFilterMatch,
              (const SubscriptionService::Snapshot,
               const GURL&,
               ContentType,
               content::GlobalRenderFrameHostId,
               const scoped_refptr<net::HttpResponseHeaders>&,
               CheckFilterMatchCallback),
              (override));

  MOCK_METHOD(void,
              CheckRewriteFilterMatch,
              (const SubscriptionService::Snapshot,
               const GURL&,
               content::GlobalRenderFrameHostId,
               base::OnceCallback<void(const absl::optional<GURL>&)>),
              (override));

  base::ObserverList<ResourceClassificationRunner::Observer> observers_;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CONTENT_BROWSER_TEST_MOCK_RESOURCE_CLASSIFICATION_RUNNER_
