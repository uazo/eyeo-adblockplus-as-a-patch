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

#ifndef COMPONENTS_ADBLOCK_CONTENT_BROWSER_ADBLOCK_URL_LOADER_FACTORY_H_
#define COMPONENTS_ADBLOCK_CONTENT_BROWSER_ADBLOCK_URL_LOADER_FACTORY_H_

#include "base/containers/unique_ptr_adapters.h"
#include "components/adblock/content/browser/adblock_filter_match.h"
#include "content/public/browser/global_routing_id.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace adblock {

class SubscriptionService;
class ResourceClassificationRunner;
class ElementHider;
class SitekeyStorage;
class ContentSecurityPolicyInjector;

struct AdblockURLLoaderFactoryConfig {
  SubscriptionService* subscription_service = nullptr;
  ResourceClassificationRunner* resource_classifier = nullptr;
  ElementHider* element_hider = nullptr;
  SitekeyStorage* sitekey_storage = nullptr;
  ContentSecurityPolicyInjector* csp_injector = nullptr;
};

// Processing network requests and responses.
class AdblockURLLoaderFactory : public network::mojom::URLLoaderFactory {
 public:
  using DisconnectCallback = base::OnceCallback<void(AdblockURLLoaderFactory*)>;

  AdblockURLLoaderFactory(
      AdblockURLLoaderFactoryConfig config,
      content::GlobalRenderFrameHostId host_id,
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver,
      mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory,
      std::string user_agent_string,
      DisconnectCallback on_disconnect);
  ~AdblockURLLoaderFactory() override;

  void CreateLoaderAndStart(
      ::mojo::PendingReceiver<::network::mojom::URLLoader> loader,
      int32_t request_id,
      uint32_t options,
      const ::network::ResourceRequest& request,
      ::mojo::PendingRemote<::network::mojom::URLLoaderClient> client,
      const ::net::MutableNetworkTrafficAnnotationTag& traffic_annotation)
      override;
  void Clone(::mojo::PendingReceiver<URLLoaderFactory> factory) override;

  virtual bool CheckHostValid() const;

 private:
  class InProgressRequest;
  friend class InProgressRequest;

  void OnTargetFactoryError();
  void OnProxyBindingError();
  void RemoveRequest(InProgressRequest* request);
  void MaybeDestroySelf();

  AdblockURLLoaderFactoryConfig config_;
  content::GlobalRenderFrameHostId host_id_;
  mojo::ReceiverSet<network::mojom::URLLoaderFactory> proxy_receivers_;
  std::set<std::unique_ptr<InProgressRequest>, base::UniquePtrComparator>
      requests_;
  mojo::Remote<network::mojom::URLLoaderFactory> target_factory_;
  const std::string user_agent_string_;
  DisconnectCallback on_disconnect_;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CONTENT_BROWSER_ADBLOCK_URL_LOADER_FACTORY_H_
