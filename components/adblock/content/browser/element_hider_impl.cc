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

#include "components/adblock/content/browser/element_hider_impl.h"

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/json/json_string_value_serializer.h"
#include "base/json/string_escape.h"
#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/trace_event/trace_event.h"
#include "components/adblock/core/subscription/subscription_service.h"
#include "components/grit/components_resources.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/global_routing_id.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/common/isolated_world_ids.h"
#include "ui/base/resource/resource_bundle.h"

namespace adblock {
namespace {

std::string GenerateBlockedElemhideJavaScript(
    const std::string& url,
    const std::string& filename_with_query) {
  TRACE_EVENT1("eyeo", "GenerateBlockedElemhideJavaScript", "url", url);
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // basically copy-pasted JS and saved to resources from
  // https://github.com/adblockplus/adblockpluschrome/blob/master/include.preload.js#L546
  std::string result =
      ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
          IDR_ADBLOCK_ELEMHIDE_JS);

  result.append("\n");

  // the file is template with tokens to be replaced with actual variable values
  std::string query_jst =
      ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
          IDR_ADBLOCK_ELEMHIDE_FOR_SELECTOR_JS);

  std::string build_string;
  base::EscapeJSONString(filename_with_query, false, &build_string);

  base::ReplaceSubstringsAfterOffset(&query_jst, 0, "{{url}}", url);
  base::ReplaceSubstringsAfterOffset(&query_jst, 0, "{{filename_with_query}}",
                                     build_string);
  result.append(query_jst);

  return result;
}

void GenerateStylesheet(const GURL& url,
                        std::vector<base::StringPiece>& input,
                        std::string& output) {
  TRACE_EVENT1("eyeo", "GenerateStylesheet", "url", url.spec());
  // Chromium's Blink engine supports only up to 8,192 simple selectors, and
  // even fewer compound selectors, in a rule. The exact number of selectors
  // that would work depends on their sizes (e.g. "#foo .bar" has a size of 2).
  // Since we don't know the sizes of the selectors here, we simply split them
  // into groups of 1,024, based on the reasonable assumption that the average
  // selector won't have a size greater than 8. The alternative would be to
  // calculate the sizes of the selectors and divide them up accordingly, but
  // this approach is more efficient and has worked well in practice. In theory
  // this could still lead to some selectors not working on Chromium, but it is
  // highly unlikely.
  const size_t max_selector_count = 1024u;
  for (size_t i = 0; i < input.size(); i += max_selector_count) {
    const size_t batch_size = std::min(max_selector_count, input.size() - i);
    const base::span<base::StringPiece> selectors_batch(&input[i], batch_size);
    output += base::JoinString(selectors_batch, ", ") +
              " {display: none !important;}\n";
  }
}

void GenerateElemHidingEmuJavaScript(
    const GURL& url,
    const std::vector<base::StringPiece>& input,
    std::string& output) {
  TRACE_EVENT1("eyeo", "GenerateElemHidingEmuJavaScript", "url", url.spec());
  // build the string with selectors
  std::string build_string;
  for (const auto& selector : input) {
    build_string.append("{selector:");
    base::EscapeJSONString(selector, true, &build_string);
    build_string.append(", text:");
    base::EscapeJSONString(url.host(), true, &build_string);
    build_string.append("}, \n");
  }
  output = ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
      IDR_ADBLOCK_ELEMHIDE_EMU_JS);

  base::ReplaceSubstringsAfterOffset(
      &output, 0, "{{elemHidingEmulatedPatternsDef}}", build_string);
}

void GenerateSnippetScript(const GURL& url,
                           base::Value::List input,
                           std::string& output) {
  TRACE_EVENT1("eyeo", "GenerateSnippetScript", "url", url.spec());
  // snippets must be JSON representation of the array of arrays of snippets
  std::string serialized;
  JSONStringValueSerializer serializer(&serialized);
  serializer.Serialize(std::move(input));
  // snippets_lib should be the library as-is, without any escaping or JSON
  // parsing.
  static std::string snippets_lib =
      ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
          IDR_ADBLOCK_SNIPPETS_JS);
  output = "({{callback}})({}, ...{{snippets}});";
  base::ReplaceSubstringsAfterOffset(&output, 0, "{{callback}}", snippets_lib);
  base::ReplaceSubstringsAfterOffset(&output, 0, "{{snippets}}", serialized);
}

ElementHider::ElemhideInjectionData PrepareElemhideEmulationData(
    const SubscriptionService::Snapshot subscription_collections,
    const GURL url,
    const std::vector<GURL> frame_hierarchy,
    const SiteKey sitekey) {
  TRACE_EVENT1("eyeo", "PrepareElemhideEmulationData", "url", url.spec());

  std::vector<base::StringPiece> stylesheet;
  std::vector<base::StringPiece> elemhide_js;
  base::Value::List snippet_js;
  for (const auto& collection : subscription_collections) {
    bool doc_allowlisted = !!collection->FindBySpecialFilter(
        SpecialFilterType::Document, url, frame_hierarchy, sitekey);
    bool ehe_allowlisted =
        doc_allowlisted ||
        collection->FindBySpecialFilter(SpecialFilterType::Elemhide, url,
                                        frame_hierarchy, sitekey);
    if (!ehe_allowlisted) {
      base::ranges::copy(
          collection->GetElementHideSelectors(url, frame_hierarchy, sitekey),
          std::back_inserter(stylesheet));
      base::ranges::copy(collection->GetElementHideEmulationSelectors(url),
                         std::back_inserter(elemhide_js));
    }
    if (!doc_allowlisted) {
      base::ranges::for_each(
          collection->GenerateSnippets(url, frame_hierarchy),
          [&snippet_js](auto& item) { snippet_js.Append(std::move(item)); });
    }
  }
  ElementHider::ElemhideInjectionData result;
  if (!stylesheet.empty()) {
    DVLOG(2) << "[eyeo] Got " << stylesheet.size() << " EH selectors for url "
             << url;
    GenerateStylesheet(url, stylesheet, result.stylesheet);
  }
  if (!elemhide_js.empty()) {
    DVLOG(2) << "[eyeo] Got " << elemhide_js.size()
             << " EH emu selectors for url " << url;
    GenerateElemHidingEmuJavaScript(url, elemhide_js, result.elemhide_js);
  }
  if (!snippet_js.empty()) {
    DVLOG(2) << "[eyeo] Got " << snippet_js.size() << " snippets for url "
             << url;
    GenerateSnippetScript(url, std::move(snippet_js), result.snippet_js);
  }
  return result;
}

