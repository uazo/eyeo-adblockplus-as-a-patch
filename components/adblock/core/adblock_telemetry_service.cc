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

#include "components/adblock/core/adblock_telemetry_service.h"

#include <string>

#include "base/functional/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "components/adblock/core/common/adblock_prefs.h"
#include "components/prefs/pref_service.h"
#include "net/base/load_flags.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace adblock {

namespace {

const char kDataType[] = "application/json";
net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("adblock_telemetry_request", R"(
      semantics {
        sender: "AdblockTelemetryService"
        description:
          "Messages sent to telemetry.eyeo.com to report usage statistics."
          "Contain no user-identifiable data."
        trigger:
          "Periodic, several times a day."
        data:
          "Subject to change: "
          "Dates of first ping, last ping and previous-to-last ping. "
          "A non-persistent, unique ID that disambiguates pings made in the "
          "same day. "
          "Application name and version (ex. Chromium 86.0.4240.183). "
          "Platform name and version (ex. Windows 10). "
          "Whether Acceptable Ads are in use (yes/no)."
        destination: WEBSITE
      }
      policy {
        cookies_allowed: NO
        setting:
          "Enabled or disabled via 'Ad blocking' setting."
        policy_exception_justification:
          "Parent setting may be controlled by policy"
        }
      })");

}  // namespace

// Represents an ongoing chain of requests relevant to a Topic.
// A Topic is and endpoint on the Telemetry server that expects messages
// about a domain of activity, ex. usage of Acceptable Ads or frequency of
// filter "hits" per filter list. The browser may report on multiple topics.
// Messages are sent periodically. The interval of communication and the
// content of the messages is provided by a TopicProvider.
class AdblockTelemetryService::Conversation {
 public:
  Conversation(
      std::unique_ptr<TopicProvider> topic_provider,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
      : topic_provider_(std::move(topic_provider)),
        url_loader_factory_(url_loader_factory) {}

  bool IsRequestDue() {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    const auto due_time = topic_provider_->GetTimeOfNextRequest();
    if (due_time > base::Time::Now()) {
      VLOG(1) << "[eyeo] Telemetry request for "
              << topic_provider_->GetEndpointURL()
              << " not due yet, should run at " << due_time;
      return false;
    }
    if (IsRequestInFlight()) {
      VLOG(1) << "[eyeo] Telemetry request for "
              << topic_provider_->GetEndpointURL() << " already in-flight";
      return false;
    }
    VLOG(1) << "[eyeo] Telemetry request for "
            << topic_provider_->GetEndpointURL() << " is due";
    return true;
  }

  void StartRequest() {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    VLOG(1) << "[eyeo] Telemetry request for "
            << topic_provider_->GetEndpointURL() << " starting now";
    topic_provider_->GetPayload(base::BindOnce(&Conversation::MakeRequest,
                                               weak_ptr_factory_.GetWeakPtr()));
  }

  void Stop() {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    url_loader_.reset();
  }

 private:
  bool IsRequestInFlight() {
    return url_loader_ != nullptr || weak_ptr_factory_.HasWeakPtrs();
  }

  void MakeRequest(std::string payload) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    auto request = std::make_unique<network::ResourceRequest>();
    request->url = topic_provider_->GetEndpointURL();
    VLOG(1) << "[eyeo] Sending request to: " << request->url;
    request->method = net::HttpRequestHeaders::kPostMethod;
    // The server expects authorization via a bearer token. The token may be
    // empty in testing builds.
    const auto auth_token = topic_provider_->GetAuthToken();
    if (!auth_token.empty()) {
      request->headers.SetHeader(net::HttpRequestHeaders::kAuthorization,
                                 "Bearer " + auth_token);
    }
    // Notify the server we're expecting a JSON response.
    request->headers.SetHeader(net::HttpRequestHeaders::kAccept, kDataType);
    // Disallow using cache - identical requests should be physically sent to
    // the server.
    request->load_flags = net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
    // Omitting credentials prevents cookies from being sent. The server does
    // not expect or parse cookies, but we want to be on the safe side,
    // privacy-wise.
    request->credentials_mode = network::mojom::CredentialsMode::kOmit;

    // If any url_loader_ existed previously, it will be overwritten and its
    // request will be cancelled.
    url_loader_ = network::SimpleURLLoader::Create(std::move(request),
                                                   kTrafficAnnotation);

    VLOG(2) << "[eyeo] Payload: " << payload;
    url_loader_->AttachStringForUpload(payload, kDataType);
    // The Telemetry server responds with a JSON that contains a description of
    // any potential error. We want to parse this JSON if possible, we're not
    // content with just an HTTP error code. Process the response content even
    // if the code is not 200.
    url_loader_->SetAllowHttpErrorResults(true);

    url_loader_->DownloadToString(
        url_loader_factory_.get(),
        base::BindOnce(&Conversation::OnResponseArrived,
                       base::Unretained(this)),
        network::SimpleURLLoader::kMaxBoundedStringDownloadSize - 1);
  }

