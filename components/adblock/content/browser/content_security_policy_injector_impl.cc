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

#include "components/adblock/content/browser/content_security_policy_injector_impl.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/task/thread_pool.h"
#include "base/sequence_checker.h"
#include "base/trace_event/trace_event.h"
#include "components/adblock/core/subscription/subscription_service.h"
#include "content/public/browser/network_service_instance.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace adblock {
namespace {

std::set<base::StringPiece> GetCspInjections(
    const SubscriptionService::Snapshot subscription_collections,
    const GURL request_url,
    const std::vector<GURL> frame_hierarchy_chain) {
  TRACE_EVENT1("eyeo", "GetCspInjection", "url", request_url.spec());
  std::set<base::StringPiece> injections;
  for (const auto& collection : subscription_collections) {
    const auto injection =
        collection->GetCspInjections(request_url, frame_hierarchy_chain);
    injections.insert(injection.begin(), injection.end());
  }
  if (!injections.empty()) {
    VLOG(1) << "[eyeo] Will attempt to inject CSP header/s "
            << " for " << request_url;
    DVLOG(2) << "[eyeo] CSP headers for " << request_url << ":";
    for (const auto& filter : injections) {
      DVLOG(2) << "[eyeo] " << filter;
    }
  }
  return injections;
}

}  // namespace

ContentSecurityPolicyInjectorImpl::ContentSecurityPolicyInjectorImpl(
    SubscriptionService* subscription_service,
    std::unique_ptr<FrameHierarchyBuilder> frame_hierarchy_builder)
    : subscription_service_(subscription_service),
      frame_hierarchy_builder_(std::move(frame_hierarchy_builder)) {}

ContentSecurityPolicyInjectorImpl::~ContentSecurityPolicyInjectorImpl() =
    default;

void ContentSecurityPolicyInjectorImpl::
    InsertContentSecurityPolicyHeadersIfApplicable(
        const GURL& request_url,
        content::GlobalRenderFrameHostId render_frame_host_id,
        const scoped_refptr<net::HttpResponseHeaders>& headers,
        InsertContentSecurityPolicyHeadersCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  content::RenderFrameHost* host =
      frame_hierarchy_builder_->FindRenderFrameHost(render_frame_host_id);
  if (host) {
    // GetCspInjection might take a while, let it run in the background.
    base::ThreadPool::PostTaskAndReplyWithResult(
        FROM_HERE, {},
        base::BindOnce(&GetCspInjections,
                       subscription_service_->GetCurrentSnapshot(), request_url,
                       frame_hierarchy_builder_->BuildFrameHierarchy(host)),
        base::BindOnce(
            &ContentSecurityPolicyInjectorImpl::OnCspInjectionsSearchFinished,
            weak_ptr_factory.GetWeakPtr(), request_url, std::move(headers),
            std::move(callback)));
  } else {
    // This request is likely dead, since there's no associated RenderFrameHost.
    // Post the callback to the current sequence to unblock any callers that
    // might be waiting for it.
    std::move(callback).Run(nullptr);
  }
}

void ContentSecurityPolicyInjectorImpl::OnCspInjectionsSearchFinished(
    const GURL request_url,
    const scoped_refptr<net::HttpResponseHeaders> headers,
    InsertContentSecurityPolicyHeadersCallback callback,
    std::set<base::StringPiece> csp_injections) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!csp_injections.empty()) {
    for (const auto& c_i : csp_injections) {
      // Set the CSP header according to |csp_injection|.
      headers->AddHeader("Content-Security-Policy", c_i);
    }
    // We need to ensure parsed headers match raw headers. Send the updated
    // raw headers to NetworkService for parsing.
    content::GetNetworkService()->ParseHeaders(
        request_url, headers,
        base::BindOnce(
            &ContentSecurityPolicyInjectorImpl::OnUpdatedHeadersParsed,
            weak_ptr_factory.GetWeakPtr(), std::move(callback)));
  } else {
    // No headers are injected, no need to update parsed headers.
    std::move(callback).Run(nullptr);
  }
}

void ContentSecurityPolicyInjectorImpl::OnUpdatedHeadersParsed(
    InsertContentSecurityPolicyHeadersCallback callback,
    network::mojom::ParsedHeadersPtr parsed_headers) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::move(callback).Run(std::move(parsed_headers));
}

}  // namespace adblock
