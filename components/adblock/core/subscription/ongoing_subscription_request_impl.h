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

#ifndef COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_ONGOING_SUBSCRIPTION_REQUEST_IMPL_H_
#define COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_ONGOING_SUBSCRIPTION_REQUEST_IMPL_H_

#include <string>

#include "base/callback.h"

#include "base/files/file_path.h"
#include "base/strings/string_piece_forward.h"
#include "base/timer/timer.h"
#include "components/adblock/core/subscription/ongoing_subscription_request.h"
#include "components/adblock/core/subscription/subscription_downloader.h"
#include "components/prefs/pref_member.h"
#include "net/base/backoff_entry.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace adblock {

class OngoingSubscriptionRequestImpl final : public OngoingSubscriptionRequest {
 public:
  // TODO(mpawlowski): This still observes prefs::kEnableAdblockLegacy in
  // |prefs|. This is no longer a valid way to become notified of needing to
  // stop the download. Address in DPD-1568.
  OngoingSubscriptionRequestImpl(
      PrefService* prefs,
      const net::BackoffEntry::Policy* backoff_policy,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
  ~OngoingSubscriptionRequestImpl() final;
  void Start(GURL url, Method method, ResponseCallback response_callback) final;
  void Retry() final;
  void Redirect(GURL redirect_url) final;

  size_t GetNumberOfRedirects() const final;

 private:
  bool IsStarted() const;
  bool IsConnectionAllowed() const;
  void CheckConnectionAllowed();
  void StartInternal();
  void StopInternal();
  void OnDownloadFinished(base::FilePath downloaded_file);
  void OnHeadersReceived(scoped_refptr<net::HttpResponseHeaders> headers);
  const char* MethodToString() const;

  SEQUENCE_CHECKER(sequence_checker_);
  BooleanPrefMember adblocking_enabled_;
  std::unique_ptr<net::BackoffEntry> backoff_entry_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  GURL url_;
  Method method_ = Method::GET;
  ResponseCallback response_callback_;
  std::unique_ptr<network::SimpleURLLoader> loader_;
  std::unique_ptr<base::OneShotTimer> retry_timer_;
  size_t number_of_redirects_;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_ONGOING_SUBSCRIPTION_REQUEST_IMPL_H_