void InsertUserCSSAndApplyElemHidingEmuJS(
    content::GlobalRenderFrameHostId frame_host_id,
    base::OnceCallback<void(const ElementHider::ElemhideInjectionData&)>
        on_finished,
    ElementHider::ElemhideInjectionData input) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* frame_host = content::RenderFrameHost::FromID(frame_host_id);
  if (!frame_host) {
    // Render frame host was destroyed before element hiding could be applied.
    // This is not a bug, just legitimate a race condition.
    std::move(on_finished).Run(std::move(input));
    return;
  }
  if (!input.stylesheet.empty()) {
    frame_host->InsertAbpElemhideStylesheet(input.stylesheet);
    DVLOG(1) << "[eyeo] Element hiding - inserted stylesheet in frame"
             << " '" << frame_host->GetFrameName() << "'";
  }

  if (!input.elemhide_js.empty()) {
    frame_host->ExecuteJavaScriptInIsolatedWorld(
        base::UTF8ToUTF16(input.elemhide_js),
        content::RenderFrameHost::JavaScriptResultCallback(),
        content::ISOLATED_WORLD_ID_ADBLOCK);

    DVLOG(1) << "[eyeo] Element hiding emulation - called JS in frame"
             << " '" << frame_host->GetFrameName() << "'";
  }

  if (!input.snippet_js.empty()) {
    // PK: Extension API ends up generating isolated world for injected script
    // execution. See GetIsolatedWorldIdForInstance in
    // extensions/renderer/script_injection.cc. Why not to reuse adblock space?
    frame_host->ExecuteJavaScriptInIsolatedWorld(
        base::UTF8ToUTF16(input.snippet_js),
        content::RenderFrameHost::JavaScriptResultCallback(),
        content::ISOLATED_WORLD_ID_ADBLOCK);

    DVLOG(1) << "[eyeo] Snippet - called JS in frame"
             << " '" << frame_host->GetFrameName() << "'";
  }

  std::move(on_finished).Run(std::move(input));
}

}  // namespace

ElementHiderImpl::ElementHiderImpl(SubscriptionService* subscription_service)
    : subscription_service_(subscription_service) {}

ElementHiderImpl::~ElementHiderImpl() = default;

void ElementHiderImpl::ApplyElementHidingEmulationOnPage(
    GURL url,
    std::vector<GURL> frame_hierarchy,
    content::RenderFrameHost* render_frame_host,
    SiteKey sitekey,
    base::OnceCallback<void(const ElemhideInjectionData&)> on_finished) {
  ApplyElementHidingEmulationInternal(
      std::move(url), std::move(frame_hierarchy),
      render_frame_host->GetGlobalId(), std::move(sitekey),
      std::move(on_finished));
}

bool ElementHiderImpl::IsElementTypeHideable(
    ContentType adblock_resource_type) const {
  switch (adblock_resource_type) {
    case ContentType::Image:
    case ContentType::Object:
    case ContentType::Media:
      return true;

    default:
      break;
  }
  return false;
}

void ElementHiderImpl::HideBlockedElement(
    const GURL& url,
    content::RenderFrameHost* render_frame_host) {
  TRACE_EVENT1("eyeo", "ElementHiderFlatbufferImpl::HideBlockedElemenet", "url",
               url.spec());
  // we can't get relative URL from URLRequest
  // so the hack is to select in JS with filename_with_query selector and then
  // check every found element's full absolute URL
  std::string filename_with_query = url.ExtractFileName();
  if (url.has_query()) {
    filename_with_query.append("?");
    filename_with_query.append(url.query());
  }

  const std::string js =
      GenerateBlockedElemhideJavaScript(url.spec(), filename_with_query);

  // elemhide resource by element hide rules
  render_frame_host->ExecuteJavaScriptInIsolatedWorld(
      base::UTF8ToUTF16(js),
      content::RenderFrameHost::JavaScriptResultCallback(),
      content::ISOLATED_WORLD_ID_ADBLOCK);

  VLOG(1) << "[eyeo] Element hiding - called JS: " << js;
}

void ElementHiderImpl::ApplyElementHidingEmulationInternal(
    GURL url,
    std::vector<GURL> frame_hierarchy,
    content::GlobalRenderFrameHostId frame_host_id,
    SiteKey sitekey,
    base::OnceCallback<void(const ElementHider::ElemhideInjectionData&)>
        on_finished) {
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {},
      base::BindOnce(&PrepareElemhideEmulationData,
                     subscription_service_->GetCurrentSnapshot(),
                     std::move(url), std::move(frame_hierarchy),
                     std::move(sitekey)),
      base::BindOnce(&InsertUserCSSAndApplyElemHidingEmuJS, frame_host_id,
                     std::move(on_finished)));
}

}  // namespace adblock