  void OnResponseArrived(std::unique_ptr<std::string> server_response) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    topic_provider_->ParseResponse(std::move(server_response));
    url_loader_.reset();
  }

  SEQUENCE_CHECKER(sequence_checker_);
  std::unique_ptr<TopicProvider> topic_provider_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  std::unique_ptr<network::SimpleURLLoader> url_loader_;
  base::WeakPtrFactory<Conversation> weak_ptr_factory_{this};
};

AdblockTelemetryService::AdblockTelemetryService(
    FilteringConfiguration* filtering_configuration,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    base::TimeDelta initial_delay,
    base::TimeDelta check_interval)
    : adblock_filtering_configuration_(filtering_configuration),
      url_loader_factory_(url_loader_factory),
      initial_delay_(initial_delay),
      check_interval_(check_interval) {
  DCHECK(adblock_filtering_configuration_);
  adblock_filtering_configuration_->AddObserver(this);
}

AdblockTelemetryService::~AdblockTelemetryService() {
  DCHECK(adblock_filtering_configuration_);
  adblock_filtering_configuration_->RemoveObserver(this);
}

void AdblockTelemetryService::AddTopicProvider(
    std::unique_ptr<TopicProvider> topic_provider) {
  ongoing_conversations_.push_back(std::make_unique<Conversation>(
      std::move(topic_provider), url_loader_factory_));
}

void AdblockTelemetryService::Start() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  OnEnabledStateChangedInternal();
}

void AdblockTelemetryService::OnEnabledStateChanged(FilteringConfiguration*) {
  OnEnabledStateChangedInternal();
}

void AdblockTelemetryService::OnEnabledStateChangedInternal() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (adblock_filtering_configuration_->IsEnabled() && !timer_.IsRunning()) {
    VLOG(1) << "[eyeo] Starting periodic Telemetry requests";
    timer_.Start(FROM_HERE, initial_delay_,
                 base::BindRepeating(&AdblockTelemetryService::RunPeriodicCheck,
                                     base::Unretained(this)));
  } else if (!adblock_filtering_configuration_->IsEnabled() &&
             timer_.IsRunning()) {
    VLOG(1) << "[eyeo] Stopping periodic Telemetry requests";
    Shutdown();
  }
}

void AdblockTelemetryService::RunPeriodicCheck() {
  for (auto& conversation : ongoing_conversations_) {
    if (conversation->IsRequestDue()) {
      conversation->StartRequest();
    }
  }
  timer_.Start(FROM_HERE, check_interval_,
               base::BindRepeating(&AdblockTelemetryService::RunPeriodicCheck,
                                   base::Unretained(this)));
}

void AdblockTelemetryService::Shutdown() {
  timer_.Stop();
  for (auto& conversation : ongoing_conversations_) {
    conversation->Stop();
  }
}

}  // namespace adblock
