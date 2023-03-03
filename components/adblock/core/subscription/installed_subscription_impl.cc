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

#include <algorithm>
#include <iostream>
#include <iterator>

#include "absl/types/optional.h"
#include "base/logging.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_piece_forward.h"
#include "base/strings/string_util.h"
#include "base/trace_event/trace_event.h"
#include "components/adblock/core/common/adblock_constants.h"
#include "components/adblock/core/common/adblock_utils.h"
#include "components/adblock/core/common/flatbuffer_data.h"
#include "components/adblock/core/common/regex_filter_pattern.h"
#include "components/adblock/core/common/sitekey.h"
#include "components/adblock/core/subscription/domain_splitter.h"
#include "components/adblock/core/subscription/pattern_matcher.h"
#include "components/adblock/core/subscription/regex_matcher.h"
#include "components/adblock/core/subscription/subscription.h"
#include "components/adblock/core/subscription/url_keyword_extractor.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "url/url_constants.h"

namespace adblock {
namespace {

bool NeedsLowercasing(const std::string& input) {
  return base::ranges::any_of(
      input, [](const char c) { return base::IsAsciiUpper(c); });
}

bool IsThirdParty(const GURL& url, const std::string& domain) {
  return !net::registry_controlled_domains::SameDomainOrHost(
      url, GURL(url.scheme() + "://" + domain),
      net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

bool DomainMatches(base::StringPiece filter_domain,
                   base::StringPiece document_domain) {
  // document_domain is same as filter_domain:
  // - document: subdomain.example.com
  // - filter: subdomain.example.com
  // Or document_domain ends with ".filter_domain":
  // - document: subdomain.example.com
  // - filter: example.com
  // (document ends with ".example.com")

  return document_domain == filter_domain ||
         (base::EndsWith(document_domain, filter_domain) &&
          base::EndsWith(document_domain.substr(
                             0, document_domain.size() - filter_domain.size()),
                         "."));
}

bool DomainOnList(
    base::StringPiece document_domain,
    const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>* list) {
  return std::any_of(list->begin(), list->end(), [&](auto* filter_domain) {
    return DomainMatches(
        base::StringPiece(filter_domain->c_str(), filter_domain->size()),
        document_domain);
  });
}

}  // namespace

InstalledSubscriptionImpl::InstalledSubscriptionImpl(
    std::unique_ptr<FlatbufferData> data,
    InstallationState installation_state,
    base::Time installation_time)
    : buffer_(std::move(data)),
      installation_state_(installation_state),
      installation_time_(installation_time),
      regex_matcher_(std::make_unique<RegexMatcher>()) {
  DCHECK(buffer_);
  index_ = flat::GetSubscription(buffer_->data());
  regex_matcher_->PreBuildRegexPatternsWithNoKeyword(index_);
}

InstalledSubscriptionImpl::~InstalledSubscriptionImpl() = default;

GURL InstalledSubscriptionImpl::GetSourceUrl() const {
  return index_->metadata()->url() ? GURL(index_->metadata()->url()->str())
                                   : GURL();
}

std::string InstalledSubscriptionImpl::GetTitle() const {
  return index_->metadata()->title() ? index_->metadata()->title()->str() : "";
}

std::string InstalledSubscriptionImpl::GetCurrentVersion() const {
  return index_->metadata()->version() ? index_->metadata()->version()->str()
                                       : "";
}

Subscription::InstallationState
InstalledSubscriptionImpl::GetInstallationState() const {
  return installation_state_;
}

base::Time InstalledSubscriptionImpl::GetInstallationTime() const {
  return installation_time_;
}

base::TimeDelta InstalledSubscriptionImpl::GetExpirationInterval() const {
  return base::Milliseconds(index_->metadata()->expires());
}

bool InstalledSubscriptionImpl::HasUrlFilter(const GURL& url,
                                             const std::string& document_domain,
                                             ContentType content_type,
                                             const SiteKey& sitekey,
                                             FilterCategory category) const {
  return !FindInternal(category != FilterCategory::Allowing
                           ? index_->url_subresource_block()
                           : index_->url_subresource_allow(),
                       url, content_type, document_domain, sitekey.value(),
                       category, FindStrategy::FindFirst)
              .empty();
}

bool InstalledSubscriptionImpl::HasPopupFilter(
    const GURL& url,
    const std::string& document_domain,
    const SiteKey& sitekey,
    FilterCategory category) const {
  return !FindInternal(category != FilterCategory::Allowing
                           ? index_->url_popup_block()
                           : index_->url_popup_allow(),
                       url, absl::nullopt, document_domain, sitekey.value(),
                       category, FindStrategy::FindFirst)
              .empty();
}

void InstalledSubscriptionImpl::FindCspFilters(
    const GURL& url,
    const std::string& document_domain,
    FilterCategory category,
    std::set<base::StringPiece>& results) const {
  for (auto* filter : FindInternal(category != FilterCategory::Allowing
                                       ? index_->url_csp_block()
                                       : index_->url_csp_allow(),
                                   url, absl::nullopt, document_domain, "",
                                   category, FindStrategy::FindAll)) {
    DCHECK(category == FilterCategory::Allowing || filter->csp_filter())
        << "Blocking CSP filter must contain payload";
    results.insert(filter->csp_filter()
                       ? base::StringPiece(filter->csp_filter()->c_str(),
                                           filter->csp_filter()->size())
                       : base::StringPiece());
  }
}

absl::optional<base::StringPiece> InstalledSubscriptionImpl::FindRewriteFilter(
    const GURL& url,
    const std::string& document_domain,
    FilterCategory category) const {
  auto filters = FindInternal(category != FilterCategory::Allowing
                                  ? index_->url_rewrite_block()
                                  : index_->url_rewrite_allow(),
                              url, absl::nullopt, document_domain, "", category,
                              FindStrategy::FindFirst);
  if (filters.empty()) {
    return absl::nullopt;
  }
  const auto* filter = filters[0];
  DCHECK(category == FilterCategory::Allowing || filter->rewrite())
      << "Blocking rewrite filter must contain payload";
  if (filter->rewrite()) {
    auto new_url = RewriteUrl(filter->rewrite()->replace_with());
    DCHECK(!new_url.empty());
    return new_url;
  }
  return absl::nullopt;
}

void InstalledSubscriptionImpl::FindHeaderFilters(
    const GURL& url,
    ContentType content_type,
    const std::string& document_domain,
    FilterCategory category,
    std::set<HeaderFilterData>& results) const {
  for (auto* filter : FindInternal(category != FilterCategory::Allowing
                                       ? index_->url_header_block()
                                       : index_->url_header_allow(),
                                   url, content_type, document_domain, "",
                                   category, FindStrategy::FindAll)) {
    DCHECK(category == FilterCategory::Allowing || filter->header_filter())
        << "Blocking header filter must contain header_filter() payload";
    results.insert({base::StringPiece(filter->header_filter()->c_str(),
                                      filter->header_filter()->size()),
                    GetSourceUrl()});
  }
}

bool InstalledSubscriptionImpl::HasSpecialFilter(
    SpecialFilterType type,
    const GURL& url,
    const std::string& document_domain,
    const SiteKey& sitekey) const {
  const UrlFilterIndex* index = nullptr;
  switch (type) {
    case SpecialFilterType::Document:
      index = index_->url_document_allow();
      break;
    case SpecialFilterType::Elemhide:
      index = index_->url_elemhide_allow();
      break;
    case SpecialFilterType::Genericblock:
      index = index_->url_genericblock_allow();
      break;
    case SpecialFilterType::Generichide:
      index = index_->url_generichide_allow();
      break;
  }
  return !FindInternal(index, url, absl::nullopt, document_domain,
                       sitekey.value(), FilterCategory::Allowing,
                       FindStrategy::FindFirst)
              .empty();
}

std::vector<base::StringPiece> InstalledSubscriptionImpl::GetSelectorsForDomain(
    const flat::ElemHideFiltersByDomain* category,
    base::StringPiece domain) const {
  TRACE_EVENT1("eyeo", "InstalledSubscriptionImpl::GetSelectorsForDomain",
               "domain", domain);

  if (!category || !category->filter()) {
    // No filters found for this domain.
    return {};
  }

  std::vector<base::StringPiece> selectors;
  for (auto* filter : *category->filter()) {
    const bool filter_allowed_by_includes =
        // No include domains, filter is generic:
        filter->include_domains()->size() == 0 ||
        // Or include domains contain |domain| or one of its subdomains:
        DomainOnList(domain, filter->include_domains());
    if (!filter_allowed_by_includes) {
      continue;
    }
    const bool filter_disallowed_by_excludes =
        // Some exclusions apply on this domain:
        filter->exclude_domains()->size() > 0 &&
        // And those exclusions contain |domain| or one of its subdomains:
        DomainOnList(domain, filter->exclude_domains());
    if (filter_disallowed_by_excludes) {
      continue;
    }
    selectors.push_back(filter->selector()->c_str());
  }

  return selectors;
}

InstalledSubscription::Selectors
InstalledSubscriptionImpl::GetElemhideSelectors(const GURL& url,
                                                bool domain_specific) const {
  Selectors result;
  const std::string domain(base::ToLowerASCII(url.host()));
  if (!domain_specific) {
    result.elemhide_selectors =
        GetSelectorsForDomain(index_->elemhide()->LookupByKey(""), domain);
    result.elemhide_exceptions = GetSelectorsForDomain(
        index_->elemhide_exception()->LookupByKey(""), domain);
  }

  DomainSplitter domain_splitter(domain);
  while (auto subdomain = domain_splitter.FindNextSubdomain()) {
    auto specific_selectors = GetSelectorsForDomain(
        index_->elemhide()->LookupByKey(subdomain->data()), domain);
    std::move(specific_selectors.begin(), specific_selectors.end(),
              std::back_inserter(result.elemhide_selectors));
    auto specific_exceptions = GetSelectorsForDomain(
        index_->elemhide_exception()->LookupByKey(subdomain->data()), domain);
    std::move(specific_exceptions.begin(), specific_exceptions.end(),
              std::back_inserter(result.elemhide_exceptions));
  }

  return result;
}

InstalledSubscription::Selectors
InstalledSubscriptionImpl::GetElemhideEmulationSelectors(
    const GURL& url) const {
  const std::string& domain = url.host();
  Selectors result;
  DomainSplitter domain_splitter(domain);
  while (auto subdomain = domain_splitter.FindNextSubdomain()) {
    auto elemhide_selectors = GetSelectorsForDomain(
        index_->elemhide_emulation()->LookupByKey(subdomain->data()), domain);
    std::move(elemhide_selectors.begin(), elemhide_selectors.end(),
              std::back_inserter(result.elemhide_selectors));
    auto elemhide_exceptions = GetSelectorsForDomain(
        index_->elemhide_exception()->LookupByKey(subdomain->data()), domain);
    std::move(elemhide_exceptions.begin(), elemhide_exceptions.end(),
              std::back_inserter(result.elemhide_exceptions));
  }
  return result;
}

std::vector<const flat::UrlFilter*> InstalledSubscriptionImpl::FindInternal(
    const UrlFilterIndex* index,
    const GURL& url,
    absl::optional<ContentType> content_type,
    const std::string& document_domain,
    const std::string& sitekey,
    FilterCategory category,
    FindStrategy strategy) const {
  if (!index) {
    // No filters of this type were parsed.
    return {};
  }
  const std::string& normalized_domain =
      NeedsLowercasing(document_domain) ? base::ToLowerASCII(document_domain)
                                        : document_domain;
  const std::string normalized_sitekey = base::ToUpperASCII(sitekey);
  const GURL& lowercase_url =
      NeedsLowercasing(url.spec()) ? GURL(base::ToLowerASCII(url.spec())) : url;
  const bool is_third_party_request = IsThirdParty(url, document_domain);
  std::vector<const flat::UrlFilter*> results;

  UrlKeywordExtractor keyword_extractor(lowercase_url.spec());
  while (auto current_keyword = keyword_extractor.GetNextKeyword()) {
    FindFiltersForKeyword(index, *current_keyword, url, lowercase_url,
                          content_type, normalized_domain, normalized_sitekey,
                          category, is_third_party_request, strategy, results);
    if (strategy == FindStrategy::FindFirst && !results.empty()) {
      return results;
    }
  }

  FindFiltersForKeyword(index, "", url, lowercase_url, content_type,
                        normalized_domain, normalized_sitekey, category,
                        is_third_party_request, strategy, results);
  return results;
}

void InstalledSubscriptionImpl::FindFiltersForKeyword(
    const UrlFilterIndex* index,
    base::StringPiece keyword,
    const GURL& url,
    const GURL& lowercase_url,
    absl::optional<ContentType> content_type,
    const std::string& document_domain,
    const std::string& sitekey,
    FilterCategory category,
    bool is_third_party_request,
    FindStrategy strategy,
    std::vector<const flat::UrlFilter*>& out_results) const {
  const auto* idx = index->LookupByKey(keyword.data());

  if (!idx) {
    return;
  }

  for (const auto* filter : *(idx->filter())) {
    if (!CandidateFilterViable(filter, content_type, document_domain, sitekey,
                               category, is_third_party_request)) {
      continue;
    }

    if (filter->pattern()->size() == 0u) {
      // This filter applies to all URLs, assuming prior checks passed.
      out_results.push_back(filter);
      if (strategy == FindStrategy::FindFirst) {
        return;
      }
    }
    // During flatbuffer conversion, the pattern is lowercased for
    // case-insensitive filters, and left in original form for case-sensitive
    // filters.
    const base::StringPiece pattern(filter->pattern()->c_str(),
                                    filter->pattern()->size());
    if (ExtractRegexFilterFromPattern(pattern)) {
      if (regex_matcher_->MatchesRegex(pattern, url, filter->match_case())) {
        out_results.push_back(filter);
        if (strategy == FindStrategy::FindFirst) {
          return;
        }
      }
    } else {
      const auto& normalized_url = filter->match_case() ? url : lowercase_url;
      if (DoesPatternMatchUrl(pattern, normalized_url)) {
        out_results.push_back(filter);
        if (strategy == FindStrategy::FindFirst) {
          return;
        }
      }
    }
  }
}

bool InstalledSubscriptionImpl::CandidateFilterViable(
    const flat::UrlFilter* candidate,
    absl::optional<ContentType> content_type,
    const std::string& document_domain,
    const std::string& sitekey,
    FilterCategory category,
    bool is_third_party_request) const {
  if (content_type && (candidate->resource_type() & *content_type) == 0) {
    return false;
  }
  if (category == FilterCategory::DomainSpecificBlocking &&
      IsGenericFilter(candidate)) {
    return false;
  }
  if (!CheckThirdParty(candidate, is_third_party_request)) {
    return false;
  }
  if (!IsActiveOnDomain(candidate, document_domain, sitekey)) {
    return false;
  }
  return true;
}

bool InstalledSubscriptionImpl::CheckThirdParty(
    const flat::UrlFilter* filter,
    bool is_third_party_request) const {
  switch (filter->third_party()) {
    case flat::ThirdParty_Ignore:
      // This filter applies to first- and third-party requests requests.
      return true;
    case flat::ThirdParty_FirstPartyOnly:
      // This filter applies only to first-party requests.
      return !is_third_party_request;
    case flat::ThirdParty_ThirdPartyOnly:
      // This filter applies only to third-party requests.
      return is_third_party_request;
  }
}

bool InstalledSubscriptionImpl::IsGenericFilter(
    const flat::UrlFilter* filter) const {
  const auto* sitekeys = filter->sitekeys();
  DCHECK(sitekeys);

  if (sitekeys->size()) {
    return false;
  }

  return IsEmptyDomainAllowed(filter->include_domains(),
                              filter->exclude_domains());
}

bool InstalledSubscriptionImpl::IsActiveOnDomain(
    const flat::UrlFilter* filter,
    const std::string& document_domain,
    const std::string& sitekey) const {
  const auto* sitekeys = filter->sitekeys();
  DCHECK(sitekeys);

  if (sitekeys->size() != 0u) {
    if (std::none_of(
            sitekeys->begin(), sitekeys->end(),
            [&sitekey](const auto* it) { return it->c_str() == sitekey; })) {
      // This filter requires a sitekey, and the one provided doesn't match.
      return false;
    }
  }

  const auto* include_domains = filter->include_domains();
  const auto* exclude_domains = filter->exclude_domains();
  return IsActiveOnDomain(document_domain, include_domains, exclude_domains);
}

bool InstalledSubscriptionImpl::IsActiveOnDomain(
    const std::string& document_domain,
    const Domains* include_domains,
    const Domains* exclude_domains) const {
  if (IsEmptyDomainAllowed(include_domains, exclude_domains)) {
    return true;
  }

  // If |document_domain| matches any exclusion-type mapping for this filter,
  // the filter may not be applied to this domain.
  if (exclude_domains && DomainOnList(document_domain, exclude_domains)) {
    return false;
  }

  if (include_domains && include_domains->size()) {
    if (DomainOnList(document_domain, include_domains)) {
      return true;
    }
    return false;
  }

  // But if there are no include requirements for the filter, only exclude
  // domains, the filter applies.
  return true;
}

bool InstalledSubscriptionImpl::IsEmptyDomainAllowed(
    const Domains* include_domains,
    const Domains* exclude_domains) const {
  const bool has_no_exclude_domains =
      !exclude_domains || exclude_domains->size() == 0u;
  return  // optimization: instead of checking domains->LookupByKey(""), just
          // check first element is empty (list is sorted)
      (!include_domains || !include_domains->size() ||
       !include_domains->Get(0)->size()) &&
      has_no_exclude_domains;
}

std::vector<InstalledSubscription::Snippet>
InstalledSubscriptionImpl::MatchSnippets(
    const std::string& document_domain) const {
  std::vector<InstalledSubscription::Snippet> result;
  if (!index_->snippet()) {
    return result;
  }

  DomainSplitter domain_splitter(document_domain);
  while (auto subdomain = domain_splitter.FindNextSubdomain()) {
    const auto* idx = index_->snippet()->LookupByKey(subdomain->data());

    if (!idx) {
      continue;
    }

    for (const auto* cur : (*idx->filter())) {
      if (IsActiveOnDomain(document_domain, cur->include_domains(),
                           cur->exclude_domains())) {
        for (const auto* line : (*cur->script())) {
          InstalledSubscription::Snippet obj;
          obj.command = base::StringPiece(line->command()->c_str(),
                                          line->command()->size());
          obj.arguments.reserve(line->arguments()->size());

          for (const auto* arg : (*line->arguments())) {
            obj.arguments.emplace_back(arg->c_str(), arg->size());
          }

          result.push_back(std::move(obj));
        }
      }
    }
  }

  return result;
}

void InstalledSubscriptionImpl::MarkForPermanentRemoval() {
  buffer_->PermanentlyRemoveSourceOnDestruction();
}

}  // namespace adblock
